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
#include "button_handler_shared.h"
#include "gpio_example_handler.h"

static const char *TAG = "cmd_btn";

extern char *button_cmd_list[NUM_COMMANDS];
extern int button_cmd_lut[NUM_BUTTONS];

static struct {
    struct arg_int *button_number;
    struct arg_int *command_number;
    struct arg_end *end;
} set_button_command_args;


static int set_button_command(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &set_button_command_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_button_command_args.end, argv[0]);
        return 1;
    }
    
    // Got all the inputs
    if ( (set_button_command_args.button_number->count) && (set_button_command_args.command_number->count) ) {
        int button_num = set_button_command_args.button_number->ival[0];
        int command_num = set_button_command_args.command_number->ival[0];
        ESP_LOGI(TAG,"button: %d, command number: %d", button_num, command_num );
        button_cmd_lut[button_num] = command_num;
    }

    // Missing some inputs
    else {
        if(!set_button_command_args.button_number->count) { ESP_LOGW(TAG,"No button number given!"); }
        if(!set_button_command_args.command_number->count) { ESP_LOGW(TAG,"No command number given!"); }
    }

    return 0;
}

static void register_set_button_command(void)
{
    int num_args = 0;
    set_button_command_args.button_number =
        arg_int0("b", "button_number", "<n>", "Button number");
    num_args += 1;
    set_button_command_args.command_number =
        arg_int0("c", "command_number", "<n>", "Command number");
    num_args += 1;
    set_button_command_args.end = arg_end(num_args);

    const esp_console_cmd_t cmd = {
        .command = "set_button_command",
        .help = "Set command number for specified button",
        .hint = NULL,
        .func = &set_button_command,
        .argtable = &set_button_command_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

static int print_command_list(int argc, char **argv)
{
    ESP_LOGI(TAG,"Command List:");
    for(int i = 0;i<NUM_COMMANDS;i++)
    {
        if(button_cmd_list[i]){
            ESP_LOGI(TAG,"cmd [%d]: %s",i,button_cmd_list[i]);
        }
    }

    ESP_LOGI(TAG,"Button LUT:");
    for(int i = 0;i<NUM_BUTTONS;i++)
    {
        ESP_LOGI(TAG,"LUT[%d]: %d",i,button_cmd_lut[i]);
    }

    return 0;
}

static void register_print_command_list(void)
{
    const esp_console_cmd_t cmd = {
        .command = "print_command_list",
        .help = "Print command list",
        .hint = NULL,
        .func = &print_command_list,
        .argtable = NULL
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

void register_button_handler(void)
{
    register_set_button_command();
    register_print_command_list();
}