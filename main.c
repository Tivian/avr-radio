/*
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

typedef struct {
    int8_t min;
    int8_t max;
    int8_t step;
} slider_t;

typedef struct {
    const char *name;
    int8_t *val;
    setting_type_t type;
    union {
        slider_t slider;
        PGM_P const options;
    } data;
} setting_opt_t;

1) pos < len         -> &str[pos]
2) len - pos - 3 < 0 -> &str[line - pos + len - 4]
3) line - pos + len - 4 == 0 -> pos = 0

*/
/*#include "main.h"
#include "menu.h"
#include "ui.h"
#include "lcd.h"
#include "nec.h"
#include "remote.h"
#include "interval.h"
#include "timeout.h"
#include "scroll.h"
#include "audio.h"
#include "radio.h"
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <string.h>

#include <util/delay.h>*/



//HANDLE mode_handle;
//HANDLE caller;
//uint8_t eq_pos = 0;
//uint8_t sett_pos = 0;
//uint16_t sett_val;

/*void main_screen(void);
bool common_screen(uint8_t cmd, bool repeat);
void external_screen(uint8_t cmd, bool repeat);
void radio_screen(uint8_t cmd, bool repeat);
void radio_seek(uint8_t cmd);
void show_radio_info(void);
void settings_screen(uint8_t cmd, bool repeat);
void setting_control(uint8_t cmd, bool repeat);
void change_screen(HANDLE screen, bool clear);
void eq_screen(uint8_t cmd, bool repeat);
void eq_control(void);
void volume_control(uint8_t cmd);
HANDLE get_default_handle(void);
void change_source(void);
void show_error(const char *s, uint16_t errno);
void show_error_p(PGM_P progmem_s, uint16_t errno);
void show_title(const char *s);
void show_title_p(PGM_P progmem_s);
void show_text(const char *s);
void show_text_p(PGM_P progmem_s);
void clear_working_line(void);
void menu_member(PGM_P progmem_s);*/

#include "audio.h"
#include "lcd.h"
#include "menu.h"
#include "nec.h"
#include "radio.h"
#include "ui.h"
#include <avr/interrupt.h>

int main(void) {
    ui_init();
    nec_init();
    lcd_init();
    menu_init();
    sei();
    _delay_ms(200);

    uint16_t ret;
    if (ret = audio_init())
        show_error(EXTERNAL, ret);

    if (ret = radio_init(true, 9600))
        show_error(RADIO, ret);

    menu_home();
    ui_led_toggle();
    
    for (;;) {
        if (nec_is_ready()) {
            ui_led_toggle();
            menu_update(nec_get_command(), nec_is_repeat());
            ui_led_toggle();
            nec_reset();
        }
    }

    return 0;
}
/*
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
    sett_val = *(opt.val);
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

            if (cmd == REMOTE_MINUS && *(opt.val) > 0) {
                (*(opt.val))--;
            } else if (cmd == REMOTE_PLUS && *(opt.val) < opt.data.options.count - 1) {
                (*(opt.val))++;
            }
            break;

        case REMOTE_BACKWARD:
        case REMOTE_FORWARD:
            if (opt.type == OPTIONS)
                return;

            if (cmd == REMOTE_BACKWARD && *(opt.val) > opt.data.slider.min) {
                *(opt.val) -= opt.data.slider.step;
            } else if (cmd == REMOTE_FORWARD && *(opt.val) < opt.data.slider.max) {
                *(opt.val) += opt.data.slider.step;
            }
            break;

        case REMOTE_CLEAR:
            if (!repeat) {
                *(opt.val) = opt.init_val;
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
                *(opt.val) = sett_val;
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
            (*(opt.val) == 0) ? up_arrow :
            (*(opt.val) == opt.data.options.count - 1) ? down_arrow : vert_arrows, 0, 1);
        lcd_goto(2, 1);
        lcd_puts_p((PGM_P) pgm_read_word(&(opt.data.options.name[*(opt.val)])));
    } else if (opt.type == SLIDER) {
        if (*(opt.val) > opt.data.slider.min)
            lcd_put_def_p(left_arrow, 0, 1);

        if (*(opt.val) < opt.data.slider.max)
            lcd_put_def_p(right_arrow, LCD_DISP_LENGTH - 1, 1);

        utoa(*(opt.val), buffer, 10);
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
