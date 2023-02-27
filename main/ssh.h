#ifndef _SSH_TASK_H
#define _SSH_TASK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#define SSH_TASK_FINISH_BIT BIT4

extern EventGroupHandle_t xEventGroup;

#endif
