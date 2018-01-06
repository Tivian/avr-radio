#ifndef UI_H_
#define UI_H_

#include "bool.h"
#include <avr/io.h>

#define DDR(x) (*(&(x) - 1))

#define OP_LED_PORT PORTB
#define OP_LED_PIN PB7

void ui_init(bool switch_on);
void ui_led_toggle(void);

#endif