#include "utility.hpp"
#include "lcd.hpp"

inline void lcd_hex_digit(uint8_t x) {
    LCD::putc((x >= 10 ? 'A' - 10 : '0') + x);
}

void lcd_hex(uint8_t x) {
    lcd_hex_digit(x >> 4);
    lcd_hex_digit(x & 0xf);
}