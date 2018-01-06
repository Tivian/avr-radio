#include "menu.h"
#include "audio.h"
#include "lcd.h"
#include "radio.h"
#include "remote.h"
#include "save.h"
#include "timeout.h"
#include <stdlib.h>

const char external_str[] PROGMEM   = "    EXTERNAL";
const char radio_str[] PROGMEM      = "    RADIO FM";
const char settings_str[] PROGMEM   = "    SETTINGS";
const char bass_str[] PROGMEM       = "  BASS  ";
const char treble_str[] PROGMEM     = " TREBLE ";
const char balance_str[] PROGMEM    = "BALANCE ";
const char center_str[] PROGMEM     = "CENTER";
const char zero_eq_str[] PROGMEM    = "   0";
const char decibel_str[] PROGMEM    = "dB";
const char volume_str[] PROGMEM     = "   VOLUME ";
const char mute_str[] PROGMEM       = "     MUTE ON";
const char max_str[] PROGMEM        = "MAX";
const char on_str[] PROGMEM         = "ON";
const char off_str[] PROGMEM        = "OFF";
const char selector_str[] PROGMEM   = "SOURCE SELECT";
const char left_str[] PROGMEM       = "LEFT";
const char right_str[] PROGMEM      = "RIGHT";
const char both_str[] PROGMEM       = "BOTH";
const char stereo_m_str[] PROGMEM   = "STEREO MODE";
const char stereo_spat[] PROGMEM    = "SPATIAL";
const char stereo_lin[] PROGMEM     = "LINEAR";
const char stereo_pseu[] PROGMEM    = "PSEUDO";
const char stereo_mono[] PROGMEM    = "FORCE MONO";
const char scroll_str[] PROGMEM     = "SCROLL SPEED";
const char timeout_str[] PROGMEM    = "TIMEOUT";
const char micsec_str[] PROGMEM     = "ms";
const char seconds_str[] PROGMEM    = "s";
const char boost_str[] PROGMEM      = "BOOST";
const char radio_err_str[] PROGMEM  = "RADIO";
const char preamp_err_str[] PROGMEM = "PREAMP";
const char error_str[] PROGMEM      = "ERROR";
const char success_str[] PROGMEM    = "SUCCESS";
const char blank_str[] PROGMEM      = "";

PGM_P const source_opt[] PROGMEM = { left_str, right_str, both_str };
PGM_P const stereo_opt[] PROGMEM = { stereo_mono, stereo_pseu, stereo_lin, stereo_spat  };
PGM_P const off_on_opt[] PROGMEM = { off_str, on_str };

const uint8_t vert_arrows[] PROGMEM = { 0x04, 0x0E, 0x1F, 0x00, 0x00, 0x1F, 0x0E, 0x04 };
const uint8_t up_arrow[] PROGMEM    = { 0x04, 0x0E, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00 };
const uint8_t down_arrow[] PROGMEM  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x0E, 0x04 };
const uint8_t left_arrow[] PROGMEM  = { 0x02, 0x06, 0x0E, 0x1E, 0x1E, 0x0E, 0x06, 0x02 };
const uint8_t right_arrow[] PROGMEM = { 0x08, 0x0C, 0x0E, 0x0F, 0x0F, 0x0E, 0x0C, 0x08 };

static int16_t temp;
static uint8_t buf_index = 0;
static char buffer[16];

static uint8_t eq_pos = 0;
static uint8_t sett_pos = 0;
static HANDLE handle;
static mode_t caller = EXTERNAL;

extern save_t save;

static setting_opt_t opt;
const setting_opt_t settings[] PROGMEM = {
    { selector_str, BOTH,    &save.audio.channel_select, BOTH,  OPTIONS, { .options = { 3, source_opt } } },
    { stereo_m_str, SPATIAL, &save.audio.stereo_mode,    BOTH,  OPTIONS, { .options = { 4, stereo_opt } } },
    { timeout_str,  5,       &save.setup.timeout,        BOTH,  SLIDER,  { .slider  = {   1,    8,   1, seconds_str } } },
    { scroll_str,   350,     &save.setup.scroll_speed,   BOTH,  SLIDER,  { .slider  = { 250, 1000,  50, micsec_str  } } },
    { boost_str,    OFF, (uint16_t*) &save.radio.boost,  RADIO, OPTIONS, { .options = { 2, off_on_opt } } }
};

