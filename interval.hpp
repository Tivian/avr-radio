#ifndef INTERVAL_HPP
#define INTERVAL_HPP

#include <stdint.h>

namespace Interval {
enum speed_t : uint8_t {
    slowest  = 7,
    slower   = 6,
    slow     = 5,
    slow_avg = 4,
    fast_avg = 3,
    fast     = 2,
    faster   = 1,
    fastest  = 0
};

enum class clock_speed_t : uint8_t {
    slowest = 0b101,
    slow    = 0b100,
    avg     = 0b011,
    fast    = 0b010,
    fastest = 0b001
};

using callback_t = bool (*)();

void init(clock_speed_t speed = clock_speed_t::avg);
void set(speed_t speed, const callback_t& fx);
const callback_t& get(speed_t speed);
}

#endif