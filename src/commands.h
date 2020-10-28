#ifndef COMMANDS_H_
#define COMMANDS_H_
#include <avr/pgmspace.h>

const char command_on[] PROGMEM = "ON";
const char command_off[] PROGMEM = "OFF";
const char command_die[] PROGMEM = "-";

PGM_P const uart_command_table[] PROGMEM = 
{   
    command_on,
    command_off,
    command_die
};
#endif