static eq_t eq;
const eq_t eq_controls[] PROGMEM = {
    { bass_str,    &save.audio.bass,    BASS    },
    { treble_str,  &save.audio.treble,  TREBLE  },
    { balance_str, &save.audio.balance, BALANCE }
};

static PGM_P get_mode_str(void);
static HANDLE get_handle(void);
static HANDLE get_default_handle(void);
static mode_t get_default_mode(void);
static void repaint(brush_t brush);
static bool common_mode(REMOTE_CMD cmd, bool repeat);
static void external_mode(REMOTE_CMD cmd, bool repeat);
static void radio_mode(REMOTE_CMD cmd, bool repeat);
static void equalizer_mode(REMOTE_CMD cmd, bool repeat);
static void settings_mode(REMOTE_CMD cmd, bool repeat);
static void selector_mode(REMOTE_CMD cmd, bool repeat);
static void change_mode(mode_t mode, bool clear);
static inline void change_source(void);

void menu_init(void) {
    save.mode = (save.mode == EXTERNAL) ? EXTERNAL : RADIO;
    handle = get_handle();
}

void menu_home(void) {
    lcd_clr();
    show_title_p(get_mode_str());
}

void menu_update(REMOTE_CMD cmd, bool repeat) {
    if (!common_mode(cmd, repeat))
        handle(cmd, repeat);
}

void show_title(const char *s) {
    lcd_clear_line(TITLE_LINE);
    lcd_puts(s);
}

void show_title_p(PGM_P progmem_s) {
    lcd_clear_line(TITLE_LINE);
    lcd_puts_p(progmem_s);
}

void show_text(const char *s) {
    lcd_clear_line(WORKING_LINE);
    lcd_puts(s);
}

void show_text_p(PGM_P progmem_s) {
    lcd_clear_line(WORKING_LINE);
    lcd_puts_p(progmem_s);
}

void show_error(mode_t type, uint16_t errno) {
    show_title_p(type == EXTERNAL ? preamp_err_str : type == RADIO ? radio_err_str : blank_str);
    buffer[0] = '0' + (errno >> 8);
    buffer[1] = ' ';
    utoa((errno & 0xff), &buffer[2], 16);
    show_text(buffer);
    for(;;){}
}

static PGM_P get_mode_str(void) {
    switch (save.mode) {
        case EXTERNAL:
            return external_str;

        case RADIO:
            return radio_str;

        case SETTINGS:
            return settings_str;
    }

    return blank_str;
}

static HANDLE get_handle(void) {
    switch (save.mode) {
        case EXTERNAL:
            return external_mode;

        case RADIO:
            return radio_mode;

        case EQUALIZER:
            return equalizer_mode;

        case SETTINGS:
            return settings_mode;

        case SELECTOR:
            return selector_mode;
    }

    return 0;
}

static HANDLE get_default_handle(void) {
    return save.audio.source == SOURCE_A ? external_mode : radio_mode;
}

static mode_t get_default_mode(void) {
    return save.audio.source == SOURCE_A ? EXTERNAL : RADIO;
}

