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

#define WDT_PERIOD_TICKS	100
#define CHAR_PRINT_LOW		32
#define CHAR_PRINT_HIGH		126
#define CHAR_BS				8
#define CHAR_SPACE			32
#define CHAR_ENTER			10

esp_err_t get_command(char* buff, int buff_len)
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
