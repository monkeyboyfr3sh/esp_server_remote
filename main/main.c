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

#include "display_helper.h"
// #include "msg_handler.h"

static const char *TAG = "MAIN";

// static void SPIFFS_Directory(char * path) {
// 	DIR* dir = opendir(path);
// 	assert(dir != NULL);
// 	while (true) {
// 		struct dirent*pe = readdir(dir);
// 		if (!pe) break;
// 		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
// 	}
// 	closedir(dir);
// }

void app_main(void)

{
	//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

    // Initialize display and set ot turn on mode
    ESP_LOGI(TAG, "[ 0 ] Initialize display peripheral");
    display_service_handle_t led_periph =   my_audio_board_init();
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_TURN_ON , 0);

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // ESP_LOGI(TAG, "[4.4] Initialize msg handler");
    // init_msg_handler(&led_periph);


	// Connect to wifi
	wifi_init_sta();

	// Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);

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

	ESP_LOGI(TAG, "SSH all finish");
}