static void repaint(brush_t brush) {
    switch (brush) {
        case B_VOLUME:
            if (save.audio.mute) {
                show_text_p(mute_str);
            } else {
                show_text_p(volume_str);
                if (save.audio.volume == AUDIO_VOLUME_MAX) {
                    lcd_puts_p(max_str);
                } else {
                    buffer[0] = '0' + (save.audio.volume / 10);
                    buffer[1] = '0' + (save.audio.volume % 10);
                    buffer[2] = '\0';
                    lcd_puts(buffer);
                }
            }

            set_timeout(save.setup.timeout, ({ void fx(void) {
                lcd_clear_line(WORKING_LINE);
            } fx; }));
            break;

        case B_EQUALIZER:
            show_text_p(eq.name);
            lcd_put_def_p(left_arrow, 8, 1);
            if (*eq.data == 0) {
                lcd_puts_p(eq.type == BALANCE ? center_str : zero_eq_str);
            } else {
                uint8_t res = abs(*eq.data);
                buffer[0] = ' ';
                if (eq.type != BALANCE) {
                    buffer[1] = (*eq.data) < 0 ? '-' : '+';
                    buffer[2] = res < 10 ? ' ' : '0' + res / 10;
                    buffer[3] = '0' + res % 10;
                    buffer[4] = '\0';
                } else {
                    buffer[1] = '+';
                    buffer[2] = (*eq.data) < 0 ? 'L' : 'R';
                    buffer[3] = '0';
                    buffer[4] = '0' + res % 10;
                    buffer[5] = ' ';
                    buffer[6] = '\0';
                }
                lcd_puts(buffer);
            }

            if (eq.type != BALANCE)
                lcd_puts_p(decibel_str);

            lcd_put_def_p(right_arrow, LCD_DISP_LENGTH - 1, 1);
            break;

        case B_SETTINGS:
            lcd_clear_line(WORKING_LINE);
            lcd_put_def_p(vert_arrows, 0, 1);

            lcd_goto(2, 1);
            lcd_puts_p(opt.name);
            break;

        case B_SELECTOR:
            lcd_clr();
            lcd_puts_center_p(opt.name, 0);
            if (opt.type == OPTIONS) {
                lcd_put_def_p(
                    (*opt.val == 0) ? up_arrow :
                    (*opt.val == opt.data.options.count - 1) ? down_arrow : vert_arrows, 0, 1
                );
                lcd_goto(2, 1);
                lcd_puts_p((PGM_P) pgm_read_word(&(opt.data.options.name[*opt.val])));
            } else {
                utoa(*opt.val, buffer, 10);
                lcd_puts_center(buffer, 1);

                if (*opt.val > opt.data.slider.min)
                    lcd_put_def_p(left_arrow, 0, 1);

                if (*opt.val < opt.data.slider.max)
                    lcd_put_def_p(right_arrow, LCD_DISP_LENGTH - 1, 1);
            }
            break;

        case B_RADIO:

            break;
    }
}

static bool common_mode(REMOTE_CMD cmd, bool repeat) {
    if (save.mode != EXTERNAL && save.mode != RADIO)
        return false;

    switch (cmd) {
        case REMOTE_PLAY:
            if (!repeat)
                save.audio.mute = !save.audio.mute;
            break;

        case REMOTE_PLUS:
            save.audio.mute = false;
            audio_control_up(VOLUME);
            break;

        case REMOTE_MINUS:
            save.audio.mute = false;
            audio_control_down(VOLUME);
            break;

        case REMOTE_TEST:
            eq_pos = 0;
            memcpy_P(&eq, &eq_controls[eq_pos], sizeof(eq_t));
            change_mode(EQUALIZER, false);
            break;

        case REMOTE_CLEAR:
            if (!repeat)
                change_source();
            break;

        case REMOTE_MENU:
            if (!repeat) {
                sett_pos = 0;
                memcpy_P(&opt, &settings[sett_pos], sizeof(setting_opt_t));
                change_mode(SETTINGS, true);
            }
            break;

        default:
            return false;
    }

    if ((cmd == REMOTE_PLAY && !repeat) || cmd == REMOTE_PLUS || cmd == REMOTE_MINUS) {
        audio_update();
        repaint(B_VOLUME);
    }

    return true;
}

static void external_mode(REMOTE_CMD cmd, bool repeat) {
    return;
}

static void radio_mode(REMOTE_CMD cmd, bool repeat) {
    switch (cmd) {
        case REMOTE_BACKWARD:
            radio_seek_down(true);
            break;

        case REMOTE_FORWARD:
            radio_seek_up(true);
            break;
    }

    repaint(B_RADIO);
}

static void equalizer_mode(REMOTE_CMD cmd, bool repeat) {
    switch (cmd) {
        case REMOTE_BACKWARD:
            audio_control_down(eq.type);
            break;

        case REMOTE_FORWARD:
            audio_control_up(eq.type);
            break;

        case REMOTE_TEST:
            if (!repeat) {
                eq_pos++;
                if (eq_pos >= EQ_OPTIONS)
                    eq_pos = 0;

                memcpy_P(&eq, &eq_controls[eq_pos], sizeof(eq_t));
            }
            break;

        case REMOTE_RETURN:
            change_mode(caller, true);
            return;
    }

    if (cmd == REMOTE_BACKWARD || cmd == REMOTE_FORWARD)
        audio_update();

    repaint(B_EQUALIZER);
    set_timeout(save.setup.timeout, ({ void fx(void) {
        change_mode(caller, true);
    } fx; }));
}

