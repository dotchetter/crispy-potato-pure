#include "led.h"

void LED_init() {
	// ...
}

void toggle_led_on(ENTITY_LED *led)
{
    PORTB = 1 << led->registry_bit | PORTB;
    led->is_lit = 1;
}

void toggle_led_off(ENTITY_LED *led)
{   
    PORTB = ~(1 << led->registry_bit) & PORTB;
    led->is_lit = 0;
}

void switch_led(ENTITY_LED *led)
/*
* Switches led either on or off depending on state.
*/
{
	switch (led->is_lit)
	{
		case 0: toggle_led_on(led); break;
		case 1: toggle_led_off(led); break;
	}
}