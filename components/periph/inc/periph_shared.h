#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"

#define ESP_INTR_FLAG_DEFAULT 0

typedef struct dio_evt_t {
    bool level;
    TickType_t tick;
    uint32_t event_pin;
} dio_evt_t;

#ifdef __cplusplus
}
#endif