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
#include "periph.h"

#include "freertos/queue.h"
#include "driver/gpio.h"

static const char *TAG = "BTN-GPIO";

QueueHandle_t button_pad_evt_queue = NULL;

static void IRAM_ATTR fancy_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(button_pad_evt_queue, &gpio_num, NULL);
}

void setup_button_pad(void)
{
	gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;

    io_conf.pin_bit_mask = (1ULL << BUTTON_PAD_3_PIN );
    gpio_config(&io_conf);
	io_conf.pin_bit_mask = (1ULL << BUTTON_PAD_5_PIN );
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    button_pad_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_PAD_3_PIN, fancy_isr_handler, (void*) BUTTON_PAD_3_PIN);
    gpio_isr_handler_add(BUTTON_PAD_5_PIN, fancy_isr_handler, (void*) BUTTON_PAD_5_PIN);
}

esp_err_t fancy_evt_queue_push(uint32_t item)
{
    uint32_t item_push = (uint32_t) item;
    if( xQueueSendToBack(button_pad_evt_queue, &item_push, portMAX_DELAY) ){
        return ESP_OK;
    } else {
        ESP_LOGW(TAG,"Failed to queue item");
        return ESP_FAIL;
    }
}