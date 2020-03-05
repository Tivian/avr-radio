#include "nec.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace NEC {
namespace {
    volatile bool _ready;
    data_t _data;

    inline void clear_timer() {
        TCNT1 = AGC_MAX + 1;
    }

    event_t get_event(uint16_t delay) {
        if (delay >= AGC_MIN && delay <= AGC_MAX) {
            return event_t::agc_space;
        } else if (delay >= CMD_SPACE_MIN && delay <= CMD_SPACE_MAX) {
            return event_t::cmd_space;
        } else if (delay >= REPEAT_SPACE_MIN && delay <= REPEAT_SPACE_MAX) {
            return event_t::repeat;
        } else if (delay >= LOGIC_ONE_MIN && delay <= LOGIC_ONE_MAX) {
            return event_t::logic_one;
        } else if (delay >= LOGIC_ZERO_MIN && delay <= LOGIC_ZERO_MAX) {
            return event_t::logic_zero;
        } else {
            return event_t::idle;
        }
    }
}

void (* volatile indicate)(void) = nullptr;

void init() {
    reset();

    DDRD &= ~_BV(PD2);   // set PD2 as input
    PORTD |= _BV(PD2);   // turn on pull-up resistor
    MCUCR |= _BV(ISC00); // trigger on any logical change
    GICR |= _BV(INT0);   // enable INT0
    clear_timer();       // clear timer
    TCCR1B = _BV(CS11);  // turn on timer with 1/8 clock
    TIMSK |= _BV(TOIE1); // enable timer overflow interrupt
}

void reset() {
    _ready = false;
}

bool ready() {
    return _ready;
}

data_t& data() {
    return _data;
}
}

ISR (INT0_vect) {
    using namespace NEC;
    static data_t data;
    static state_t state;
    static uint8_t part, bit, *ptr;
    static bool is_data_bit;

    if (indicate != nullptr)
        indicate();

    if (_ready)
        return;

    uint16_t delay = TCNT1;
    TCNT1 = 0;
    event_t event = get_event(delay);

    if (state == state_t::idle && event == event_t::idle) {
        state = state_t::agc;
    } else if (state == state_t::agc && event == event_t::agc_space) {
        state = state_t::space;
    } else if (state == state_t::space && event == event_t::repeat) {
        state = state_t::end;
        data.repeat = true;
    } else if (state == state_t::space && event == event_t::cmd_space) {
        state = state_t::data;
        data.repeat = false;
        ptr = &data.addr;
        *ptr = 0;

        bit = 0;
        part = 0;
        is_data_bit = false;
    } else if (state == state_t::data && (event == event_t::logic_one || event == event_t::logic_zero)) {
        if (is_data_bit) {
            *ptr |= ((event == event_t::logic_one) << bit);

            if (++bit >= 8) {
                bit = 0;

                if (++part >= 4)
                    state = state_t::end;
                else
                    *++ptr = 0;
            }
        }

        is_data_bit = !is_data_bit;
    } else if (state == state_t::end && event == event_t::logic_zero) {
        state = state_t::idle;
        _ready = true;
        _data = data;
    } else {
        reset();
    }
}

ISR (TIMER1_OVF_vect) {
    NEC::clear_timer();
}