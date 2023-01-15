#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTON_1_PIN	17
#define BUTTON_2_PIN	5
#define BUTTON_3_PIN	18
#define BUTTON_4_PIN	19
#define BUTTON_5_PIN	16

extern QueueHandle_t gpio_evt_queue;

void setup_gpio(void);

#ifdef __cplusplus
}
#endif