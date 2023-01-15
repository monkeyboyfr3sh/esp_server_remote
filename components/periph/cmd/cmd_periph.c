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
#include "button_gpio.h"

static const char *TAG = "cmd_periph";

/** Arguments used by 'join' function */
static struct {
    struct arg_int *fancy_button_number;
    struct arg_end *end;
} fancy_button_push_args;

static int fancy_push(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &fancy_button_push_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, fancy_button_push_args.end, argv[0]);
        return 1;
    }
    if(fancy_button_push_args.fancy_button_number->count){
        return fancy_evt_queue_push(fancy_button_push_args.fancy_button_number->ival[0]);
    } else {
        ESP_LOGW(TAG,"No button number!");
        return ESP_ERR_INVALID_ARG;
    }
}

void register_fancy_button_push(void)
{
    fancy_button_push_args.fancy_button_number = arg_int0("n", "button_number", "<pin#>", "Button number to push");
    fancy_button_push_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "fancy_push",
        .help = "Pushes button_number into fancy event queue to simulate hw trigger",
        .hint = NULL,
        .func = &fancy_push,
        .argtable = &fancy_button_push_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


void register_periph(void)
{
    register_fancy_button_push();
}