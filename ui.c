#include "ui.h"

void ui_init(void) {
    DDR(OP_LED_PORT) |= _BV(OP_LED_PIN);
    ui_led_toggle();
}

void ui_led_toggle(void) {
    OP_LED_PORT ^= _BV(OP_LED_PIN);
}