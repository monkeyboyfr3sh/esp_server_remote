#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_system.h"
#include "esp_err.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "sdkconfig.h"
#include "periph.h"

static const char *TAG = "cmd_periph";

void register_periph(void)
{
}