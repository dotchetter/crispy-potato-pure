#ifndef _ENTITY_H_
#define _ENTITY_H_
#include <avr/io.h>

typedef struct {
    uint8_t registry_bit;
    uint32_t last_updated_ms;
    uint32_t debounce_ms;
    uint32_t long_press_trigger_ms;
    uint8_t is_active;
    volatile uint8_t *port;
    volatile uint8_t *data_direction_register;
}ENTITY;

typedef enum
{
    IDLE,
    PULSE_LED,
    POTENTIOMETER_LED,
    FLASH_LED,
    OFF
} state;

#endif