#include "lcd.h"

static void lcd_write(uint8_t data, uint8_t rs) {
    if (rs) {
        lcd_rs_high();
    } else {
        lcd_rs_low();
    }

    lcd_rw_low();
    lcd_data_output();

    lcd_data_clear();
    lcd_data_set_high(data);
    lcd_e_toggle();

    lcd_data_clear();
    lcd_data_set_low(data);
    lcd_e_toggle();

    lcd_data_inactive();
}

static uint8_t lcd_read(uint8_t rs) {
    uint8_t data = 0;

    if (rs) {
        lcd_rs_high();
    } else {
        lcd_rs_low();
    }

    lcd_rw_high();
    lcd_data_input();

    lcd_e_high();
    lcd_e_delay();
    lcd_data_get_high(data);
    lcd_e_low();

    lcd_e_delay();

    lcd_e_high();
    lcd_e_delay();
    lcd_data_get_low(data);
    lcd_e_low();

    return data;
}

static uint8_t lcd_wait(void) {
    uint16_t timeout = 0;
    while((lcd_read(0) & _BV(LCD_BUSY)) && (timeout < (F_CPU / 500))) {
        timeout++;
    }

    _delay_us(LCD_DELAY_BUSY_FLAG);
    return lcd_read(0);
}

static inline void lcd_newline(uint8_t pos) {
    lcd_command(
        _BV(LCD_DDRAM) +
        (pos < LCD_START_LINE2 ? LCD_START_LINE2 : LCD_START_LINE1)
    );
}

void lcd_command(uint8_t cmd) {
    lcd_wait();
    lcd_write(cmd, 0);
}

void lcd_data(uint8_t data) {
    lcd_wait();
    lcd_write(data, 1);
}

void lcd_goto(uint8_t x, uint8_t y) {
    lcd_command(
        _BV(LCD_DDRAM) +
        (y == 0 ? LCD_START_LINE1 : LCD_START_LINE2) +
        x
    );
}

uint8_t lcd_getxy(void) {
    return lcd_wait();
}

void lcd_clr(void) {
    lcd_command(_BV(LCD_CLR));
}

void lcd_home(void) {
    lcd_command(_BV(LCD_HOME));
}

void lcd_putc(char c) {
    uint8_t pos = lcd_wait();
    if (c == '\n') {
        lcd_newline(pos);
    } else {
        lcd_write(c, 1);
    }
}

void lcd_puts(const char *s) {
    while (*s) {
        lcd_putc(*s++);
    }
}

void lcd_puts_p(PGM_P progmem_s) {
    char c;
    while (c = pgm_read_byte(progmem_s++)) {
        lcd_putc(c);
    }
}

static void _lcd_put_def(uint8_t char_id, uint8_t x, uint8_t y) {
    lcd_goto(x, y);
    lcd_putc(char_id);
}

void lcd_put_def(const uint8_t *c, uint8_t x, uint8_t y) {
    _lcd_put_def(lcd_define_char(c), x, y);
}

void lcd_put_def_p(const uint8_t *progmem_c, uint8_t x, uint8_t y) {
    _lcd_put_def(lcd_define_char_p(progmem_c), x, y);
}

uint8_t lcd_get_next_char_id(void) {
    static uint8_t user_defined = 7;

    user_defined++;
    if (user_defined == 8)
        user_defined = 0;

    return user_defined;
}

uint8_t lcd_define_char(const uint8_t *c) {
    return lcd_define_char_id(c, lcd_get_next_char_id());
}

uint8_t lcd_define_char_id(const uint8_t *c, uint8_t id) {
    lcd_command(_BV(LCD_CGRAM) + 8 * id);
    for (uint8_t i = 0; i < 8; i++)
        lcd_data(c[i]);

    return id;
}

uint8_t lcd_define_char_p(const uint8_t *progmem_c) {
    return lcd_define_char_id_p(progmem_c, lcd_get_next_char_id());
}

uint8_t lcd_define_char_id_p(const uint8_t *progmem_c, uint8_t id) {
    lcd_command(_BV(LCD_CGRAM) + 8 * id);
    for (uint8_t i = 0; i < 8; i++)
        lcd_data(pgm_read_byte(progmem_c++));

    return id;
}

void lcd_clear_line(uint8_t line) {
    lcd_goto(0, line);
    for (uint8_t i = 0; i < LCD_DISP_LENGTH; i++)
        lcd_putc(' ');
    lcd_goto(0, line);
}

void lcd_init(void) {
    DDR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
    DDR(LCD_RW_PORT) |= _BV(LCD_RW_PIN);
    DDR(LCD_E_PORT)  |= _BV(LCD_E_PIN);
    lcd_data_output();

    _delay_us(LCD_DELAY_BOOTUP);

    lcd_db5_high();
    lcd_db4_high();
    lcd_e_toggle();
    _delay_us(LCD_DELAY_INIT);

    lcd_e_toggle();
    _delay_us(LCD_DELAY_INIT_REP);

    lcd_e_toggle();
    _delay_us(LCD_DELAY_INIT_REP);

    lcd_db4_low();
    lcd_e_toggle();
    _delay_us(LCD_DELAY_INIT_4BIT);

    lcd_command(LCD_FUNCTION_DEFAULT);
    lcd_command(LCD_DISP_OFF);
    lcd_clr();
    lcd_command(LCD_MODE_DEFAULT);
    lcd_command(LCD_DISP_ON);
}
