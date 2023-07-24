#include "msg_handler.h"

#include <string.h>
#include <stdbool.h>
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_peripherals.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "periph_button.h"

#include "audio_element.h"
#include "i2s_stream.h"
#include "board.h"
#include "display_helper.h"
#include "audio_idf_version.h"

#include "display_service.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "periph_adc_button.h"

#include "ssh.h"
#include "display_helper.h"

static const char *TAG = "BUTTON_HANDLER";

typedef enum speaker_mode_t {
    bluetooth_mode = 1,
    ssh_mode = 0,
} speaker_mode_t;
EventGroupHandle_t xEventGroup;

void run_ssh_task_blocked(char * command) {
    // Create input for ssh task
    ssh_task_input_t task_parameters;
    ESP_ERROR_CHECK( create_ssh_task_input((ssh_task_input_t *)&task_parameters, (char *)command));

    // Execute ssh command
    xTaskCreate(&ssh_task, "SSH", 1024 * 8, (void *)&task_parameters, 2, NULL);

    // Wait for ssh task to finish.
    xEventGroupWaitBits(task_parameters.xEventGroup,
                        SSH_TASK_FINISH_BIT, /* The bits within the event group to wait for. */
                        pdTRUE,              /* SSH_TASK_FINISH_BIT should be cleared before returning. */
                        pdFALSE,             /* Don't wait for both bits, either bit will do. */
                        portMAX_DELAY);      /* Wait forever. */

    // Check if failed
    EventBits_t flags = xEventGroupGetBits(task_parameters.xEventGroup);
    if (flags & SSH_TASK_FAIL_BIT) {
        ESP_LOGE(TAG, "SSH task failed!");
    } else {
        ESP_LOGI(TAG, "SSH task finished successfully!");
    }

    // Delete the input
    ESP_ERROR_CHECK(delete_ssh_task_input((ssh_task_input_t *)&task_parameters));
}

esp_err_t msg_handler(audio_event_iface_msg_t msg)
{
    esp_err_t ret = ESP_OK;
    static speaker_mode_t mode = ssh_mode;
    static bool recording = false;
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, mode);

    if ((msg.source_type == PERIPH_ID_TOUCH || msg.source_type == PERIPH_ID_BUTTON || msg.source_type == PERIPH_ID_ADC_BTN)
        && (msg.cmd == PERIPH_TOUCH_TAP || msg.cmd == PERIPH_BUTTON_PRESSED || msg.cmd == PERIPH_ADC_BUTTON_PRESSED)) {

            ESP_LOGI(TAG,"%d msg data: %d",msg.cmd,(int)msg.data);

            if ((int) msg.data == get_input_play_id()) {
                ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
                // Set to wakeup on pattern
                display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

                // Create input for ssh task
                ssh_task_input_t task_parameters;
                char * command = "qm list";
                ESP_ERROR_CHECK( create_ssh_task_input( (ssh_task_input_t *)&task_parameters, (char *)command ) );

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
            } else if ((int) msg.data == get_input_set_id()) {
                ESP_LOGI(TAG, "[ * ] [Set] touch tap event");
                // periph_bluetooth_pause((*bt_periph_shared));
            } else if ((int) msg.data == get_input_volup_id()) {
                ESP_LOGI(TAG, "[ * ] [Vol+] touch tap event");
                // periph_bluetooth_next((*bt_periph_shared));
            } else if ((int) msg.data == get_input_voldown_id()) {
                ESP_LOGI(TAG, "[ * ] [Vol-] touch tap event");
                // Set to wakeup on pattern
                display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

                // Run ssh task
                run_ssh_task_blocked("qm start 101");

                // Set to wakeup on pattern
                display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);

            } else if ((int) msg.data == get_input_mode_id()) {
                ESP_LOGI(TAG,"[ * ] [Mode] touch tap event");
                // Set to wakeup on pattern
                display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

                // Run ssh task
                run_ssh_task_blocked("qm start 105");

                // Set to wakeup on pattern
                display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
            } else if ((int) msg.data == get_input_rec_id()) {
                ESP_LOGI(TAG,"[ * ] [Rec] touch tap event");
                recording = !recording;
                if(recording){
                    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_RECORDING_START, 100);
                } else {
                    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_RECORDING_STOP, 100);
                }
            }
    }

    return ret;
}