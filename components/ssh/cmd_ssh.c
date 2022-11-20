/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include "ssh.h"

static const char *TAG = "cmd_ssh";

EventGroupHandle_t xEventGroup;
int TASK_FINISH_BIT	= BIT4;
#define SSH_MIN_ARGS    3

static void register_ssh_start(void);
static void register_ssh_command_task(void);

void register_ssh(void)
{
    register_ssh_start();
    register_ssh_command_task();
}

static int ssh_start(int argc, char **argv)
{
    // Create Eventgroup
	xEventGroup = xEventGroupCreate();
	configASSERT( xEventGroup );

    char * buff = "qm list";

    // Execute ssh command
    xEventGroupClearBits( xEventGroup, TASK_FINISH_BIT );
    xTaskCreate(&ssh_task, "SSH", 1024*8, (void *) buff, 2, NULL);

    // Wit for ssh finish.
    xEventGroupWaitBits( xEventGroup,
        TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
        pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
        pdFALSE,			/* Don't wait for both bits, either bit will do. */
        portMAX_DELAY);		/* Wait forever. */	

	ESP_LOGI(TAG, "SSH all finish");
    return 0;
}

static void register_ssh_start(void)
{
    const esp_console_cmd_t ssh_start_cmd = {
        .command = "ssh",
        .help = "Start SSH session",
        .hint = NULL,
        .func = &ssh_start,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&ssh_start_cmd) );

}

static int ssh_command_task_start(int argc, char **argv)
{
    // Create Eventgroup
	xEventGroup = xEventGroupCreate();
	configASSERT( xEventGroup );

    // Dump the args for debug
    printf("[");
    for(int i = 0;i<argc;i++){ printf("%s, ",argv[i]); }
    printf("]\r\n");

    // We should have some number of arguments
    int num_inputs = argc-1;
    if(num_inputs<SSH_MIN_ARGS){
        ESP_LOGE(TAG,"Not enough args. Min is %d",SSH_MIN_ARGS);
        return ESP_ERR_INVALID_ARG;
    }

    // Get the arguments and check them
    char *username = argv[1];
    char *host = argv[2];
    char *command = argv[3];
    if( (username==NULL) || (host==NULL) || (command==NULL) ){
        ESP_LOGE(TAG,"Error with argument.");
        return ESP_ERR_INVALID_ARG;
    }

    // Put them into the input struct
    ssh_cmd_input_t ssh_command;
    ssh_command.username = username;
    ssh_command.host = host;
    ssh_command.command = command;
    ssh_command.password = NULL;

    // Now check for options
    for(int i = SSH_MIN_ARGS+1;i<argc;i++){
        if( ( !strncmp(argv[i],"-p",1024) || ( !strncmp(argv[i],"-P",1024))) && 
            ( i+1 < argc )
        ){
            ssh_command.password = argv[i+1];
        }
    }

    // Execute ssh command
    xEventGroupClearBits( xEventGroup, TASK_FINISH_BIT );
    xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);

    // Wit for ssh finish.
    xEventGroupWaitBits( xEventGroup,
        TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
        pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
        pdFALSE,			/* Don't wait for both bits, either bit will do. */
        portMAX_DELAY);		/* Wait forever. */	

	ESP_LOGI(TAG, "SSH all finish");
    return 0;
}

static void register_ssh_command_task(void)
{
    const esp_console_cmd_t ssh_command_task_cmd = {
        .command = "ssh_cmd",
        .help = "Execute single task via SSH",
        .hint = NULL,
        .func = &ssh_command_task_start,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&ssh_command_task_cmd) );

}