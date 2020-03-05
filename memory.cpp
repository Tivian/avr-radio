#include "memory.hpp"
#include "audio.hpp"
#include "brightness.hpp"
#include "menu.hpp"
#include "radio.hpp"

namespace Memory {
Eprom<uint16_t> rom   { ROM_ADDRESS   };
Eeprom<uint8_t> ram_a { RAM_A_ADDRESS };
Eeprom<uint8_t> ram_b { RAM_B_ADDRESS };

char buffer[16];

void load() {
    uint32_t saved = 0x00;
    if (ram_a.read_block(0x00, &saved, sizeof(Audio::registers_t))) {
        Audio::config(std::move(Audio::registers_t { saved }));
        Menu::set(Audio::config().source == Audio::source_t::second);
    }
    Brightness::value = ram_a.read((const void*) 0xff);
}

void save() {
    uint32_t config = *Audio::config();;
    ram_a.update_block(0x00, &config, sizeof(Audio::registers_t));
    ram_a.update((const void*) 0xff, Brightness::value);
}
}