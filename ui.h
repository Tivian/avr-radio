#ifndef UI_H_
#define UI_H_

#include <avr/io.h>

#define DDR(x) (*(&(x) - 1))

#define OP_LED_PORT PORTB
#define OP_LED_PIN PB7

void ui_init(void);
void ui_led_toggle(void);

#endif