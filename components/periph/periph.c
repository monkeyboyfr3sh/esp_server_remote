/* ssh Client Example

	 This example code is in the Public Domain (or CC0 licensed, at your option.)

	 Unless required by applicable law or agreed to in writing, this
	 software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	 CONDITIONS OF ANY KIND, either express or implied.
*/
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
#include "periph.h"

#include "freertos/queue.h"
#include "driver/gpio.h"

#include "ssh.h"
#include "periph.h"

static const char *TAG = "HW";

#define INTERRUPT_MIN_PERIOD_MS		1000

#define ESP_INTR_FLAG_DEFAULT 0
#define BUTTON_1_PIN	17
#define BUTTON_2_PIN	5
#define BUTTON_3_PIN	18
#define BUTTON_4_PIN	19
#define BUTTON_5_PIN	16

static QueueHandle_t gpio_evt_queue = NULL;

char *button_cmd_list[NUM_COMMANDS];
int button_cmd_lut[NUM_BUTTONS];

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void set_default_commands(void)
{
	// Default commands to set
	char * default_commands[NUM_COMMANDS] = {
		// 101
		"qm start 101",
		"qm stop 101",
		// 102
		"qm start 102",
		"qm stop 102",
		// 103
		"qm start 103",
		"qm stop 103",
		// 104
		"qm start 104",
		"qm stop 104",
		// 105
		"qm start 105",
		"qm stop 105",
		
		"qm list",
	};

	// Copy commands in master list
	for(int i = 0;i<NUM_COMMANDS;i++)
	{
		if(default_commands[i]) { strncpy(button_cmd_list[i],default_commands[i],COMMAND_MAX_SIZE); }
		else { memset(button_cmd_list[i],0,COMMAND_MAX_SIZE); }
	}

	// Default LUT indexes
	int default_LUT_values[NUM_BUTTONS] = {
		0, 1, 8, 9, 10
	};

	// Assign command LUT 
	for(int i = 0;i<NUM_BUTTONS;i++)
	{
		button_cmd_lut[i] = default_LUT_values[i];
	}
}

static void setup_gpio(void)
{
	gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
	io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    io_conf.pin_bit_mask = (1ULL << BUTTON_1_PIN );
    gpio_config(&io_conf);
	io_conf.pin_bit_mask = (1ULL << BUTTON_2_PIN );
    gpio_config(&io_conf);
    io_conf.pin_bit_mask = (1ULL << BUTTON_3_PIN );
    gpio_config(&io_conf);
	io_conf.pin_bit_mask = (1ULL << BUTTON_4_PIN );
    gpio_config(&io_conf);
	io_conf.pin_bit_mask = (1ULL << BUTTON_5_PIN );
    gpio_config(&io_conf);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(BUTTON_1_PIN, gpio_isr_handler, (void*) BUTTON_1_PIN);
    gpio_isr_handler_add(BUTTON_2_PIN, gpio_isr_handler, (void*) BUTTON_2_PIN);
    gpio_isr_handler_add(BUTTON_3_PIN, gpio_isr_handler, (void*) BUTTON_3_PIN);
    gpio_isr_handler_add(BUTTON_4_PIN, gpio_isr_handler, (void*) BUTTON_4_PIN);
    gpio_isr_handler_add(BUTTON_5_PIN, gpio_isr_handler, (void*) BUTTON_5_PIN);

	// Initialize command list
	for(int i = 0;i<NUM_COMMANDS;i++){ button_cmd_list[i] = malloc( COMMAND_MAX_SIZE * sizeof(char) ); }
	set_default_commands();

}

void gpio_task_example(void* arg)
{

	setup_gpio();

	TickType_t button_tick_goal = pdMS_TO_TICKS(INTERRUPT_MIN_PERIOD_MS);
	TickType_t button_1_timestamp = 0;
	TickType_t button_2_timestamp = 0;
	TickType_t button_3_timestamp = 0;
	TickType_t button_4_timestamp = 0;
	TickType_t button_5_timestamp = 0;

    uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {

			switch (io_num)
			{
			case BUTTON_1_PIN:
				if( (xTaskGetTickCount()-button_1_timestamp) > button_tick_goal )
				{
					button_1_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 1!");

					// Put them into the input struct
					ssh_cmd_input_t ssh_command;
					ssh_command.username = "root";
					ssh_command.host = "192.168.0.15";
					int command_index = button_cmd_lut[0];
					ssh_command.command = button_cmd_list[command_index];
					ssh_command.password = "openDoor";
				
					xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;

			case BUTTON_2_PIN:
				if( (xTaskGetTickCount()-button_2_timestamp) > button_tick_goal )
				{
					button_2_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 2!");

					// Put them into the input struct
					ssh_cmd_input_t ssh_command;
					ssh_command.username = "root";
					ssh_command.host = "192.168.0.15";
					int command_index = button_cmd_lut[1];
					ssh_command.command = button_cmd_list[command_index];
					ssh_command.password = "openDoor";
				
					xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;

			case BUTTON_3_PIN:
				if( (xTaskGetTickCount()-button_3_timestamp) > button_tick_goal )
				{
					button_3_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 3!");

					// Put them into the input struct
					ssh_cmd_input_t ssh_command;
					ssh_command.username = "root";
					ssh_command.host = "192.168.0.15";
					int command_index = button_cmd_lut[2];
					ssh_command.command = button_cmd_list[command_index];
					ssh_command.password = "openDoor";
				
					xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;

			case BUTTON_4_PIN:
				if( (xTaskGetTickCount()-button_4_timestamp) > button_tick_goal )
				{
					button_4_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 4!");

					// Put them into the input struct
					ssh_cmd_input_t ssh_command;
					ssh_command.username = "root";
					ssh_command.host = "192.168.0.15";
					int command_index = button_cmd_lut[3];
					ssh_command.command = button_cmd_list[command_index];
					ssh_command.password = "openDoor";
				
					xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;

			case BUTTON_5_PIN:
				if( (xTaskGetTickCount()-button_5_timestamp) > button_tick_goal )
				{
					button_5_timestamp = xTaskGetTickCount();
					ESP_LOGI(TAG,"Interrupt from Button 5!");

					// Put them into the input struct
					ssh_cmd_input_t ssh_command;
					ssh_command.username = "root";
					ssh_command.host = "192.168.0.15";
					int command_index = button_cmd_lut[4];
					ssh_command.command = button_cmd_list[command_index];
					ssh_command.password = "openDoor";
				
					xTaskCreate(&ssh_command_task, "SSH_CMD", 1024*8, (void *) &ssh_command, 2, NULL);
				}
				break;
			
			default:
				break;
			}
        }

		vTaskDelay(1);
    }
}