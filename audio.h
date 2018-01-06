#ifndef AUDIO_H_
#define AUDIO_H_

#include "bool.h"
#include <stdint.h>

#define PREAMP_TWI_TWBR 33
#define PREAMP_TWI_TWPS 0

#define PREAMP_REG_LEFT   0x00
#define PREAMP_REG_RIGHT  0x01
#define PREAMP_REG_BASS   0x02
#define PREAMP_REG_TREBLE 0x03
#define PREAMP_REG_CTRL   0x04

#define PREAMP_SLA 0x41

#define AUDIO_VOLUME_MAX 35
#define AUDIO_VOLUME_MIN 0
#define AUDIO_VOLUME_STEP 1

#define AUDIO_BALANCE_MAX 6
#define AUDIO_BALANCE_MIN -6
#define AUDIO_BALANCE_STEP 1

#define AUDIO_BASS_MAX 15
#define AUDIO_BASS_MIN -12
#define AUDIO_BASS_STEP 3

#define AUDIO_TREBLE_MAX 12
#define AUDIO_TREBLE_MIN -12
#define AUDIO_TREBLE_STEP 3

typedef uint8_t(*AUDIO_FX_P)(void);

typedef struct {
    uint8_t addr;
    AUDIO_FX_P fx;
} preamp_reg_t;

typedef struct {
    int8_t *data;
    int8_t min;
    int8_t max;
    int8_t step;
} audio_control_t;

typedef enum {
    VOLUME,
    BALANCE,
    BASS,
    TREBLE
} audio_type_t;

typedef enum {
    SOURCE_A,
    SOURCE_B
} source_t;

typedef enum {
    LEFT,
    RIGHT,
    BOTH
} channel_t;

typedef enum {
    FORCE_MONO,
    PSEUDO,
    LINEAR,
    SPATIAL
} stereo_t;

typedef struct {
    source_t source;
    bool mute;
    uint8_t volume;
    int8_t balance;
    int8_t bass;
    int8_t treble;
    channel_t channel_select;
    stereo_t stereo_mode;
} sound_t;

uint16_t audio_init(void);
uint16_t audio_update(void);
void audio_control_up(audio_type_t type);
void audio_control_down(audio_type_t type);
void audio_control_set(audio_type_t type, int8_t step);

#endif
