#include "display_helper.h"
#include "esp_types.h"
#include "esp_log.h"
#include "board.h"
#include "audio_mem.h"

#include "periph_sdcard.h"
#include "periph_adc_button.h"
#include "led_bar_is31x.h"

#include <string.h>
#include "led_bar_is31x.h"
#include "periph_is31fl3216.h"

static const char *TAG = "DISPLAY_HELPER";
#define MAX_LED_INDEX       (14)
#define GREEN_LED_INDEX     (13)
#define RED_LED_INDEX       (12)
#define GREEN_LED_BIT_MASK  (1 << GREEN_LED_INDEX)
#define RED_LED_BIT_MASK    (1 << RED_LED_INDEX)
static esp_err_t my_led_bar_is31x_pattern(void *handle, int pat, int value)
{
    esp_err_t ret =  ESP_OK;
    if (handle == NULL) {
        ESP_LOGE(TAG, "led_bar_is31x_pattern instance has not initialized");
        return ESP_FAIL;
    }
    esp_periph_handle_t h = (esp_periph_handle_t)handle;

    switch (pat) {
        case DISPLAY_PATTERN_WIFI_SETTING: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_duty_step(h, 20);
                periph_is31fl3216_set_interval(h, 10);
                periph_is31fl3216_set_act_time(h, 0);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_FLASH);
                break;
            }
        case DISPLAY_PATTERN_WIFI_CONNECTTING: {
                periph_is31fl3216_set_light_on_num(h, 1, BLUE_LED_MAX_NUM);
                periph_is31fl3216_set_interval(h, 100);
                periph_is31fl3216_set_shift_mode(h, PERIPH_IS31_SHIFT_MODE_ACC);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_SHIFT);
                break;
            }
        case DISPLAY_PATTERN_WIFI_CONNECTED: {
                periph_is31fl3216_set_light_on_num(h, 2, BLUE_LED_MAX_NUM);
                periph_is31fl3216_set_interval(h, 150);
                periph_is31fl3216_set_shift_mode(h, PERIPH_IS31_SHIFT_MODE_SINGLE);
                periph_is31fl3216_set_act_time(h, 2500);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_SHIFT);
                break;
            }
        case DISPLAY_PATTERN_WIFI_DISCONNECTED: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_duty_step(h, 20);
                periph_is31fl3216_set_interval(h, 10);
                periph_is31fl3216_set_act_time(h, 0);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_FLASH);
                break;
            }
            break;
        case DISPLAY_PATTERN_WIFI_SETTING_FINISHED: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_OFF);
                break;
            }

        case DISPLAY_PATTERN_BT_CONNECTTING:
            break;
        case DISPLAY_PATTERN_BT_CONNECTED:{
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = DISPLAY_ACTIVE_DUTY_VAL;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                // Turn on one of 4 leds using duty
                for(int i = 0;i<BLUE_LED_MAX_NUM;i++){
                    if(value==0){
                        if( !(i%3) )    { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else if (value==1){
                        if( !((i+1))%3) { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else {
                        if( !((i+2)%3) ){ periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    }
                }
                // Also turn on green LED
                periph_is31fl3216_set_duty(h, GREEN_LED_INDEX, duty); 
                // Turn on the display
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_ON);
                break;
            }
        case DISPLAY_PATTERN_BT_DISCONNECTED: {
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = DISPLAY_IDLE_DUTY_VAL;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                // Turn on one of 4 leds using duty
                for(int i = 0;i<BLUE_LED_MAX_NUM;i++){
                    if(value==0){
                        if( !(i%3) )    { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else if (value==1){
                        if( !((i+1))%3) { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else {
                        if( !((i+2)%3) ){ periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    }
                }
                // Also turn on green LED
                periph_is31fl3216_set_duty(h, GREEN_LED_INDEX, duty); 
                // Turn on the display
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_ON);
                break;
        }
        case DISPLAY_PATTERN_RECORDING_START: {
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = 200;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_ON);
                // Now turn on red
                periph_is31fl3216_set_duty(h, RED_LED_INDEX, duty); 
                break;
            }
        case DISPLAY_PATTERN_RECORDING_STOP: {
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = 0;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_ON);
                // Now turn off red
                periph_is31fl3216_set_duty(h, RED_LED_INDEX, duty); 
                break;
            }

        case DISPLAY_PATTERN_RECOGNITION_START: {
                periph_is31fl3216_set_light_on_num(h, 1, BLUE_LED_MAX_NUM);
                periph_is31fl3216_set_interval(h, 150);
                periph_is31fl3216_set_shift_mode(h, PERIPH_IS31_SHIFT_MODE_SINGLE);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_SHIFT);
                break;
            }
        case DISPLAY_PATTERN_RECOGNITION_STOP: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_OFF);
                break;
            }
        case DISPLAY_PATTERN_WAKEUP_ON: {
#if FLASH_SHIFT_ON_BOOT_SEL
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_duty_step(h, 20);
                periph_is31fl3216_set_interval(h, 10);
                periph_is31fl3216_set_act_time(h, 0);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_FLASH);
#else
                periph_is31fl3216_set_light_on_num(h, 2, BLUE_LED_MAX_NUM);
                periph_is31fl3216_set_interval(h, 70);
                periph_is31fl3216_set_shift_mode(h, PERIPH_IS31_SHIFT_MODE_SINGLE);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_SHIFT);
#endif
                break;
            }
            break;
        case DISPLAY_PATTERN_WAKEUP_FINISHED: {
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = DISPLAY_IDLE_DUTY_VAL;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                // Turn on one of 4 leds using duty
                for(int i = 0;i<BLUE_LED_MAX_NUM;i++){
                    if(value==0){
                        if( !(i%3) )    { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else if (value==1){
                        if( !((i+1)%3) ) { periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    } else {
                        if( !((i+2)%3) ){ periph_is31fl3216_set_duty(h, i, duty); }
                        else            { periph_is31fl3216_set_duty(h, i, 0); }
                    }
                }
                // Also turn on green LED
                periph_is31fl3216_set_duty(h, GREEN_LED_INDEX, duty); 
                // Turn on the display
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_ON);
                break;
            }
        case DISPLAY_PATTERN_MUSIC_ON: {
                // Enable all to turn on
                const int bit_mask_max = ((1 << MAX_LED_INDEX)-1);
                const int duty = DISPLAY_ACTIVE_DUTY_VAL;
                periph_is31fl3216_set_blink_pattern(h, bit_mask_max);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_BY_AUDIO);
                // Turn on one of 4 leds using duty
                for(int i = 0;i<BLUE_LED_MAX_NUM;i++){
                    periph_is31fl3216_set_duty(h, i, duty);
                }
                // Also turn on green LED
                periph_is31fl3216_set_duty(h, GREEN_LED_INDEX, duty); 
                break;
            }
        case DISPLAY_PATTERN_MUSIC_FINISHED: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_BY_AUDIO);
                break;
            }

        case DISPLAY_PATTERN_VOLUME:
            break;

        case DISPLAY_PATTERN_TURN_ON: {
                // Blank the display
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_OFF);
                break;
            }
            break;
        case DISPLAY_PATTERN_TURN_OFF: {
                int bits_mask = ((1 << BLUE_LED_MAX_NUM) - 1);
                periph_is31fl3216_set_blink_pattern(h, bits_mask);
                periph_is31fl3216_set_state(h, IS31FL3216_STATE_OFF);
                break;
            }
            break;
        default:
            ESP_LOGW(TAG, "The mode is invalid");
            break;
    }

    return ret;
}

display_service_handle_t my_audio_board_init(void)
{
    esp_periph_handle_t led = led_bar_is31x_init();
    AUDIO_NULL_CHECK(TAG, led, return NULL);
    display_service_config_t display = {
        .based_cfg = {
            .task_stack = 0,
            .task_prio  = 0,
            .task_core  = 0,
            .task_func  = NULL,
            .service_start = NULL,
            .service_stop = NULL,
            .service_destroy = NULL,
            .service_ioctl = my_led_bar_is31x_pattern,
            .service_name = "DISPLAY_serv",
            .user_data = NULL,
        },
        .instance = led,
    };

    return display_service_create(&display);
}
