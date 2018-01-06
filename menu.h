#ifndef MENU_H_
#define MENU_H_

#include "bool.h"
#include "audio.h"
#include <avr/pgmspace.h>

#define TITLE_LINE 0
#define WORKING_LINE 1

#define EQ_RADIO_OPTIONS (sizeof(eq_controls) / sizeof(eq_controls[0]))
#define EQ_EXTERNAL_OPTIONS (EQ_RADIO_OPTIONS)
#define SETTINGS_OPTIONS (sizeof(settings) / sizeof(settings[0]))

typedef uint8_t REMOTE_CMD;
typedef void(*HANDLE)(REMOTE_CMD, bool);

typedef enum {
    B_NOTHING,
    B_VOLUME,
    B_EQUALIZER,
    B_SETTINGS
} brush_t;

typedef enum {
    EXTERNAL,
    RADIO,
    EQUALIZER,
    SETTINGS
} mode_t;

typedef enum {
    SLIDER,
    OPTIONS
} setting_type_t;

typedef struct {
    int16_t min;
    int16_t max;
    int16_t step;
    PGM_P units;
} slider_t;

typedef struct {
    uint8_t count;
    PGM_P const *name;
} options_t;

typedef struct {
    PGM_P name;
    int8_t *data;
    audio_type_t type;
} eq_t;

typedef struct {
    PGM_P name;
    uint16_t init_val;
    uint16_t *val;
    setting_type_t type;
    union {
        slider_t slider;
        options_t options;
    } data;
} setting_opt_t;

void menu_init(void);
void menu_home(void);
void menu_update(REMOTE_CMD cmd, bool repeat);
void show_title(const char *s);
void show_title_p(PGM_P progmem_s);
void show_text(const char *s);
void show_text_p(PGM_P progmem_s);
void show_error(mode_t type, uint16_t errno);

#endif