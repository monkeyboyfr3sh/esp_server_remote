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
#include "esp_sleep.h"
#include "esp_spi_flash.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "cmd_system.h"
#include "sdkconfig.h"
#include "ssh.h"

static const char *TAG = "cmd_system";

EventGroupHandle_t xEventGroup;
int TASK_FINISH_BIT	= BIT4;

static void register_ssh_start(void);

void register_ssh(void)
{
    register_ssh_start();
}

/* 'clear' command clears terminal screen */
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