static void settings_mode(REMOTE_CMD cmd, bool repeat) {
    switch (cmd) {
        case REMOTE_PLUS:
        case REMOTE_MINUS:
            if (!repeat) {
                do {
                    if (cmd == REMOTE_PLUS) {
                        sett_pos++;
                        if (sett_pos == SETTINGS_OPTIONS)
                            sett_pos = 0;
                    } else {
                        if (sett_pos == 0)
                            sett_pos = SETTINGS_OPTIONS;
                        sett_pos--;
                    }
                    memcpy_P(&opt, &settings[sett_pos], sizeof(setting_opt_t));
                } while (
                    opt.mode != BOTH &&
                    (save.mode == EXTERNAL && opt.mode != S_EXTERNAL) &&
                    (save.mode == RADIO && opt.mode != S_RADIO)
                );
            }
            break;

        case REMOTE_MENU:
        case REMOTE_RETURN:
        case REMOTE_BACKWARD:
            if (!repeat) {
                change_mode(caller, true);
                return;
            }
            break;

        case REMOTE_PLAY:
        case REMOTE_FORWARD:
            if (!repeat) {
                temp = *opt.val;
                change_mode(SELECTOR, true);
                return;
            }
            break;
    }

    repaint(B_SETTINGS);
}

static void selector_mode(REMOTE_CMD cmd, bool repeat) {
    int16_t min, max, step;

    switch (cmd) {
        case REMOTE_PLUS:
        case REMOTE_MINUS:
        case REMOTE_FORWARD:
        case REMOTE_BACKWARD:
            if (opt.type == SLIDER) {
                min = opt.data.slider.min;
                max = opt.data.slider.max;
                step = opt.data.slider.step;
            } else { // OPTIONS
                min = 0;
                max = opt.data.options.count - 1;
                step = 1;
            }

            if (cmd == REMOTE_MINUS || cmd == REMOTE_BACKWARD)
                step = -step;

            *opt.val += step;
            if (*opt.val < min || *opt.val > max)
                *opt.val -= step;
            break;

        case REMOTE_CLEAR:
            if (!repeat)
                *opt.val = opt.initial;
            break;

        case REMOTE_MENU:
        case REMOTE_RETURN:
            *opt.val = temp;
        case REMOTE_PLAY:        
            if (!repeat) {
                save.mode = get_default_mode();
                change_mode(caller, true);
                return;
            }
            break;
    }

    repaint(B_SELECTOR);
}

static void change_mode(mode_t mode, bool clear) {
    if (is_timeout_set())
        clear_timeout();

    caller = save.mode;
    save.mode = mode;
    handle = get_handle();

    if (clear) {
        menu_home();
        handle(REMOTE_BLANK, false);
    }
}

static inline void change_source(void) {
    change_mode(save.mode == EXTERNAL ? RADIO : EXTERNAL, true);
}

