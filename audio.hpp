#ifndef AUDIO_HPP
#define AUDIO_HPP

#include <stdint.h>
#include <avr/pgmspace.h>
#include "utility.hpp"

namespace Audio {
enum class eq_t : uint8_t {
    bass, treble, balance
};

inline constexpr eq_t& operator++(eq_t& v) {
    switch (v) {
        case eq_t::bass:
            return v = eq_t::treble;
        case eq_t::treble:
            return v = eq_t::balance;
        case eq_t::balance:
        default:
            return v = eq_t::bass;
    }
}

inline constexpr uint8_t operator*(const eq_t& v) {
    switch (v) {
        case eq_t::treble:
            return 1;
        case eq_t::balance:
            return 2;
        case eq_t::bass:
        default:
            return 0;
    }
}

enum class source_t : uint8_t {
    first  = 0b0,
    second = 0b1
};

inline source_t operator!(const source_t& s) {
    return s == source_t::first ? source_t::second : source_t::first;
}

enum class stereo_t : uint8_t {
    mono    = 0b00,
    linear  = 0b01,
    pseudo  = 0b10,
    spatial = 0b11
};

enum class channel_t : uint8_t {
    left  = 0b01,
    right = 0b10,
    both  = 0b11
};

struct registers_t {
    uint8_t   volume  : 6;
    int8_t    balance : 4;
    uint8_t   bass    : 4;
    uint8_t   treble  : 4;
    bool      mute    : 1;
    stereo_t  stereo  : 2;
    channel_t channel : 2;
    source_t  source  : 1;

    registers_t()
     : volume (0b011011)
     , balance(0)
     , bass   (0b0110)
     , treble (0b0110)
     , mute   (0b0)
     , stereo (stereo_t::linear)
     , channel(channel_t::both)
     , source (source_t::first) {}

    registers_t(uint32_t val)
     : volume (val >> 18)
     , balance(val >> 14)
     , bass   (val >> 10)
     , treble (val >> 6)
     , mute   ((val >> 5) & 0x1)
     , stereo (static_cast<stereo_t> (val >> 3))
     , channel(static_cast<channel_t>(val >> 1))
     , source (static_cast<source_t> (val)) {}

    constexpr uint8_t operator()(uint8_t index) const {
        return index < 4 ? index : 8;
    }

    inline uint8_t balanced_volume(uint8_t channel) const {
        if ((balance > 0 && channel == 0) || (balance < 0 && channel == 1))
            return volume * (6 - std::abs(balance)) / 6;
        else
            return volume;
    }

    uint8_t operator[](uint8_t index) const {
        switch (index) {
            case 0:
            case 1:
                return 0xC0 | balanced_volume(index);
            case 2:
                return 0xF0 | bass;
            case 3:
                return 0xF0 | treble;
            case 4:
                return 0xC0 | mute << 5
                    | static_cast<uint8_t>(stereo)  << 3
                    | static_cast<uint8_t>(channel) << 1
                    | static_cast<uint8_t>(source);
            default:
                return 0x00;
        }
    }

    constexpr uint32_t operator*() const {
        return
              static_cast<uint32_t>(volume) << 18
            | balance << 14
            | bass    << 10
            | treble  << 6
            | mute    << 5
            | static_cast<uint8_t>(stereo)  << 3
            | static_cast<uint8_t>(channel) << 1
            | static_cast<uint8_t>(source);
    }
};

static constexpr uint8_t DEVICE_ADDRESS = 0x41;

void init();
void update();

void volume(bool delta);
void mute(bool value);
void mute_toggle();
int8_t equalize(eq_t type);
void equalize(eq_t type, bool delta);
void source(source_t src);

registers_t& config();
void config(const registers_t& reg);
void config(registers_t&& reg);
}

#endif