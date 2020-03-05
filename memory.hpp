#ifndef MEMORY_HPP
#define MEMORY_HPP

#include "eeprom.hpp"
#include <avr/pgmspace.h>

namespace Memory {
static constexpr uint8_t ROM_ADDRESS = 0x50;
static constexpr uint8_t RAM_A_ADDRESS = 0x52;
static constexpr uint8_t RAM_B_ADDRESS = 0x53;

extern Eprom<uint16_t> rom;
extern Eeprom<uint8_t> ram_a;
extern Eeprom<uint8_t> ram_b;

extern char buffer[16];

void load();
void save();
}

#endif