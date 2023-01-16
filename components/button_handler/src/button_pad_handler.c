#include "button_pad_handler.h"

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

#include "freertos/queue.h"
#include "driver/gpio.h"

#include "periph_shared.h"
#include "button_pad.h"
#include "button_handler_shared.h"

static const char *TAG = "BTN-PAD-H";

static void button_pad_exit(void)
{
	ESP_LOGI(TAG,"Exiting!");
	vTaskDelete(NULL);
}

void button_pad_task(void* arg)
{
	ESP_LOGI(TAG,"Button pad enters!");
	
	// Setup button pad
	esp_err_t ret = setup_button_pad();
	ESP_LOGI(TAG,"Button pad setup status: '%s'",esp_err_to_name(ret));
	if( ret!=ESP_OK ) {
		ESP_LOGE(TAG,"Failed to init button pad");
		button_pad_exit();
	}

	const TickType_t button_tick_goal = pdMS_TO_TICKS(INTERRUPT_MIN_PERIOD_MS);
	TickType_t button_1_timestamp = 0;
	TickType_t button_2_timestamp = 0;
	TickType_t button_3_timestamp = 0;
	TickType_t button_4_timestamp = 0;
	TickType_t button_5_timestamp = 0;
	
    dio_evt_t prev_dio_evt = { 0 };
    dio_evt_t dio_evt = { 0 };
	while(1)
	{
		// Update history
		prev_dio_evt = dio_evt;
        // Capture new event
		if(xQueueReceive(button_pad_evt_queue, &dio_evt, portMAX_DELAY)) {
			ESP_LOGI(TAG,"Event details: level=%d tick=%d pin=%d",dio_evt.level,dio_evt.tick,dio_evt.event_pin);

			switch (dio_evt.event_pin)
			{
			case BUTTON_PAD_3_PIN:
				if( (xTaskGetTickCount()-button_3_timestamp) > button_tick_goal )
				{
					button_3_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 3!");
				
					// DO CODE
				}
				break;
			case BUTTON_PAD_4_PIN:
				if( (xTaskGetTickCount()-button_4_timestamp) > button_tick_goal )
				{
					button_4_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 4!");
				
					// DO CODE
				}
				break;
			case BUTTON_PAD_5_PIN:
				if( (xTaskGetTickCount()-button_5_timestamp) > button_tick_goal )
				{
					button_5_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 5!");
				
					// DO CODE
				}
				break;
			
			default:
				ESP_LOGW(TAG,"err, PIN: %d",dio_evt.event_pin);
				break;
			}
        }

		vTaskDelay(1);
    }

	button_pad_exit();
}