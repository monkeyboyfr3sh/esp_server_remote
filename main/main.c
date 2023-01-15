#include <stdio.h>
#include <string.h>

#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "ssh.h"
#include "terminal.h"
#include "button_handler.h"
#include "connect.h"

static const char* TAG = "main";

void app_main(void)
{
    // initialize_nvs();

	// ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	// wifi_init_sta();

    xTaskCreate(terminal_task, "terminal_thread", 4096, NULL, 5, NULL);
    // xTaskCreate(gpio_task_example, "gpio_thread", 4096, NULL, 5, NULL);
    xTaskCreate(fancy_button_task, "fancy_button_thread", 4096, NULL, 5, NULL);

	// Main exit
	ESP_LOGI(TAG,"Main thread exit");
	vTaskDelete(NULL);
}