#include "helpers.h"
#include "timer.h"
#include "commands.h"

const uint8_t parse_command()
/*
* Read the incoming command over USART. Parses the command and
* returns the int value for map place of given command in the 
* lookup table for PROGMEM commands, defined in commands.h.
* returns 0 if no command matched.
*/
{
    char input[255];

    uart_getline(input, sizeof(input) / sizeof(input[0]));
    
    if (strcasecmp_P(input, command_on) == 0)
    {
        return 1;
    }
    else if(strcasecmp_P(input, command_off) == 0)
    {
        return 2;
    }

    return 0;
}

const uint8_t debounceKey(ENTITY *key)
/*
* The entity is debounced with it's own 
* debounce time member.
*/
{
    const uint32_t current_time = millis();

    while ((current_time + key->debounce_ms) > millis())
    {
        if (!keyClicked(key))
        {
            return 0;            
        }
    }
    return 1;
}

const uint8_t keyClicked(ENTITY *key)
/*
* Return 1 if entity was pressed, 0 if not.
* Update the last_updated_ms timestamp.
*/
{
    if(*key->port & (_BV(key->registry_bit)))
    {
        key->last_updated_ms = millis();
        return 1;
    }
    return 0;
}

const uint8_t identifyLongPress(ENTITY *key)
/*
* Serve as a way to identify if a key or
* other entity is held down. Using the 
* Entity's own integer for milliseconds 
* as a classifier to the condition.
*/
{
    const uint32_t current_time = millis();

    while ((current_time + key->long_press_trigger_ms) > millis())
    {
        if (!keyClicked(key))
        {
            return 0;            
        }
    }
    return 1;
}

void analogWrite(ENTITY *entity, uint8_t val)
/*
* Writes analog value over PWM to compatible port.
*/
{
    *entity->port = val;
}


void digitalWrite(ENTITY *entity, uint8_t val)
/*
* Writes full byte of oxff if value is positive, 
* else 0x00 to compatible port.
*/
{
    if (val > 0)
    {
        *entity->port = 0xFF;
    }
    else
    {
        *entity->port = 0x00;
    }
}