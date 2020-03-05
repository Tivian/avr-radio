#ifndef BRIGHTNESS_HPP
#define BRIGHTNESS_HPP

#include <stdint.h>

namespace Brightness {
extern volatile uint8_t value;
void init(uint8_t initial_value = 0);
}

#endif