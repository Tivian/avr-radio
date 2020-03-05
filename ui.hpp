#ifndef UI_HPP
#define UI_HPP

#include <avr/io.h>
#include "utility.hpp"

#define POWER_LED_PORT PORTD
#define POWER_LED_PIN  PD7

#define ACTION_LED_PORT PORTD
#define ACTION_LED_PIN  PD6

#define MUTE_PORT PORTB
#define MUTE_PIN  PB0

namespace UI {
inline bool muted() { return MUTE_PORT & _BV(MUTE_PIN); }

template <bool v>
void power_led();

template <bool v>
void action_led();
inline void action_led_blink() { ACTION_LED_PORT ^= _BV(ACTION_LED_PIN); }

template <bool v>
void global_mute();

inline void init() {
    DDR(POWER_LED_PORT)  |= _BV(POWER_LED_PIN);
    DDR(ACTION_LED_PORT) |= _BV(ACTION_LED_PIN);
    DDR(MUTE_PORT)       |= _BV(MUTE_PIN);
}
}

#include "ui.tpp"

#endif