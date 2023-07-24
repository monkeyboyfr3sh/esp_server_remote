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
#include "esp_err.h"

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

esp_err_t initialize_libssh( void )
{
	// Initialize libssh library
	ESP_LOGI(TAG, "Initializing libssh2... Version: %s", LIBSSH2_VERSION);
	int rc = libssh2_init(0);
    if(rc) {
		return ESP_FAIL;
    }
	return ESP_OK;
}

int create_socket(struct sockaddr_in *sin) 
{
	int sock;

	ESP_LOGI(TAG, "CONFIG_SSH_HOST=%s", CONFIG_SSH_HOST);
	ESP_LOGI(TAG, "CONFIG_SSH_PORT=%d", CONFIG_SSH_PORT);
	sin->sin_family = AF_INET;
	//sin.sin_port = htons(22);
	sin->sin_port = htons(CONFIG_SSH_PORT);
	sin->sin_addr.s_addr = inet_addr(CONFIG_SSH_HOST);
	ESP_LOGI(TAG, "sin.sin_addr.s_addr=%x", sin->sin_addr.s_addr);
	if (sin->sin_addr.s_addr == 0xffffffff) {
		struct hostent *hp;
		hp = gethostbyname(CONFIG_SSH_HOST);
		if (hp == NULL) {
			ESP_LOGE(TAG, "gethostbyname fail %s", CONFIG_SSH_HOST);
			return -1;
		}
		struct ip4_addr *ip4_addr;
		ip4_addr = (struct ip4_addr *)hp->h_addr;
		sin->sin_addr.s_addr = ip4_addr->addr;
		ESP_LOGI(TAG, "sin.sin_addr.s_addr=%x", sin->sin_addr.s_addr);
	}

	ESP_LOGI(TAG,"Opening socket");
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock == -1) {
		ESP_LOGE(TAG, "failed to create socket!");
		return -1;
	}

    return sock;
}

LIBSSH2_SESSION *create_ssh_session(int sock)
{
    LIBSSH2_SESSION *session;

	/* Create a session instance */
	session = libssh2_session_init();
	if(!session) {
		ESP_LOGE(TAG, "failed to session init");
		return NULL;
	}

	/* ... start it up. This will trade welcome banners, exchange keys,
	 * and setup crypto, compression, and MAC layers
	 */
	int rc = libssh2_session_handshake(session, sock);
	if(rc) {
		ESP_LOGE(TAG, "Failure establishing SSH session: %d", rc);
		return NULL;
	}

	/* We could authenticate via password */
	if(libssh2_userauth_password(session, CONFIG_SSH_USER, CONFIG_SSH_PASSWORD)) {
		ESP_LOGE(TAG, "Authentication by password failed.");
		ESP_LOGE(TAG, "Authentication username : [%s].", CONFIG_SSH_USER);
		return NULL;
	}

    return session;
}

LIBSSH2_CHANNEL *exec_ssh_command(LIBSSH2_SESSION *session, int sock, char * command)
{
    LIBSSH2_CHANNEL *channel;
    int rc;

	libssh2_trace(session, LIBSSH2_TRACE_SOCKET);

	/* Exec non-blocking on the remove host */
	while((channel = libssh2_channel_open_session(session)) == NULL &&
		libssh2_session_last_error(session, NULL, NULL, 0) ==
		LIBSSH2_ERROR_EAGAIN) {
		waitsocket(sock, session);
	}
	if(channel == NULL) {
		ESP_LOGE(TAG, "libssh2_channel_open_session failed.");
		return NULL;
	}

	while((rc = libssh2_channel_exec(channel, command)) == LIBSSH2_ERROR_EAGAIN)
	waitsocket(sock, session);
	if(rc != 0) {
		ESP_LOGE(TAG, "libssh2_channel_exec failed: %d", rc);
		return NULL;
	}

    return channel;
}

int read_channel(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int sock)
{
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
					return -1;
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
	// Finish with newline
	fputc('\n', stdout);
	
	return bytecount;
}

int close_ssh_channel(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int sock, int bytecount) {
	int rc;
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
		return -1;
	}

	// How are we exiting
	if(exitsignal)	{ ESP_LOGI(TAG, "EXIT: %d, SIGNAL: %s, bytecount: %d", exitcode, exitsignal, bytecount); }
	else			{ ESP_LOGI(TAG, "EXIT: %d, bytecount: %d", exitcode, bytecount); }

	// Set bit based on exit status
	if( (exitsignal==0) || (exitsignal==1) ){
		// xEventGroupSetBits( task_parameter->xEventGroup, SSH_CMD_SUCCESS_BIT );
	} else {
		return -1;
	}

	libssh2_channel_free(channel);
	channel = NULL;
	return 0;
}

void disconnect_and_cleanup(LIBSSH2_SESSION *session, int sock) {
	// Close a session
	libssh2_session_disconnect(session, "Normal Shutdown, Thank you for playing");
	libssh2_session_free(session);

	// Close socket
	close(sock);

	// Stop libssh2 lib
	libssh2_exit();
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

	// Init lib ssh
	esp_err_t ret = initialize_libssh();
	if(ret!=ESP_OK){
		ESP_LOGI(TAG, "Failed to init libssh!");
		ssh_task_fail(task_parameter);
	}

	// Create the socket
	sock = create_socket(&sin); 
	if (sock < 0){
		ESP_LOGE(TAG, "failed to create socket!");
		ssh_task_fail(task_parameter);
	}

	// Connect to socket
	ESP_LOGI(TAG,"Connecting to socket");
	if(connect(sock, (struct sockaddr*)(&sin), sizeof(struct sockaddr_in)) != 0) {
		ESP_LOGE(TAG, "failed to connect!");
		ssh_task_fail(task_parameter);
	}

	// Create the ssh session
	session = create_ssh_session(sock);
	if(session == NULL){
		ESP_LOGE(TAG,"Failed to create ssh session!");
		ssh_task_fail(task_parameter);
	}

	// Print the command we're gonna run
	print_command(task_parameter->command);

	// Open SSH channel and exec command
	channel = exec_ssh_command(session, sock, task_parameter->command);
	if(channel == NULL ){
		ESP_LOGE(TAG,"Failed to exec ssh command!");
		ssh_task_fail(task_parameter);
	}

	// Read back response TODO: Make this take a buffer
	int bytecount = read_channel(channel, session, sock);
	if(bytecount < 0 ){
		ESP_LOGE(TAG,"Failed to read channel!");
		ssh_task_fail(task_parameter);
	}

	// Close ssh channel
	int rc = close_ssh_channel(channel, session, sock, bytecount);
	if(rc<0){
		ESP_LOGE(TAG,"Failed to close ssh channel!");
		ssh_task_fail(task_parameter);
	}

	// Mark success if we got here
	xEventGroupSetBits( task_parameter->xEventGroup, SSH_CMD_SUCCESS_BIT );

	// Now cleanup
	disconnect_and_cleanup(session,sock);

	// Complete!
	ESP_LOGI(TAG, "[%s] done\n", task_parameter->command);
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