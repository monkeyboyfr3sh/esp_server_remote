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

#include "esp_log.h"
#include "esp_peripherals.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "periph_button.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"

#include "wifi_event.h"
#include "input.h"

#include "display_helper.h"
#include "msg_handler.h"
#include "ssh_task.h"

static const char *TAG = "MAIN";

void app_main(void)
{

    audio_pipeline_handle_t pipeline;

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

    ESP_LOGI(TAG, "[3.0] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[ 4 ] Initialize peripherals");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    esp_periph_set_handle_t set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[4.1] Initialize keys on board");
    audio_board_key_init(set);

    ESP_LOGI(TAG, "[ 5 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.1] Listening event from the pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[5.2] Listening event from peripherals");
    audio_event_iface_set_listener(esp_periph_set_get_event_iface(set), evt);

    ESP_LOGI(TAG, "[ 6 ] Starting pipeline");
    audio_pipeline_run(pipeline);

	// Connect to wifi
	wifi_init_sta();

	// // Initialize an input task
	// xTaskCreate(&stdin_input_task, "input", 1024*8, NULL, 2, NULL);

    // Set to wakeup finished pattern
    vTaskDelay(pdMS_TO_TICKS(20));
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 0);    

	while (1) {
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Event interface error: '%s'", esp_err_to_name(ret) );
            continue;
        }

		// Handle msg events
        msg_handler(msg);
	}
	// Now kill main thread
	vTaskDelete(NULL);
}
