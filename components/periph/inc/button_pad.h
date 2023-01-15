#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

void setup_button_pad(void);
esp_err_t fancy_evt_queue_push(uint32_t item);

#ifdef __cplusplus
}
#endif
