#ifdef __cplusplus
extern "C" {
#endif
#ifndef _LED_H_
#define _LED_H_
#include <avr/io.h>
#include "entity.h"

void toggle_led_on(ENTITY *led);
void toggle_led_off(ENTITY *led);
void switch_led(ENTITY *led);
void LED_init(void);

#endif
#ifdef __cplusplus
}
#endif