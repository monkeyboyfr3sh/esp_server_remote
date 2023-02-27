#ifndef _DISPLAY_HELPER_H_
#define _DISPLAY_HELPER_H_

#include "esp_err.h"
#include "board.h"

#define FLASH_SHIFT_ON_BOOT_SEL 0


#define DISPLAY_IDLE_DUTY_VAL       20
#define DISPLAY_ACTIVE_DUTY_VAL     60

extern display_service_handle_t led_periph;
esp_err_t init_display_helper(void);

#endif