/*
Radio screen
0123456789012345
 FM P01 0123456

Volume control
Step: 2dB (35 values)
+ 6dB 111111 (63 / 0x3F) MAX
............
+ 0dB 111100 (60 / 0x3C)
............
-64dB 011100 (28 / 0x1C) MIN
Formula: floor(0.35 * x) + 28
Default: 10

Balance control
Step: 1
VALUE   R    L
+R06   100%   0%
+R05   100%  16%
+R04   100%  33%
+R03   100%  50%
+R02   100%  67%
+R01   100%  83%
CENTER 100% 100%
+L01    83% 100%
+L02    67% 100%
+L03    50% 100%
+L04    33% 100%
+L05    16% 100%
+L06     0% 100%

volume * 200%

Bass control
Step: 3dB (9 values)
+15dB 1011 (11 / 0x0B) MAX
..........
+ 0dB 0110 ( 6 / 0x06) DEFAULT
..........
-12dB 0010 ( 2 / 0x02) MIN
Formula: floor(x / 3) + 6
Default: 0
Unit dB

Treble control
Step: 3dB (8 values)
+12dB 1010 (10 / 0x0A) MAX
..........
+ 0dB 0110 ( 6 / 0x06) DEFAULT
..........
-12dB 0010 ( 2 / 0x02) MIN
Formula: floor(x / 3) + 6
Default: 0
Unit dB

int val = MIN_VAL, n = MIN;
while (n != bass/treble) {
    n += STEP;
    val++;
}

All modes:
POWER -> power on/off
MENU  -> settings
TEST  -> bass/treble/balance
 C    -> change source
PLAY  -> mute on/off
 +    -> volume up
 -    -> volume down

Bass/treble/balance control:
|<<  -> down
>>|  -> up

In settings:
RETURN -> exit menu
 C     -> set default
 +     -> up
 -     -> down
|<<    -> left
>>|    -> right

Radio mode:
|<< -> previous channel
>>| -> next channel
0-9 -> channel preset

"                "

External screen:
"    EXTERNAL    "

Radio screen:
"     RADIO      "
???

Settings screen:
"    SETTINGS    "

Volume control:
"     MUTE ON    "
"   VOLUME MAX   "
"   VOLUME 00    "
"   VOLUME 99    "

Bass/treble/balance control:
        "<   0dB>"
"  BASS  <  +0dB>"
"  BASS  < -06dB>"
"  BASS  < +15dB>"
" TREBLE <  +0dB>"
"BALANCE < +L06 >" -3
"BALANCE < +L04 >" -2
"BALANCE < +L02 >" -1
"BALANCE <CENTER>"  0
"BALANCE < +R02 >"  1
"BALANCE < +R04 >"  2
"BALANCE < +R06 >"  3
" BOOST  <ON/OFF>" (radio only)
" BOOST  <  /OFF>" 0
" BOOST  <ON/   >" 1

Settings:
- channel select
  o stereo
  o left
  o right
- stereo
  o spatial stereo
  o linear stereo
  o pseudo stereo
  o forced mono
- scroll speed
- timeout

Save to EEPROM:
- mode
- volume
- balance
- bass
- treble
- mute
- settings

setting struct
- name
- pointer to value
- type
  o slider
    - min
    - max
    - step
  o list
    - array of options

void main_screen(void) {
    lcd_clr();

    show_title_p(mode_handle == external_screen ? external_str :
                (mode_handle == radio_screen    ? radio_str    :
                (mode_handle == settings_screen ? settings_str : blank_str)));
}

bool common_screen(uint8_t cmd, bool repeat) {
    if (mode_handle != external_screen &&
      mode_handle != radio_screen) {
        return false;
    }

    switch (cmd) {
        case REMOTE_PLAY:
            if (!repeat) {
                save.audio.mute = !save.audio.mute;
            }
        case REMOTE_PLUS:
        case REMOTE_MINUS:
            volume_control(cmd);
            break;

        case REMOTE_TEST:
            if (mode_handle != eq_screen) {
                eq_pos = 0;
                change_screen(eq_screen, false);
            }
            break;

        case REMOTE_CLEAR:
            if (!repeat) {
                change_source();
            }
            break;

        case REMOTE_MENU:
            if (!repeat) {
                sett_pos = 0;
                change_screen(settings_screen, true);
            }
            break;

        default:
            return false;
    }

    return true;
}

void external_screen(uint8_t cmd, bool repeat) {
    switch (cmd) {

    }
}

void radio_screen(uint8_t cmd, bool repeat) {
    switch (cmd) {
        case REMOTE_BACKWARD:
        case REMOTE_FORWARD:
            radio_seek(cmd);
            break;
    }
}

void radio_seek(uint8_t cmd) {
    if (cmd == REMOTE_BACKWARD)
        radio_seek_down(true);
    else if (cmd == REMOTE_FORWARD)
        radio_seek_up(true);
}

void show_radio_info(void) {
    RADIO_INFO info;
    radio_get_info(&info);

    if (info.rds) {
        RADIO_RDS rds;
        if (radio_get_rds(&rds)) {
            char top[8], bottom[8];
            utoa(rds.group_type, top, 2);
            utoa(rds.group_version, bottom, 2);
            show_title(top);
            show_text(bottom);
        }
    }
}

void settings_screen(uint8_t cmd, bool repeat) {
    switch (cmd) {
        case REMOTE_PLUS:
            if (!repeat) {
                sett_pos++;
                if (sett_pos == SETTINGS_OPTIONS)
                    sett_pos = 0;
            }
            break;

        case REMOTE_MINUS:
            if (!repeat) {
                if (sett_pos == 0)
                    sett_pos = SETTINGS_OPTIONS;
                sett_pos--;
            }
            break;

        case REMOTE_MENU:
        case REMOTE_RETURN:
        case REMOTE_BACKWARD:
            if (!repeat) {
                change_screen(get_default_handle(), true);
                return;
            }
            break;

        case REMOTE_PLAY:
        case REMOTE_FORWARD:
            if (!repeat) {
                change_screen(setting_control, false);
                return;
            }
            break;
    }

    setting_opt_t opt;
    memcpy_P(&opt, &settings[sett_pos], sizeof(setting_opt_t));
    sett_val = *opt.val;
    menu_member(opt.name);
}

void setting_control(uint8_t cmd, bool repeat) {
    char buffer[16];
    setting_opt_t opt;
    memcpy_P(&opt, &settings[sett_pos], sizeof(setting_opt_t));

    switch (cmd) {
        case REMOTE_MINUS:
        case REMOTE_PLUS:
            if (opt.type == SLIDER)
                return;

            if (cmd == REMOTE_MINUS && *opt.val > 0) {
                (*opt.val)--;
            } else if (cmd == REMOTE_PLUS && *opt.val < opt.data.options.count - 1) {
                (*opt.val)++;
            }
            break;

        case REMOTE_BACKWARD:
        case REMOTE_FORWARD:
            if (opt.type == OPTIONS)
                return;

            if (cmd == REMOTE_BACKWARD && *opt.val > opt.data.slider.min) {
                *opt.val -= opt.data.slider.step;
            } else if (cmd == REMOTE_FORWARD && *opt.val < opt.data.slider.max) {
                *opt.val += opt.data.slider.step;
            }
            break;

        case REMOTE_CLEAR:
            if (!repeat) {
                *opt.val = opt.init_val;
            }
            break;

        case REMOTE_MENU:
            if (!repeat) {
                change_screen(get_default_handle(), true);
                return;
            }
            break;

        case REMOTE_RETURN:
            if (!repeat) {
                *opt.val = sett_val;
            }
        case REMOTE_PLAY:
            if (!repeat) {
                change_screen(caller, true);
                return;
            }
            break;
    }

    lcd_clr();
    lcd_puts_center_p(opt.name, 0);

    if (opt.type == OPTIONS) {
        lcd_put_def_p(
            (*opt.val == 0) ? up_arrow :
            (*opt.val == opt.data.options.count - 1) ? down_arrow : vert_arrows, 0, 1);
        lcd_goto(2, 1);
        lcd_puts_p((PGM_P) pgm_read_word(&(opt.data.options.name[*opt.val])));
    } else if (opt.type == SLIDER) {
        if (*opt.val > opt.data.slider.min)
            lcd_put_def_p(left_arrow, 0, 1);

        if (*opt.val < opt.data.slider.max)
            lcd_put_def_p(right_arrow, LCD_DISP_LENGTH - 1, 1);

        utoa(*opt.val, buffer, 10);
        strcat(buffer, " ");
        strcat_P(buffer, opt.data.slider.units);
        lcd_puts_center(buffer, 1);
    }
}

void change_screen(HANDLE screen, bool clear) {
    if (is_timeout_set()) {
        clear_timeout();
    }

    caller = mode_handle;
    mode_handle = screen;

    if (clear) {
        main_screen();
    }

    screen(REMOTE_BLANK, false);
}

void eq_screen(uint8_t cmd, bool repeat) {
    eq_t eq;
    memcpy_P(&eq, &eq_controls[eq_pos], sizeof(eq_t));

    switch (cmd) {
        case REMOTE_BACKWARD:
            if (*(eq.p) > eq.slider.min)
                *(eq.p) -= eq.slider.step;
            break;

        case REMOTE_FORWARD:
            if (*(eq.p) < eq.slider.max)
                *(eq.p) += eq.slider.step;
            break;

        case REMOTE_TEST:
            if (!repeat) {
                eq_pos++;
                if (eq_pos >= (caller == external_screen ?
                  EQ_EXTERNAL_OPTIONS : EQ_RADIO_OPTIONS)) {
                    eq_pos = 0;
                }
            }
            break;

        case REMOTE_RETURN:
            change_screen(caller, true);
            return;
    }

    eq_control();
    set_timeout(save.setup.timeout, ({ void fx(void) {
        change_screen(caller, true);
    } fx; }));
}

void eq_control(void) {
    eq_t eq;
    char buffer[4];
    memcpy_P(&eq, &eq_controls[eq_pos], sizeof(eq_t));

    clear_working_line();
    lcd_goto(0, 1);
    lcd_puts_p(eq.name);

    lcd_put_def_p(left_arrow, 8, 1);
    if (eq.name == bass_str ||
        eq.name == treble_str) {
        lcd_putc(' ');
        if (*(eq.p) == 0)
            lcd_putc(' ');

        lcd_putc(*(eq.p) < 0 ? '-' : '+');
        if (abs(*(eq.p)) < 10)
            lcd_putc('0');

        if (*(eq.p) != 0) {
            itoa(abs(*(eq.p)), buffer, 10);
            lcd_puts(buffer);
        }

        lcd_puts_p(decibel_str);
    } else if (eq.name == balance_str) {
        if (*(eq.p) == 0) {
            lcd_puts_p(center_str);
        } else {
            lcd_putc(' ');
            lcd_putc('+');
            lcd_putc(*(eq.p) < 0 ? 'L' : 'R');
            if (abs(*(eq.p)) < 10) {
                lcd_putc('0');
                lcd_putc('0' + abs(*(eq.p)));
            }
#if BALANCE_MAX >= 10 || BALANCE_MIN <= -10
            else {
                itoa(abs(*(eq.p)), buffer, 10);
                lcd_puts(buffer);
            }
#endif
        }
    } else if (eq.name == boost_str) {
        lcd_puts_p(!*(eq.p) == 0 ? off_str : on_str);
    }
    lcd_put_def_p(right_arrow, LCD_DISP_LENGTH - 1, 1);
}

void volume_control(uint8_t cmd) {
    if (cmd == REMOTE_PLUS && save.audio.volume != VOLUME_MAX) {
        save.audio.volume++;
    } else if (cmd == REMOTE_MINUS && save.audio.volume != VOLUME_MIN) {
        save.audio.volume--;
    }

    if (cmd != REMOTE_PLAY)
        save.audio.mute = false;

    clear_working_line();
    if (save.audio.mute) {
        lcd_puts_p(mute_str);
    } else {
        lcd_puts_p(volume_str);

        if (save.audio.volume == VOLUME_MAX) {
            lcd_puts_p(max_str);
        } else {
            if (save.audio.volume < 10)
                lcd_putc('0');

            if (save.audio.volume == 0) {
                lcd_putc('0');
            } else {
                char buffer[4];
                itoa(save.audio.volume, buffer, 10);
                lcd_puts(buffer);
            }
        }
    }

    set_timeout(save.setup.timeout, clear_working_line);
}

HANDLE get_default_handle(void) {
    return save.audio.source == EXTERNAL ? external_screen : radio_screen;
}

void change_source(void) {
    save.audio.source = save.audio.source == EXTERNAL ? RADIO : EXTERNAL;
    change_screen(mode_handle == external_screen ?
        radio_screen : external_screen, true);
}

void _show_error(uint16_t errno) {
    char buffer[8];
    buffer[0] = '0' + (errno >> 8);
    buffer[1] = ' ';
    utoa((errno & 0xff), &buffer[2], 16);
    show_text(buffer);
    for(;;){}
}

void show_error(const char *s, uint16_t errno) {
    show_title(s);
    _show_error(errno);
}

void show_error_p(PGM_P progmem_s, uint16_t errno) {
    show_title_p(progmem_s);
    _show_error(errno);
}

void show_title(const char *s) {
    lcd_home();
    lcd_puts(s);
}

void show_title_p(PGM_P progmem_s) {
    lcd_home();
    lcd_puts_p(progmem_s);
}

void show_text(const char *s) {
    lcd_goto(0, 1);
    lcd_clear_line(1);
    lcd_puts(s);
}

void show_text_p(PGM_P progmem_s) {
    lcd_goto(0, 1);
    lcd_clear_line(1);
    lcd_puts_p(progmem_s);
}

void clear_working_line(void) {
    lcd_clear_line(1);
}

void menu_member(PGM_P progmem_s) {
    clear_working_line();
    lcd_put_def_p(vert_arrows, 0, 1);

    lcd_goto(2, 1);
    lcd_puts_p(progmem_s);
}*/
