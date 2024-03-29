#include "ssh_task.h"

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

#include "ssh_driver.h"

static const char *TAG = "SSH_DRIVER";

static void ssh_task_exit(ssh_task_input_t * task_parameter);
static void ssh_task_fail(ssh_task_input_t * task_parameter);

void ssh_task(void *pvParameters)
{
    // null input check
    if(pvParameters==NULL){
        ESP_LOGE(TAG,"Null param!");
	    vTaskDelete( NULL );
    }

    // Get input
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
    const size_t buffer_size = 4096;
    char * rx_buff = malloc(buffer_size*sizeof(char));
    int bytecount = read_channel(channel, session, sock, rx_buff, buffer_size);
	// int bytecount = read_channel(channel, session, sock);
	if(bytecount < 0 ){
		ESP_LOGE(TAG,"Failed to read channel!");
		ssh_task_fail(task_parameter);
	}
    ESP_LOGI(TAG,"SSH CHannel read:\r\n%s",rx_buff);
    free(rx_buff);

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
	ESP_LOGI(TAG, "[%s] done", task_parameter->command);
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

esp_err_t run_ssh_task_blocked(char * command)
{
    
    esp_err_t ret = ESP_OK;

    // Create input for ssh task
    ssh_task_input_t task_parameters;
    ESP_ERROR_CHECK( create_ssh_task_input((ssh_task_input_t *)&task_parameters, (char *)command));

    // Execute ssh command
    xTaskCreate(&ssh_task, "SSH", 1024 * 8, (void *)&task_parameters, 2, NULL);

    // Wait for ssh task to finish.
    xEventGroupWaitBits(task_parameters.xEventGroup,
                        SSH_TASK_FINISH_BIT, /* The bits within the event group to wait for. */
                        pdTRUE,              /* SSH_TASK_FINISH_BIT should be cleared before returning. */
                        pdFALSE,             /* Don't wait for both bits, either bit will do. */
                        portMAX_DELAY);      /* Wait forever. */

    // Check if failed
    EventBits_t flags = xEventGroupGetBits(task_parameters.xEventGroup);
    if (flags & SSH_TASK_FAIL_BIT) {
        ESP_LOGE(TAG, "SSH task failed!");
        ret = ESP_FAIL;
    } else {
        ESP_LOGI(TAG, "SSH task finished successfully!");
        ret = ESP_OK;
    }

    // Delete the input
    ESP_ERROR_CHECK(delete_ssh_task_input((ssh_task_input_t *)&task_parameters));

    return ret;
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
