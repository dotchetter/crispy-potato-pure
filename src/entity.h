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
}ENTITY;

typedef enum
{
    IDLE,
    ACTIVE,
    INACTIVE
} state;

#endif