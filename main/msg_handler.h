#ifndef _MSG_HANDLER_H_
#define _MSG_HANDLER_H_

#include "esp_peripherals.h"
#include "esp_err.h"

#include "audio_element.h"
#include "audio_event_iface.h"
#include "display_service.h"
#include "periph_touch.h"
#include "periph_button.h"
#include "periph_adc_button.h"

/**
 * Message handler will take audio event messages 
 * and executed correlating handlers
 *
 * @param msg The message to be handlled
 *
 * @return Execution result (PASS/FAIL)
 *
 */
esp_err_t msg_handler(audio_event_iface_msg_t msg);

#endif