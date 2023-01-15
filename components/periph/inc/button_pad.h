#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define BUTTON_PAD_1_PIN  GPIO_NUM_32
#define BUTTON_PAD_2_PIN  GPIO_NUM_33
#define BUTTON_PAD_3_PIN  GPIO_NUM_25
#define BUTTON_PAD_4_PIN  GPIO_NUM_26
#define BUTTON_PAD_5_PIN  GPIO_NUM_27

extern QueueHandle_t button_pad_evt_queue;

esp_err_t setup_button_pad(void);
esp_err_t button_pad_evt_queue_push(bool level, TickType_t tick, uint32_t event_pin);

#ifdef __cplusplus
}
#endif
