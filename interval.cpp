#include "interval.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace Interval {
namespace {
    uint8_t prescaler = 0;
    uint8_t mask = 0;

    callback_t call[8];
}

void init(clock_speed_t speed) {
    TCCR0 |= static_cast<uint8_t>(speed);
    TIMSK |= _BV(TOIE0);

    for (uint8_t i = 0; i < 8; i++)
        call[i] = nullptr;
}

void set(speed_t speed, const callback_t& fx) {
    if (fx != nullptr)
        mask |= speed;

    call[speed] = fx;
}

const callback_t& get(speed_t speed) {
    return call[speed];
}
}

ISR (TIMER0_OVF_vect) {
    using namespace Interval;

    ++prescaler;
    uint8_t index = 0;
    uint8_t value = prescaler & -prescaler;
    while (value >>= 1) index++;
    if (call[index] != nullptr) {
        if (mask & index)
            mask &= ~index;
        else if (call[index]())
            call[index] = nullptr;
    }
}