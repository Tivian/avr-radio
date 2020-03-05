#include "sleep.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace Sleep {
namespace {
    volatile bool awaken = false;
}

volatile mode_t mode = mode_t::idle;
void (* volatile on_asleep)(void) = nullptr;
void (* volatile on_wakeup)(void) = nullptr;
bool (* volatile keep_sleeping)(void) = nullptr;

void init() {
    DDRD &= ~_BV(PD3);   // INT1 as input
    PORTD |= _BV(PD3);   // turn on pull-up resistor
    GICR |= _BV(INT1);   // enable INT1
    MCUCR |= _BV(ISC11); // trigger on level change
}

void activate() {
    awaken = false;
    if (on_asleep != nullptr)
        on_asleep();

    set_sleep_mode(static_cast<uint8_t>(mode));
    cli();

    do {
        sleep_enable();
        sei();
        sleep_cpu();
    } while (!awaken && (keep_sleeping == nullptr || (keep_sleeping != nullptr && keep_sleeping())));

    sleep_disable();
    sei();

    if (on_wakeup != nullptr)
        on_wakeup();
}

inline bool is_sleeping() {
    return bit_is_set(MCUCR, SE);
}

bool awaken_by_button() {
    return awaken;
}
}

ISR (INT1_vect) {
    using namespace Sleep;
    if (!is_sleeping())
        activate();
    else
        awaken = true;
}
