#ifndef SAVE_H_
#define SAVE_H_

#include "audio.h"
#include "menu.h"
#include "radio.h"
#include <stdint.h>

typedef struct {
    uint16_t scroll_speed;
    uint16_t timeout;
} settings_t;

typedef struct {
    mode_t mode;
    sound_t audio;
    radio_t radio;
    settings_t setup;
} save_t;

void load_from_eeprom(void);
void save_to_eeprom(void);

#endif