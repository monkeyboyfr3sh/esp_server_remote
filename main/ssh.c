#include "ssh.h"

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "netdb.h" // gethostbyname

#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define COLOR_1 "\e[38;5;45m"
#define COLOR_2 "\e[38;5;83m"

static const char *TAG = "SSH";

static void ssh_task_exit(ssh_task_input_t * task_parameter);
static void ssh_task_fail(ssh_task_input_t * task_parameter);

static int waitsocket(int socket_fd, LIBSSH2_SESSION *session)
{
	struct timeval timeout;
	int rc;
	fd_set fd;
	fd_set *writefd = NULL;
	fd_set *readfd = NULL;
	int dir;

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);

	FD_SET(socket_fd, &fd);

	/* now make sure we wait in the correct direction */
	dir = libssh2_session_block_directions(session);

	if(dir & LIBSSH2_SESSION_BLOCK_INBOUND)
		readfd = &fd;

	if(dir & LIBSSH2_SESSION_BLOCK_OUTBOUND)
		writefd = &fd;

	rc = select(socket_fd + 1, readfd, writefd, NULL, &timeout);

	return rc;
}

#define BUFSIZE 3200

void print_command(char* command)
{
	// TODO: want to configure this to use STDOUT/ERR
	printf("%s%s%s@%s%s\e[0m: ", COLOR_1,CONFIG_SSH_USER,COLOR_2,COLOR_1,CONFIG_SSH_HOST);
	printf("%s > \r\n",command);
}

void ssh_task(void *pvParameters)
{
	ssh_task_input_t * task_parameter = (ssh_task_input_t *)pvParameters;
	ESP_LOGI(TAG, "Start task_parameter=%s", task_parameter->command);

	// SSH Staff
	int sock;
	struct sockaddr_in sin;
	LIBSSH2_SESSION *session;
	LIBSSH2_CHANNEL *channel;

	ESP_LOGI(TAG, "libssh2_version is %s", LIBSSH2_VERSION);
	int rc = libssh2_init(0);
	if(rc) {
		ESP_LOGE(TAG, "libssh2 initialization failed (%d)", rc);
		ssh_task_fail(task_parameter);
	}

	ESP_LOGI(TAG, "CONFIG_SSH_HOST=%s", CONFIG_SSH_HOST);
	ESP_LOGI(TAG, "CONFIG_SSH_PORT=%d", CONFIG_SSH_PORT);
	sin.sin_family = AF_INET;
	//sin.sin_port = htons(22);
	sin.sin_port = htons(CONFIG_SSH_PORT);
	sin.sin_addr.s_addr = inet_addr(CONFIG_SSH_HOST);
	ESP_LOGI(TAG, "sin.sin_addr.s_addr=%x", sin.sin_addr.s_addr);
	if (sin.sin_addr.s_addr == 0xffffffff) {
		struct hostent *hp;
		hp = gethostbyname(CONFIG_SSH_HOST);
		if (hp == NULL) {
			ESP_LOGE(TAG, "gethostbyname fail %s", CONFIG_SSH_HOST);
			ssh_task_fail(task_parameter);
		}
		struct ip4_addr *ip4_addr;
		ip4_addr = (struct ip4_addr *)hp->h_addr;
		sin.sin_addr.s_addr = ip4_addr->addr;
		ESP_LOGI(TAG, "sin.sin_addr.s_addr=%x", sin.sin_addr.s_addr);
	}

	ESP_LOGI(TAG,"Opening socket");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		ESP_LOGE(TAG, "failed to create socket!");
		ssh_task_fail(task_parameter);
	}

	ESP_LOGI(TAG,"Connecting to socket");
	if(connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
		ESP_LOGE(TAG, "failed to connect!");
		ssh_task_fail(task_parameter);
	}

	/* Create a session instance */
	session = libssh2_session_init();
	if(!session) {
		ESP_LOGE(TAG, "failed to session init");
		ssh_task_fail(task_parameter);
	}

	/* ... start it up. This will trade welcome banners, exchange keys,
	 * and setup crypto, compression, and MAC layers
	 */
	rc = libssh2_session_handshake(session, sock);
	if(rc) {
		ESP_LOGE(TAG, "Failure establishing SSH session: %d", rc);
		ssh_task_fail(task_parameter);
	}

	/* We could authenticate via password */
	if(libssh2_userauth_password(session, CONFIG_SSH_USER, CONFIG_SSH_PASSWORD)) {
		ESP_LOGE(TAG, "Authentication by password failed.");
		ESP_LOGE(TAG, "Authentication username : [%s].", CONFIG_SSH_USER);
		ssh_task_fail(task_parameter);
	}

