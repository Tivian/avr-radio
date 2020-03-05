#ifndef REMOTE_HPP
#define REMOTE_HPP

#include <stdint.h>

namespace Remote {
enum class btn_t : uint8_t {
    power = 0x45,               menu =  0x47,
    test =  0x44, plus =  0x40, back =  0x43,
    prev =  0x07, play =  0x15, next =  0x09,
    zero =  0x16, minus = 0x19, clear = 0x0D,
    one =   0x0C, two =   0x18, three = 0x5E,
    four =  0x08, five =  0x1C, six =   0x5A,
    seven = 0x42, eight = 0x52, nine =  0x4A
};

inline constexpr uint8_t not_digit = 0xFF;

constexpr uint8_t operator*(const btn_t& btn) {
    switch (btn) {
        case btn_t::zero:
            return 0;
        case btn_t::one:
            return 1;
        case btn_t::two:
            return 2;
        case btn_t::three:
            return 3;
        case btn_t::four:
            return 4;
        case btn_t::five:
            return 5;
        case btn_t::six:
            return 6;
        case btn_t::seven:
            return 7;
        case btn_t::eight:
            return 8;
        case btn_t::nine:
            return 9;
        default:
            return not_digit;
    }
}
}

#endif
