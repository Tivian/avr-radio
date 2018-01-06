#include "save.h"

// defaults
save_t save = {
    .mode = EXTERNAL,
    .audio = {
        .source = SOURCE_A,
        .mute = false,
        .volume = 10,
        .balance = 0,
        .bass = 0,
        .treble = 0,
        .channel_select = BOTH,
        .stereo_mode = LINEAR
    },
    .setup = {        
        .scroll_speed = 350,
        .timeout = 5
    }
};

void load_from_eeprom(void) {
    // Read save from EEPROM **TODO**
}

void save_to_eeprom(void) {
    // Write save to EEPROM **TODO**
}