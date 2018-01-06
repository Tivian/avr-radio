#include "timeout.h"
#include <avr/interrupt.h>

static struct {
    uint8_t counter;
    uint8_t reminder;
    CALLBACK_FX callback;
} global;

static void _start_timer(void) {
    TIMSK |= _BV(OCIE2);
    TCCR2 |= _BV(WGM21) | _BV(CS22) | _BV(CS21) | _BV(CS20);
}

static void _stop_timer(void) {
    TIMSK &= ~_BV(OCIE2);
    TCCR2 &= ~(_BV(CS22) | _BV(CS21) | _BV(CS20));
}

bool is_timeout_set(void) {
    return global.callback != 0x00;
}

void set_timeout(uint8_t time, CALLBACK_FX fx) {
    uint16_t t = (7813 * (uint16_t) time - 1);
    global.counter = t >> 8;
    global.reminder = t & 0xFF;
    TCNT2 = 0x00;
    OCR2 = 0xFF;
    _start_timer();
    global.callback = fx;
}

void clear_timeout(void) {
    _stop_timer();
    global.callback = 0x00;
}

ISR(TIMER2_COMP_vect) {
    if (global.counter > 0) {
        global.counter--;

        if (global.counter == 0)
            OCR2 = global.reminder;
    } else {
        _stop_timer();
        global.callback();
        global.callback = 0x00;
    }
}
