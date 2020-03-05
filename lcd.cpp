#include "lcd.hpp"
#include "utility.hpp"
#include <string.h>

namespace LCD {
namespace {
    uint8_t y = 0;
    uint8_t user_defined = 7;
	
	inline void rs_high()  { LCD_RS_PORT  |=  _BV(LCD_RS_PIN);  }
	inline void rs_low()   { LCD_RS_PORT  &= ~_BV(LCD_RS_PIN);  }
	inline void e_high()   { LCD_E_PORT   |=  _BV(LCD_E_PIN);   }
	inline void e_low()    { LCD_E_PORT   &= ~_BV(LCD_E_PIN);   }
	inline void db4_high() { LCD_DB4_PORT |=  _BV(LCD_DB4_PIN); }
	inline void db4_low()  { LCD_DB4_PORT &= ~_BV(LCD_DB4_PIN); }
	inline void db5_high() { LCD_DB5_PORT |=  _BV(LCD_DB5_PIN); }
	inline void db5_low()  { LCD_DB5_PORT &= ~_BV(LCD_DB5_PIN); }
	inline void db6_high() { LCD_DB6_PORT |=  _BV(LCD_DB6_PIN); }
	inline void db6_low()  { LCD_DB6_PORT &= ~_BV(LCD_DB6_PIN); }
	inline void db7_high() { LCD_DB7_PORT |=  _BV(LCD_DB7_PIN); }
	inline void db7_low()  { LCD_DB7_PORT &= ~_BV(LCD_DB7_PIN); }

    inline void data_output(void) {
        DDR(LCD_DB4_PORT) |= _BV(LCD_DB4_PIN);
        DDR(LCD_DB5_PORT) |= _BV(LCD_DB5_PIN);
        DDR(LCD_DB6_PORT) |= _BV(LCD_DB6_PIN);
        DDR(LCD_DB7_PORT) |= _BV(LCD_DB7_PIN);
    }

    inline void data_set(uint8_t data) {
        if (data & 0x1) db4_high();
        else db4_low();

        if (data & 0x2) db5_high();
        else db5_low();

        if (data & 0x4) db6_high();
        else db6_low();

        if (data & 0x8) db7_high();
        else db7_low();
    }

    void e_toggle(void) {
        e_low();
        _delay_us(LCD_DELAY_ENABLE_PULSE);
        e_high();
        _delay_us(LCD_DELAY_ENABLE_PULSE);
        e_low();
        _delay_us(LCD_DELAY_CMD);
    }

    void write(uint8_t data, bool rs) {
        if (rs)
			rs_high();
        else
            rs_low();

        data_set(data >> 4);
        e_toggle();

        data_set(data);
        e_toggle();
    }

    void command(uint8_t cmd) {
        write(cmd, LCD_RS_INSTRUCTION);
    }

    void data(uint8_t data) {
        write(data, LCD_RS_DATA);
    }
}

void init() {
    DDR(LCD_RS_PORT) |= _BV(LCD_RS_PIN);
    DDR(LCD_E_PORT)  |= _BV(LCD_E_PIN);
    data_output();
    
    _delay_us(LCD_DELAY_BOOTUP);

    db5_high();
    db4_high();
    e_toggle();
    _delay_us(LCD_DELAY_INIT);

    e_toggle();
    _delay_us(LCD_DELAY_INIT_REP);

    e_toggle();
    _delay_us(LCD_DELAY_INIT_REP);

    db4_low();
    e_toggle();
    _delay_us(LCD_DELAY_INIT_4BIT);

    command(LCD_FUNCTION_DEFAULT);
    command(LCD_DISP_OFF);
    clear();
    command(LCD_MODE_DEFAULT);
    command(LCD_DISP_ON);
}

void pos(uint8_t x, uint8_t y) {
    LCD::y = y == 1;

    command(
        _BV(LCD_DDRAM) +
        (y == 0 ? LCD_START_LINE1 : LCD_START_LINE2) +
        x
    );
}

void clear() {
    y = 0;
    command(_BV(LCD_CLR));
    _delay_us(LCD_DELAY_CLEAR);
}

void clear(uint8_t line) {
    pos(0, line);
    put_space(LCD_DISP_LENGTH);
    pos(0, line);
}

void home() {
    y = 0;
    command(_BV(LCD_HOME));
    _delay_us(LCD_DELAY_CLEAR);
}

void newline() {
    pos(0, y + 1);
}

void putc(char c) {
    if (c == '\n')
        newline();
    else
        data(c);
}

void puts(const char* s) {
    while (*s) 
        putc(*s++);
}

void puts(const char* s, uint8_t line, bool also_clear) {
    if (also_clear)
        clear(line);
    pos((LCD_DISP_LENGTH - strlen(s)) / 2, line);
    puts(s);
}

void put_space(uint8_t len) {
    while (len--)
        putc(' ');
}

void put_def(const uint8_t* c, uint8_t x, uint8_t y) {
    auto id = define_char(c);
    pos(x, y);
    putc(id);
}

uint8_t get_next_char_id(void) {
    if (++user_defined == 8)
        user_defined = 0;

    return user_defined;
}

uint8_t define_char(const uint8_t* c) {
    return define_char_id(c, get_next_char_id());
}

uint8_t define_char_id(const uint8_t* c, uint8_t id) {
    command(_BV(LCD_CGRAM) | (id << 3));
    for (uint8_t i = 0; i < 8; i++)
        data(c[i]);

    return id;
}

inline void shift_left() {
    command(LCD_MOVE_DISP_LEFT);
}

inline void shift_right() {
    command(LCD_MOVE_DISP_RIGHT);
}
}