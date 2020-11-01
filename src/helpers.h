#ifdef __cplusplus
extern "C" {
#endif
#ifndef HELPERS_H_
#define HELPERS_H_

#include "serial.h"
#include "entity.h"
#include "commands.h"

#define HIGH 0xFF
#define LOW 0x00

const uint8_t parse_command();
const uint8_t debounceKey(ENTITY *key);
const uint8_t keyClicked(ENTITY *key);
void analogWrite(ENTITY *entity, uint8_t val);
void digitalWrite(ENTITY *entity, uint8_t val);
const uint8_t identifyLongPress(ENTITY *key);

#endif
#ifdef __cplusplus
}
#endif