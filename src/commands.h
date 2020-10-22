#pragma once
#include <avr/io.h>
#include <avr/pgmspace.h>

const char command_on[] PROGMEM = "ON";
const char command_off[] PROGMEM = "OFF";

PGM_P const uart_command_table[] PROGMEM = 
{   
    command_on,
    command_off,
};
#endif
