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

#include "wifi_event.h"
#include "input.h"
#include "ssh.h"

static const char *TAG = "MAIN";

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

void ssh_task(void *pvParameters);


void app_main(void)
{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Connect to wifi
	wifi_init_sta();

	// Create Eventgroup
	xEventGroup = xEventGroupCreate();
	configASSERT( xEventGroup );

	char buff[1024];
	while(1){
		
		// Get the command
		esp_err_t err = get_command(buff,1024);
		if (err != ESP_OK){
			ESP_LOGE(TAG,"Error: %s",esp_err_to_name(err));
		}

		// Execute ssh command
		xEventGroupClearBits( xEventGroup, SSH_TASK_FINISH_BIT );
		xTaskCreate(&ssh_task, "SSH", 1024*8, (void *) buff, 2, NULL);

		// Wit for ssh finish.
		xEventGroupWaitBits( xEventGroup,
			SSH_TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
			pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
			pdFALSE,			/* Don't wait for both bits, either bit will do. */
			portMAX_DELAY);		/* Wait forever. */	
	}
	ESP_LOGI(TAG, "SSH all finish");

}
