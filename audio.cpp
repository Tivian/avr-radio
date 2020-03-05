#include "audio.hpp"
#include "i2c_master.hpp"
#include "utility.hpp"
#include <stdlib.h>

namespace Audio {
namespace {
    registers_t registers;
}

void init() {
    update();
}

void update() {
    for (uint8_t i = 0; i < 5; i++) {
        I2C::start(DEVICE_ADDRESS, I2C::mode_t::write);
        I2C::write(registers(i));
        I2C::write(registers[i]);
    }

    I2C::stop();
}

uint8_t volume() {
    return registers.volume - 27;
}

void volume(bool delta) {
    if ((registers.volume == 27 && !delta) || (registers.volume == 63 && delta))
        return;

    registers.volume += delta ? 1 : -1;
}

void mute(bool value) {
    registers.mute = value;
}

void mute_toggle() {
    mute(!registers.mute);
}

int8_t equalize(eq_t type) {
    switch (type) {
        case Audio::eq_t::bass:
            return (registers.bass - 6) * 3;
        case Audio::eq_t::treble:
            return (registers.treble - 6) * 3;
        case Audio::eq_t::balance:
            return registers.balance;
    }

    return 0;
}

void equalize(eq_t type, bool delta) {
    int8_t val = delta ? 1 : -1;
    switch (type) {
        case eq_t::bass:
            if (!((registers.bass == 0b0010 && !delta) || (registers.bass == 0b1011 && delta)))
                registers.bass += val;
            return;

        case eq_t::treble:
            if (!((registers.treble == 0b0010 && !delta) || (registers.treble == 0b1010 && delta)))
                registers.treble += val;
            return;

        case eq_t::balance:
            if (!((registers.balance == -6 && !delta) || (registers.balance == 6 && delta)))
                registers.balance += val;
            return;
    }
}

void source(source_t src) {
    registers.source = src;
    update();
}

registers_t& config() {
    return registers;
}

void config(const registers_t& reg) {
    registers = reg;
}

void config(registers_t&& reg) {
    registers = std::move(reg);
}
}
