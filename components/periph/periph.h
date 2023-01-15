#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTON_1_PIN	17
#define BUTTON_2_PIN	5
#define BUTTON_3_PIN	18
#define BUTTON_4_PIN	19
#define BUTTON_5_PIN	16

#define FANCY_BUTTON_1_PIN  GPIO_NUM_32
#define FANCY_BUTTON_2_PIN  GPIO_NUM_33
#define FANCY_BUTTON_3_PIN  GPIO_NUM_25
#define FANCY_BUTTON_4_PIN  GPIO_NUM_26
#define FANCY_BUTTON_5_PIN  GPIO_NUM_27

extern QueueHandle_t gpio_evt_queue;
extern QueueHandle_t fancy_evt_queue;

void setup_gpio(void);

void setup_fancy_button(void);
esp_err_t fancy_evt_queue_push(uint32_t item);

#ifdef __cplusplus
}
#endif