#if 0
	/* We could authenticate via privatekey */
	char publickey[64];
	char privatekey[64];
	strcpy(publickey, "/spiffs/id_rsa.pub");
	strcpy(privatekey, "/spiffs/id_rsa");
	if(libssh2_userauth_publickey_fromfile(session, CONFIG_SSH_USER, publickey, privatekey, NULL)) {
		ESP_LOGE(TAG, "Authentication by privatekey failed.");
		ESP_LOGE(TAG, "Authentication username : [%s].", CONFIG_SSH_USER);
		ssh_task_fail(task_parameter);
	}
#endif


	libssh2_trace(session, LIBSSH2_TRACE_SOCKET);

	/* Exec non-blocking on the remove host */
	while((channel = libssh2_channel_open_session(session)) == NULL &&
		libssh2_session_last_error(session, NULL, NULL, 0) ==
		LIBSSH2_ERROR_EAGAIN) {
		waitsocket(sock, session);
	}
	if(channel == NULL) {
		ESP_LOGE(TAG, "libssh2_channel_open_session failed.");
		ssh_task_fail(task_parameter);
	}

	while((rc = libssh2_channel_exec(channel, task_parameter->command)) == LIBSSH2_ERROR_EAGAIN)
	waitsocket(sock, session);
	if(rc != 0) {
		ESP_LOGE(TAG, "libssh2_channel_exec failed: %d", rc);
		ssh_task_fail(task_parameter);
	}

	print_command(task_parameter->command);

	// Now execute the command
	int bytecount = 0;
	for(;;) {
		/* loop until we block */
		int rc;
		do {
			char buffer[128];
			rc = libssh2_channel_read(channel, buffer, sizeof(buffer) );
			if(rc > 0) {
				int i;
				bytecount += rc;
				for(i = 0; i < rc; ++i)
					// fputc(buffer[i], stderr);
					fputc(buffer[i], stdout);
			}
			else if(rc < 0) {
					/* no need to output this for the EAGAIN case */
					ESP_LOGI(TAG, "libssh2_channel_read returned %d", rc);
					ssh_task_fail(task_parameter);
			}
		}
		while(rc > 0);

		/* this is due to blocking that would occur otherwise so we loop on
		 this condition */
		if(rc == LIBSSH2_ERROR_EAGAIN) {
			waitsocket(sock, session);
		}
		else
			break;
	} // end for
	fputc("\n", stdout);

	int exitcode = 127;
	char *exitsignal = (char *)"none";
	while((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN)
	waitsocket(sock, session);
	if(rc == 0) {
		exitcode = libssh2_channel_get_exit_status(channel);
		libssh2_channel_get_exit_signal(channel, &exitsignal,
										NULL, NULL, NULL, NULL, NULL);
	} else {
		ESP_LOGE(TAG, "libssh2_channel_close failed: %d", rc);
		ssh_task_fail(task_parameter);
	}

	// How are we exiting
	if(exitsignal)	{ ESP_LOGI(TAG, "EXIT: %d, SIGNAL: %s, bytecount: %d", exitcode, exitsignal, bytecount); }
	else			{ ESP_LOGI(TAG, "EXIT: %d, bytecount: %d", exitcode, bytecount); }

	libssh2_channel_free(channel);
	channel = NULL;

	// Close a session
	libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
	libssh2_session_free(session);

	// Close socket
	close(sock);
	ESP_LOGI(TAG, "[%s] done\n", task_parameter->command);

	libssh2_exit();

	ssh_task_exit(task_parameter);
}

static void ssh_task_exit(ssh_task_input_t * task_parameter)
{
	xEventGroupSetBits( task_parameter->xEventGroup, SSH_TASK_FINISH_BIT );
	vTaskDelete( NULL );
}

static void ssh_task_fail(ssh_task_input_t * task_parameter)
{
	xEventGroupSetBits( task_parameter->xEventGroup, SSH_TASK_FAIL_BIT );
	ssh_task_exit(task_parameter);
}

esp_err_t create_ssh_task_input(ssh_task_input_t * task_parameters, char * command)
{
	// Create Eventgroup
	EventGroupHandle_t xEventGroup = xEventGroupCreate();
	configASSERT( xEventGroup );
	xEventGroupClearBits( xEventGroup, SSH_TASK_FINISH_BIT );
	
	ssh_task_input_t input = {
		.xEventGroup = xEventGroup,
		.command = command,
	};
	*task_parameters = input;

    return ESP_OK;
}

esp_err_t delete_ssh_task_input(ssh_task_input_t * task_parameters)
{
	// De-alloc event group
	vEventGroupDelete(task_parameters->xEventGroup);
	return ESP_OK;
}