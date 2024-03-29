#include "button_pad.h"

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_err.h"
#include "periph_shared.h"

#include "freertos/queue.h"
#include "driver/gpio.h"

static const char *TAG = "BTN-GPIO";

QueueHandle_t button_pad_evt_queue = NULL;

static void IRAM_ATTR fancy_isr_handler(void* arg)
{
    // Create input
    uint32_t gpio_num = (uint32_t) arg;
    dio_evt_t dio_evt = {
        .level = gpio_get_level(gpio_num),
        .tick = xTaskGetTickCount(),
        .event_pin = gpio_num,
    };
    // Now push into queue (with isr priority)
    xQueueSendFromISR(button_pad_evt_queue, ( void* )&dio_evt, NULL);
}

esp_err_t setup_button_pad(void)
{
    // Initialize GPIO
	gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;

    io_conf.pin_bit_mask = (1ULL << BUTTON_PAD_3_PIN );
    esp_err_t ret = gpio_config(&io_conf);
    if( ret!=ESP_OK ){
        ESP_LOGW(TAG,"Failed config [%d]. err: '%s'",BUTTON_PAD_3_PIN, esp_err_to_name(ret));
        return ret;
    }
	io_conf.pin_bit_mask = (1ULL << BUTTON_PAD_5_PIN );
    ret = gpio_config(&io_conf);
    if( ret!=ESP_OK ){
        ESP_LOGW(TAG,"Failed config [%d]. err: '%s'",BUTTON_PAD_5_PIN, esp_err_to_name(ret));
        return ret;
    }

    //create a queue to handle gpio event from isr
    ESP_LOGI(TAG,"Creating button pad event queue");
    button_pad_evt_queue = xQueueCreate(10, sizeof(dio_evt_t));
    if(!button_pad_evt_queue){
        ESP_LOGW(TAG,"Failed to create button pad queue!");
        return ESP_FAIL;
    }

    //install gpio isr service
    ret = gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    if( ret!=ESP_OK ){
        ESP_LOGW(TAG,"Failed to install gpio isr service. err: '%s'",esp_err_to_name(ret));
        return ret;
    }
    //hook isr handler for specific gpio pin
    ret = gpio_isr_handler_add(BUTTON_PAD_3_PIN, fancy_isr_handler, (void*) BUTTON_PAD_3_PIN);
    if( ret!=ESP_OK ){
        ESP_LOGW(TAG,"Failed to add handle for [%d] to gpio isr. err: '%s'",BUTTON_PAD_3_PIN, esp_err_to_name(ret));
        return ret;
    }
    gpio_isr_handler_add(BUTTON_PAD_5_PIN, fancy_isr_handler, (void*) BUTTON_PAD_5_PIN);
    if( ret!=ESP_OK ){
        ESP_LOGW(TAG,"Failed to add handle for [%d] to gpio isr. err: '%s'",BUTTON_PAD_5_PIN, esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

esp_err_t button_pad_evt_queue_push(bool level, TickType_t tick, uint32_t event_pin)
{
    // Create input
    dio_evt_t dio_evt = {
        .level = level,
        .tick = tick,
        .event_pin = event_pin,
    };
    // Now push into queue
    if( xQueueSendToBack(button_pad_evt_queue, ( void* )&dio_evt, portMAX_DELAY) ){
        return ESP_OK;
    } else {
        ESP_LOGW(TAG,"Failed to queue item");
        return ESP_FAIL;
    }
}