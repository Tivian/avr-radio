#include "ui.h"

void ui_init(bool switch_on) {
    DDR(OP_LED_PORT) |= _BV(OP_LED_PIN);
    if (switch_on)
        ui_led_toggle();
}

void ui_led_toggle(void) {
    OP_LED_PORT ^= _BV(OP_LED_PIN);
}