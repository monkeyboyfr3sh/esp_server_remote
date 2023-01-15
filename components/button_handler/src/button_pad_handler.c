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

void button_pad_task(void* arg)
{
	ESP_LOGI(TAG,"Fancy button enters!");
	setup_button_pad();

	const TickType_t button_tick_goal = pdMS_TO_TICKS(INTERRUPT_MIN_PERIOD_MS);
	TickType_t button_1_timestamp = 0;
	TickType_t button_2_timestamp = 0;
	TickType_t button_3_timestamp = 0;
	TickType_t button_4_timestamp = 0;
	TickType_t button_5_timestamp = 0;

	// while(1)
	// {
	// 	uint8_t btn3 = gpio_get_level(BUTTON_PAD_3_PIN);
	// 	uint8_t btn4 = gpio_get_level(BUTTON_PAD_4_PIN);
	// 	uint8_t btn5 = gpio_get_level(BUTTON_PAD_5_PIN);
	// 	ESP_LOGI(TAG,"%d %d %d",btn3,btn4,btn5);
	// 	vTaskDelay(pdMS_TO_TICKS(200));
	// }

    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(button_pad_evt_queue, &io_num, portMAX_DELAY)) {

			switch (io_num)
			{
			case BUTTON_PAD_3_PIN:
				if( (xTaskGetTickCount()-button_3_timestamp) > button_tick_goal )
				{
					button_3_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 3!");
				
					// xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;
			case BUTTON_PAD_4_PIN:
				if( (xTaskGetTickCount()-button_4_timestamp) > button_tick_goal )
				{
					button_4_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 4!");
				
					// xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;
			case BUTTON_PAD_5_PIN:
				if( (xTaskGetTickCount()-button_5_timestamp) > button_tick_goal )
				{
					button_5_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 5!");
				
					// xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;
			
			default:
				ESP_LOGW(TAG,"err, PIN: %d",io_num);
				break;
			}
        }

		vTaskDelay(1);
    }

	ESP_LOGI(TAG,"Fancy button exits!");
	vTaskDelete(NULL);
}