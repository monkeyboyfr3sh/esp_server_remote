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

#include "ssh_task.h"
#include "display_helper.h"

static const char *TAG = "BUTTON_HANDLER";

typedef enum speaker_mode_t {
    bluetooth_mode = 1,
    ssh_mode = 0,
} speaker_mode_t;
EventGroupHandle_t xEventGroup;

// Button event handlers
static void handle_play_event();
static void handle_vol_up_event();
static void handle_vol_down_event();
static void handle_mode_event();
static void handle_recording_event(bool recording);



esp_err_t msg_handler(audio_event_iface_msg_t msg)
{
    esp_err_t ret = ESP_OK;
    static speaker_mode_t mode = ssh_mode;
    static bool recording = false;

    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, mode);

    if ((msg.source_type == PERIPH_ID_TOUCH || msg.source_type == PERIPH_ID_BUTTON || msg.source_type == PERIPH_ID_ADC_BTN)
        && (msg.cmd == PERIPH_TOUCH_TAP || msg.cmd == PERIPH_BUTTON_PRESSED || msg.cmd == PERIPH_ADC_BUTTON_PRESSED)) {

        ESP_LOGI(TAG, "%d msg data: %d", msg.cmd, (int)msg.data);
        int8_t data_value = (int8_t)msg.data;

        if (data_value == get_input_play_id()) {
            ESP_LOGI(TAG, "[ * ] [Play] touch tap event");
            handle_play_event();
        } else if (data_value == get_input_set_id()) {
            ESP_LOGI(TAG, "[ * ] [Set] touch tap event");
            // periph_bluetooth_pause((*bt_periph_shared));
        } else if (data_value == get_input_volup_id()) {
            ESP_LOGI(TAG, "[ * ] [Vol+] touch tap event");
            handle_vol_up_event();
        } else if (data_value == get_input_voldown_id()) {
            ESP_LOGI(TAG, "[ * ] [Vol-] touch tap event");
            handle_vol_down_event();
        } else if (data_value == get_input_mode_id()) {
            ESP_LOGI(TAG, "[ * ] [Mode] touch tap event");
            handle_mode_event();
        } else if (data_value == get_input_rec_id()) {
            ESP_LOGI(TAG, "[ * ] [Rec] touch tap event");
            recording = !recording;
            handle_recording_event(recording);
        }
    }

    return ret;
}

void handle_play_event()
{
    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // Run ssh task
    run_ssh_task_blocked("qm list");

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
}

void handle_vol_up_event()
{
    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // Run ssh task
    run_ssh_task_blocked("qm stop 105; qm stop 101;");

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
}

void handle_vol_down_event()
{
    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // Run ssh task
    run_ssh_task_blocked("qm start 101");

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
}

void handle_mode_event()
{
    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_ON, 100);

    // Run ssh task
    run_ssh_task_blocked("qm start 105");

    // Set to wakeup on pattern
    display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_WAKEUP_FINISHED, 100);
}

void handle_recording_event(bool recording)
{
    if (recording) {
        display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_RECORDING_START, 100);
    } else {
        display_service_set_pattern((void *)led_periph, DISPLAY_PATTERN_RECORDING_STOP, 100);
    }
}
