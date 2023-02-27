#ifndef _MY_WIFI_EVENT_H_
#define _MY_WIFI_EVENT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT	   BIT1
/* FreeRTOS event group to signal when we are connected*/
extern EventGroupHandle_t s_wifi_event_group;

void wifi_init_sta(void);

#endif