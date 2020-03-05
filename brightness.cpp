#include "brightness.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace Brightness {
volatile uint8_t value;// = 0;

void init(uint8_t initial_value) {
    DDRB |= _BV(PB3);    // set PB3 as output
    TIMSK |= _BV(TOIE2); // enable Timer2 overflow interrupt
    TCCR2 |= _BV(WGM20)  // PWM, phase correct
           | _BV(COM21) | _BV(COM20) // OC2 set on up-counting, clear on down-counting
           | _BV(CS21)  | _BV(CS20); // prescaler 32

    value = initial_value;
}
}

ISR (TIMER2_OVF_vect) {
    OCR2 = Brightness::value;
}