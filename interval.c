#include "interval.h"
#include <avr/interrupt.h>

static struct {
    uint8_t counter;
    uint8_t interval;
    CALLBACK_FX callback;
} global;

static void _start_timer(void) {
    TIMSK |= _BV(TOIE0);
    TCCR0 |= _BV(CS02);
}

static void _stop_timer(void) {
    TIMSK &= ~_BV(TOIE0);
    TCCR0 &= ~_BV(CS02);
}

bool is_interval_set(void) {
    return global.callback != 0x00;
}

void set_interval(uint16_t time, CALLBACK_FX fx) {
    global.interval = (31 * time - 1) >> 8;
    global.counter = global.interval;
    TCNT0 = 0x00;
    _start_timer();
    global.callback = fx;
}

void clear_interval(void) {
    _stop_timer();
    global.callback = 0x00;
}

ISR(TIMER0_OVF_vect) {
    if (--global.counter > 0) {
        global.counter = global.interval;
        global.callback();
    }
}

