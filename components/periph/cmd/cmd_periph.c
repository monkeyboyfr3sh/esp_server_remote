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

#include "periph_shared.h"
#include "button_pad.h"

static const char *TAG = "cmd_periph";

/** Arguments used by 'join' function */
static struct {
    struct arg_int *level;
    struct arg_int *tick;
    struct arg_int *event_pin;
    struct arg_end *end;
} button_pad_push_args;

static int button_pad_push(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &button_pad_push_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, button_pad_push_args.end, argv[0]);
        return 1;
    }

    // Check for empty input    
    if( (!button_pad_push_args.level->count) || (!button_pad_push_args.tick->count) || (!button_pad_push_args.event_pin->count) ){
        if( !button_pad_push_args.level->count ){ ESP_LOGW(TAG,"No level given!"); }
        if( !button_pad_push_args.tick->count ){ ESP_LOGW(TAG,"No tick given!"); }
        if( !button_pad_push_args.event_pin->count ){ ESP_LOGW(TAG,"No event pin given!"); }
        return ESP_ERR_INVALID_ARG;
    } else {
        return button_pad_evt_queue_push(button_pad_push_args.level->ival[0], button_pad_push_args.tick->ival[0], button_pad_push_args.event_pin->ival[0]);
    }
}

void register_button_pad_push(void)
{
    button_pad_push_args.level = arg_int0("l", "level", "<0/1>", "Level for event push");
    button_pad_push_args.tick = arg_int0("t", "tick", "<u32>", "Tick for event push");
    button_pad_push_args.event_pin = arg_int0("e", "event_pin", "<u32>", "Pin number for event push");
    button_pad_push_args.end = arg_end(2);

    const esp_console_cmd_t cmd = {
        .command = "button_pad_push",
        .help = "Pushes event_details into button_pad_evt_queue",
        .hint = NULL,
        .func = &button_pad_push,
        .argtable = &button_pad_push_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}


void register_periph(void)
{
    register_button_pad_push();
}