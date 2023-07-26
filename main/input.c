#include "input.h"

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_err.h"
#include "esp_log.h"

#include "ssh_task.h"
#include "display_helper.h"

static const char *TAG = "INPUT";

#define WDT_PERIOD_TICKS	100
#define CHAR_PRINT_LOW		32
#define CHAR_PRINT_HIGH		126
#define CHAR_BS				8
#define CHAR_SPACE			32
#define CHAR_ENTER			10

static esp_err_t get_command(char* buff, int buff_len)
{
	TickType_t WDT_timestamp = xTaskGetTickCount();
	int char_count = 0;
	char c;

	printf("Enter command: ");

	// Go into busy loop for capturing the command
	do{
		// WDT reset
		if(xTaskGetTickCount()-WDT_timestamp > WDT_PERIOD_TICKS ){ 
			WDT_timestamp = xTaskGetTickCount();
			vTaskDelay(1); 
		}

		// Get the character, print if valid
		c = fgetc(stdin);
		
		// New character to capture
		if( (CHAR_PRINT_LOW <= c) && (c <= CHAR_PRINT_HIGH) && ( (char_count-1) < buff_len ) ){
			fputc(c,stdout);
			buff[char_count] = c;
			char_count++;
		}
		// Detect back space
		else if( c == CHAR_BS && ( char_count ) ){
			char_count--;
			fputc(CHAR_BS,stdout);
			fputc(CHAR_SPACE,stdout);
			fputc(CHAR_BS,stdout);	
		}
		// Detect enter key 
		else if( c == CHAR_ENTER ){
			printf("\r\n");
			break;
		}

	} while(1);

	buff[char_count] = 0;

	return ESP_OK;
}

void stdin_input_task(void * pvParameters)
{
	char buff[1024];
	while(1){
		
		// Get the command
		esp_err_t err = get_command(buff,1024);
		if (err != ESP_OK){
			ESP_LOGE(TAG,"Error: %s",esp_err_to_name(err));
		}
		
    	// Set to wakeup on pattern
    	display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

		// Create input for ssh task
		ssh_task_input_t task_parameters;
		ESP_ERROR_CHECK( create_ssh_task_input( (ssh_task_input_t *)&task_parameters, (char *)buff ) );

		// Execute ssh command
		xTaskCreate(&ssh_task, "SSH", 1024*8, (void *) &task_parameters, 2, NULL);

		// Wait for ssh task to finish.
		xEventGroupWaitBits( task_parameters.xEventGroup,
			SSH_TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
			pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
			pdFALSE,			/* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);		/* Wait forever. */

		// Check if failed
		EventBits_t flags = xEventGroupGetBits(task_parameters.xEventGroup);
		if( flags & SSH_TASK_FAIL_BIT ) { ESP_LOGE(TAG,"SSH task failed!"); }
		else 							{ ESP_LOGI(TAG,"SSH task finished successfully!"); }

		// Delete the input
		ESP_ERROR_CHECK( delete_ssh_task_input( (ssh_task_input_t *)&task_parameters ) );

		// Set to wakeup on pattern
		display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
	}
}