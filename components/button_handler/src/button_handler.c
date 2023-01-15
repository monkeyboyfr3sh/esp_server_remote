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

#include "ssh.h"
#include "button_handler.h"
#include "periph.h"
#include "button_pad.h"

static const char *TAG = "BTN-HANDLER";

char *button_cmd_list[NUM_COMMANDS];
int button_cmd_lut[NUM_BUTTONS];

#define DEBUG_COMMAND_1 "qm list"

static void set_default_commands(void)
{
	// Default commands to set
	char * default_commands[NUM_COMMANDS] = {
		"qm list",
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



void gpio_task_example(void* arg)
{

	setup_gpio();

	// Initialize command list
	for(int i = 0;i<NUM_COMMANDS;i++){ button_cmd_list[i] = malloc( COMMAND_MAX_SIZE * sizeof(char) ); }
	set_default_commands();

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