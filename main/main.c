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
	init_display_helper();
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_TURN_ON , 0);

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // ESP_LOGI(TAG, "[4.4] Initialize msg handler");
    // init_msg_handler(&led_periph);

	// Connect to wifi
	wifi_init_sta();

	// Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);

	// Initialize an input task
	xTaskCreate(&stdin_input_task, "input", 1024*8, NULL, 2, NULL);

	// Now kill main thread
	vTaskDelete(NULL);
}
