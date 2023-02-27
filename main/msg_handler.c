#include "msg_handler.h"

#include <string.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_peripherals.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "periph_button.h"
#include "esp_bt_defs.h"
#include "esp_gap_bt_api.h"
#include "esp_hf_client_api.h"

#include "audio_element.h"

#include "i2s_stream.h"
#include "board.h"
#include "bluetooth_service.h"

#include "display_helper.h"

#include "audio_idf_version.h"

#include "esp_err.h"
#include "display_service.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "periph_adc_button.h"

#include "ssh.h"

static const char *TAG = "BUTTON_HANDLER";

// static esp_periph_handle_t * bt_periph_shared;
static display_service_handle_t * led_periph_shared;

typedef enum speaker_mode_t {
    bluetooth_mode = 0,
    ssh_mode = 1,
} speaker_mode_t;
EventGroupHandle_t xEventGroup;

esp_err_t init_msg_handler(display_service_handle_t * led_periph)
{
    // Verify valid input
    if( led_periph == NULL ) {
        return ESP_ERR_INVALID_ARG;
    }
    // Update shared variables
    led_periph_shared = led_periph;
    return ESP_OK;
}

esp_err_t msg_handler(audio_event_iface_msg_t msg)
{
    esp_err_t ret = ESP_OK;
    static speaker_mode_t mode = bluetooth_mode;
    static bool recording = false;

    if ((msg.source_type == PERIPH_ID_TOUCH || msg.source_type == PERIPH_ID_BUTTON || msg.source_type == PERIPH_ID_ADC_BTN)
        && (msg.cmd == PERIPH_TOUCH_TAP || msg.cmd == PERIPH_BUTTON_PRESSED || msg.cmd == PERIPH_ADC_BUTTON_PRESSED)) {

            switch (mode)
            {
            case bluetooth_mode:
                if ((int) msg.data == get_input_play_id()) {
                    ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
                    // periph_bluetooth_play((*bt_periph_shared));
                } else if ((int) msg.data == get_input_set_id()) {
                    ESP_LOGI(TAG, "[ * ] [Set] touch tap event");
                    // periph_bluetooth_pause((*bt_periph_shared));
                } else if ((int) msg.data == get_input_volup_id()) {
                    ESP_LOGI(TAG, "[ * ] [Vol+] touch tap event");
                    // periph_bluetooth_next((*bt_periph_shared));
                } else if ((int) msg.data == get_input_voldown_id()) {
                    ESP_LOGI(TAG, "[ * ] [Vol-] touch tap event");
                    // periph_bluetooth_prev((*bt_periph_shared));
                } else if ((int) msg.data == get_input_mode_id()) {
                    ESP_LOGI(TAG,"[ * ] [Mode] touch tap event");
                    ESP_LOGI(TAG,"[ * ] Switching mode to ssh mode");
                    mode = ssh_mode;
                    display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_WAKEUP_FINISHED, mode);
                } else if ((int) msg.data == get_input_rec_id()) {
                    ESP_LOGI(TAG,"[ * ] [Rec] touch tap event");
                    recording = !recording;
                    if(recording){
                        display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_RECORDING_START, 100);
                    } else {
                        display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_RECORDING_STOP, 100);
                    }
                }
                break;
            case ssh_mode:
                if ((int) msg.data == get_input_play_id()) {
                    ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
                    // // Put them into the input struct
                    // ssh_cmd_input_t ssh_command;
                    // ssh_command.username = "root";
                    // ssh_command.host = "192.168.0.15";
                    // ssh_command.command = "qm list";
                    // ssh_command.password = "openDoor";

                    // // Execute ssh command
                    // xEventGroupClearBits( xEventGroup, TASK_FINISH_BIT );
                    // xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);

                    // // Wit for ssh finish.
                    // xEventGroupWaitBits( xEventGroup,
                    //     TASK_FINISH_BIT,	/* The bits within the event group to wait for. */
                    //     pdTRUE,				/* HTTP_CLOSE_BIT should be cleared before returning. */
                    //     pdFALSE,			/* Don't wait for both bits, either bit will do. */
                    //     portMAX_DELAY);		/* Wait forever. */	
                } else if ((int) msg.data == get_input_set_id()) {
                    ESP_LOGI(TAG, "[ * ] [Set] touch tap event");
                    // periph_bluetooth_pause((*bt_periph_shared));
                } else if ((int) msg.data == get_input_volup_id()) {
                    ESP_LOGI(TAG, "[ * ] [Vol+] touch tap event");
                    // periph_bluetooth_next((*bt_periph_shared));
                } else if ((int) msg.data == get_input_voldown_id()) {
                    ESP_LOGI(TAG, "[ * ] [Vol-] touch tap event");
                    // periph_bluetooth_prev((*bt_periph_shared));
                } else if ((int) msg.data == get_input_mode_id()) {
                    ESP_LOGI(TAG,"[ * ] [Mode] touch tap event");
                    ESP_LOGI(TAG,"[ * ] Switching mode to bluetooth mode");
                    mode = bluetooth_mode;
                    display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_WAKEUP_FINISHED, mode);
                } else if ((int) msg.data == get_input_rec_id()) {
                    ESP_LOGI(TAG,"[ * ] [Rec] touch tap event");
                    recording = !recording;
                    if(recording){
                        display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_RECORDING_START, 100);
                    } else {
                        display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_RECORDING_STOP, 100);
                    }
                }
                break;
            default:
                break;
            }
    }

    // if (msg.source_type == PERIPH_ID_BLUETOOTH
    //     && msg.source == (void *)(*bt_periph_shared)) 
    // {
    //     /* Stop when the Bluetooth is disconnected or suspended */
    //     if (msg.cmd == PERIPH_BLUETOOTH_DISCONNECTED) {
    //         ESP_LOGI(TAG, "[ * ] Bluetooth disconnected");
    //         // Set to wakeup on pattern
    //         display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_BT_DISCONNECTED, mode);
    //     }
    //     /* Bluetooth is connected */
    //     else if (msg.cmd == PERIPH_BLUETOOTH_CONNECTED) {
    //         ESP_LOGI(TAG, "[ * ] Bluetooth connected");
    //         // Set to wakeup on pattern
    //         display_service_set_pattern((void *)(*led_periph_shared), DISPLAY_PATTERN_BT_CONNECTED, mode);
    //     }
    // }

    return ret;
}