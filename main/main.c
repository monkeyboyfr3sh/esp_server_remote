/* SSH Client Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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

#include "lwip/err.h"
#include "lwip/sys.h"


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT	   BIT1

static const char *TAG = "MAIN";

static int s_retry_num = 0;

EventGroupHandle_t xEventGroup;
int TASK_FINISH_BIT	= BIT4;

static void event_handler(void* arg, esp_event_base_t event_base,
								int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
		if (s_retry_num < CONFIG_ESP_MAXIMUM_RETRY) {
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(TAG, "retry to connect to the AP");
		} else {
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(TAG,"connect to the AP fail");
	} else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

static void SPIFFS_Directory(char * path) {
	DIR* dir = opendir(path);
	assert(dir != NULL);
	while (true) {
		struct dirent*pe = readdir(dir);
		if (!pe) break;
		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
	}
	closedir(dir);
}

void wifi_init_sta(void)
{
	s_wifi_event_group = xEventGroupCreate();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_create_default_wifi_sta();

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	esp_event_handler_instance_t instance_any_id;
	esp_event_handler_instance_t instance_got_ip;
	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
					ESP_EVENT_ANY_ID,
					&event_handler,
					NULL,
					&instance_any_id));
	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
					IP_EVENT_STA_GOT_IP,
					&event_handler,
					NULL,
					&instance_got_ip));

	wifi_config_t wifi_config = {
		.sta = {
			.ssid = CONFIG_ESP_WIFI_SSID,
			.password = CONFIG_ESP_WIFI_PASSWORD,
			/* Setting a password implies station will connect to all security modes including WEP/WPA.
			 * However these modes are deprecated and not advisable to be used. Incase your Access point
			 * doesn't support WPA2, these mode can be enabled by commenting below line */
		 .threshold.authmode = WIFI_AUTH_WPA2_PSK,

			.pmf_cfg = {
				.capable = true,
				.required = false
			},
		},
	};
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");

	/* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
	 * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
	EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
			WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
			pdFALSE,
			pdFALSE,
			portMAX_DELAY);

	/* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
	 * happened. */
	if (bits & WIFI_CONNECTED_BIT) {
		ESP_LOGI(TAG, "connected to ap SSID:%s",
				 CONFIG_ESP_WIFI_SSID);
	} else if (bits & WIFI_FAIL_BIT) {
		ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
				 CONFIG_ESP_WIFI_SSID, CONFIG_ESP_WIFI_PASSWORD);
	} else {
		ESP_LOGE(TAG, "UNEXPECTED EVENT");
	}

	/* The event will not be processed after unregister */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
	vEventGroupDelete(s_wifi_event_group);
}

void ssh_task(void *pvParameters);

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

void app_main(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();

	// Create Eventgroup
	xEventGroup = xEventGroupCreate();
	configASSERT( xEventGroup );

	// int num_commands = 3;
	// char *commands[] = {
	// 	"qm list",
	// 	"qm start 103",
	// 	"qm list",
	// };
	// ESP_LOGI(TAG,"Num commands: %d",num_commands);

	// for(int i = 0;i<num_commands;i++)
	// {
	// 	// Execute ssh command
	// 	xEventGroupClearBits( xEventGroup, TASK_FINISH_BIT );
	// 	xTaskCreate(&ssh_task, "SSH", 1024*8, (void *) commands[i], 2, NULL);

	// 	// Wit for ssh finish.
	// 	xEventGroupWaitBits( xEventGroup,
	// 		TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
	// 		pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
	// 		pdFALSE,			/* Don't wait for both bits, either bit will do. */
	// 		portMAX_DELAY);		/* Wait forever. */	
	// }

	char buff[1024];
	while(1){
		
		// Get the command
		esp_err_t err = get_command(buff,1024);
		if (err != ESP_OK){
			ESP_LOGE(TAG,"Error: %s",esp_err_to_name(err));
		}

		// Execute ssh command
		xEventGroupClearBits( xEventGroup, TASK_FINISH_BIT );
		xTaskCreate(&ssh_task, "SSH", 1024*8, (void *) buff, 2, NULL);

		// Wit for ssh finish.
		xEventGroupWaitBits( xEventGroup,
			TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
			pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
			pdFALSE,			/* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);		/* Wait forever. */	
	}
	ESP_LOGI(TAG, "SSH all finish");

}
