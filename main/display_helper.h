#ifndef _DISPLAY_HELPER_H_
#define _DISPLAY_HELPER_H_

#include "esp_err.h"
#include "board.h"

#define FLASH_SHIFT_ON_BOOT_SEL 0


#define DISPLAY_IDLE_DUTY_VAL       20
#define DISPLAY_ACTIVE_DUTY_VAL     60

display_service_handle_t my_audio_board_init(void);

#endif