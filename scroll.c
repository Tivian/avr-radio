#include "scroll.h"
#include "interval.h"
#include "lcd.h"
#include <string.h>

static struct {
    const char *str;
    PGM_P progmem_s;
    uint8_t pos;
    uint8_t len;
} global;

static void _scroll(void) {
    lcd_clear_line(1);

    if (global.pos < global.len) {
        lcd_goto(0, 1);
        lcd_puts(&global.str[global.pos]);
    }

    if (global.len - global.pos < 3) {
        lcd_goto(LCD_DISP_LENGTH - global.pos + global.len - 3, 1);
        lcd_puts(&global.str[0]);
    }

    global.pos++;
    if (LCD_DISP_LENGTH - global.pos + global.len - 3 == 0) {
        global.pos = 0;
    }
}

static void _scroll_p(void) {
    lcd_clear_line(1);

    if (global.pos < global.len) {
        lcd_goto(0, 1);
        lcd_puts_p(&global.progmem_s[global.pos]);
    }

    if (global.len - global.pos < 3) {
        lcd_goto(LCD_DISP_LENGTH - global.pos + global.len - 3, 1);
        lcd_puts_p(&global.progmem_s[0]);
    }

    global.pos++;
    if (LCD_DISP_LENGTH - global.pos + global.len - 3 == 0) {
        global.pos = 0;
    }
}

void scroll_text(const char *str, uint16_t speed) {
    global.pos = 0;
    global.len = strlen(str);
    global.str = str;
    set_interval(speed, _scroll);
}

void scroll_text_p(PGM_P progmem_s, uint16_t speed) {
    global.pos = 0;
    global.len = strlen_P(progmem_s);
    global.progmem_s = progmem_s;
    set_interval(speed, _scroll_p);
}

void stop_scroll(void) {
    clear_interval();
}
