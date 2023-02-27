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

esp_err_t init_msg_handler(esp_periph_handle_t * bt_periph, display_service_handle_t * led_periph);
esp_err_t msg_handler(audio_event_iface_msg_t msg);


#endif