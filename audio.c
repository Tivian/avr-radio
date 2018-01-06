#include "audio.h"
#include "i2c_master.h"
#include "save.h"
#include <stdlib.h>

static uint8_t _write_register(uint8_t byte);
static uint16_t _write_registers(void);
static uint8_t get_volume_left(void);
static uint8_t get_volume_right(void);
static uint8_t get_volume(channel_t ch);
static uint8_t get_bass(void);
static uint8_t get_treble(void);
static uint8_t get_switches(void);

extern save_t save;
static uint8_t registers[5];

const static preamp_reg_t reg[] = {
    { 0x00, &get_volume_left  },
    { 0x01, &get_volume_right },
    { 0x02, &get_bass         },
    { 0x03, &get_treble       },
    { 0x08, &get_switches     }
};

const static audio_control_t constrols[] = {
    { &save.audio.volume,  AUDIO_VOLUME_MIN,  AUDIO_VOLUME_MAX,  AUDIO_VOLUME_STEP  },
    { &save.audio.balance, AUDIO_BALANCE_MIN, AUDIO_BALANCE_MAX, AUDIO_BALANCE_STEP },
    { &save.audio.bass,    AUDIO_BASS_MIN,    AUDIO_BASS_MAX,    AUDIO_BASS_STEP    },
    { &save.audio.treble,  AUDIO_TREBLE_MIN,  AUDIO_TREBLE_MAX,  AUDIO_TREBLE_STEP  }
};

uint16_t audio_init(void) {
    i2c_init(PREAMP_TWI_TWBR, PREAMP_TWI_TWPS);
    audio_update();
}

uint16_t audio_update(void) {
    uint16_t ret = _write_registers();
    i2c_stop();
    return ret;
}

void audio_control_up(audio_type_t type) {
    audio_control_set(type, constrols[type].step);
}

void audio_control_down(audio_type_t type) {
    audio_control_set(type, -constrols[type].step);
}

void audio_control_set(audio_type_t type, int8_t step) {
    int8_t new_data = *(constrols[type].data) + step;
    if (new_data < constrols[type].min || new_data > constrols[type].max)
        return;

    *(constrols[type].data) = new_data;
}

static uint8_t _write_register(uint8_t byte) {
    uint8_t new_data = reg[byte].fx();
    if (new_data == registers[byte])
        return 0;

    return i2c_start(PREAMP_SLA, I2C_WRITE) ||
           i2c_write(reg[byte].addr) ||
           i2c_write(registers[byte] = new_data);
}

static uint16_t _write_registers(void) {
    for (uint8_t byte = 0; byte < 5; byte++) {
        if (_write_register(byte))
            return (byte << 8) | i2c_status();
    }

    return 0;
}

static uint8_t get_volume_left(void) {
    return get_volume(LEFT);
}

static uint8_t get_volume_right(void) {
    return get_volume(RIGHT);
}

static uint8_t get_volume(channel_t ch) {
    uint8_t val = 0xDC;
    uint16_t balanced_volume = ((((6 - abs(save.audio.balance)) * save.audio.volume) << 8) / 6) >> 8;

    if (save.audio.mute != true && save.audio.volume > 0) {
        val += ((ch == LEFT && save.audio.balance < 0) || (ch == RIGHT && save.audio.balance > 0))
            ? balanced_volume : save.audio.volume;
    }

    return val;
}

static uint8_t get_bass(void) {
    return ((save.audio.bass / 3) + 6) | 0xF0;
}

static uint8_t get_treble(void) {
    return ((save.audio.treble / 3) + 6) | 0xF0;
}

static uint8_t get_switches(void) {
    return (save.audio.source == SOURCE_B) | ((save.audio.channel_select + 1) << 1) |
           (save.audio.stereo_mode << 3)   | (save.audio.mute << 5) | 0xC0;
}
