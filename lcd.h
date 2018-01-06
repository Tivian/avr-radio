#ifndef LCD_H_
#define LCD_H_

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd_custom.h"

#define LCD_LINES           2
#define LCD_DISP_LENGTH    16
#define LCD_LINE_LENGTH  0x40
#define LCD_START_LINE1  0x00
#define LCD_START_LINE2  0x40

#define LCD_DELAY_BOOTUP   16000
#define LCD_DELAY_INIT      5000
#define LCD_DELAY_INIT_REP    64
#define LCD_DELAY_INIT_4BIT   64
#define LCD_DELAY_BUSY_FLAG    4
#define LCD_DELAY_ENABLE_PULSE 1

/* instruction register bit positions, see HD44780U data sheet */
#define LCD_CLR               0      /* DB0: clear display                  */
#define LCD_HOME              1      /* DB1: return to home position        */
#define LCD_ENTRY_MODE        2      /* DB2: set entry mode                 */
#define LCD_ENTRY_INC         1      /*   DB1: 1=increment, 0=decrement     */
#define LCD_ENTRY_SHIFT       0      /*   DB2: 1=display shift on           */
#define LCD_ON                3      /* DB3: turn lcd/cursor on             */
#define LCD_ON_DISPLAY        2      /*   DB2: turn display on              */
#define LCD_ON_CURSOR         1      /*   DB1: turn cursor on               */
#define LCD_ON_BLINK          0      /*     DB0: blinking cursor ?          */
#define LCD_MOVE              4      /* DB4: move cursor/display            */
#define LCD_MOVE_DISP         3      /*   DB3: move display (0-> cursor) ?  */
#define LCD_MOVE_RIGHT        2      /*   DB2: move right (0-> left) ?      */
#define LCD_FUNCTION          5      /* DB5: function set                   */
#define LCD_FUNCTION_8BIT     4      /*   DB4: set 8BIT mode (0->4BIT mode) */
#define LCD_FUNCTION_2LINES   3      /*   DB3: two lines (0->one line)      */
#define LCD_FUNCTION_10DOTS   2      /*   DB2: 5x10 font (0->5x7 font)      */
#define LCD_CGRAM             6      /* DB6: set CG RAM address             */
#define LCD_DDRAM             7      /* DB7: set DD RAM address             */
#define LCD_BUSY              7      /* DB7: LCD is busy                    */

/* set entry mode: display shift on/off, dec/inc cursor move direction */
#define LCD_ENTRY_DEC            0x04   /* display shift off, dec cursor move dir */
#define LCD_ENTRY_DEC_SHIFT      0x05   /* display shift on,  dec cursor move dir */
#define LCD_ENTRY_INC_           0x06   /* display shift off, inc cursor move dir */
#define LCD_ENTRY_INC_SHIFT      0x07   /* display shift on,  inc cursor move dir */

/* display on/off, cursor on/off, blinking char at cursor position */
#define LCD_DISP_OFF             0x08   /* display off                            */
#define LCD_DISP_ON              0x0C   /* display on, cursor off                 */
#define LCD_DISP_ON_BLINK        0x0D   /* display on, cursor off, blink char     */
#define LCD_DISP_ON_CURSOR       0x0E   /* display on, cursor on                  */
#define LCD_DISP_ON_CURSOR_BLINK 0x0F   /* display on, cursor on, blink char      */

/* move cursor/shift display */
#define LCD_MOVE_CURSOR_LEFT     0x10   /* move cursor left  (decrement)          */
#define LCD_MOVE_CURSOR_RIGHT    0x14   /* move cursor right (increment)          */
#define LCD_MOVE_DISP_LEFT       0x18   /* shift display left                     */
#define LCD_MOVE_DISP_RIGHT      0x1C   /* shift display right                    */

/* function set: set interface data length and number of display lines */
#define LCD_FUNCTION_4BIT_1LINE  0x20   /* 4-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_4BIT_2LINES 0x28   /* 4-bit interface, dual line,   5x7 dots */
#define LCD_FUNCTION_8BIT_1LINE  0x30   /* 8-bit interface, single line, 5x7 dots */
#define LCD_FUNCTION_8BIT_2LINES 0x38   /* 8-bit interface, dual line,   5x7 dots */

#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#define LCD_MODE_DEFAULT     ((1<<LCD_ENTRY_MODE) | (1<<LCD_ENTRY_INC) )

#ifndef LCD_RS_PORT
#define LCD_RS_PORT PORTD
#endif
#ifndef LCD_RS_PIN
#define LCD_RS_PIN  PD2
#endif

#ifndef LCD_RW_PORT
#define LCD_RW_PORT PORTD
#endif
#ifndef LCD_RW_PIN
#define LCD_RW_PIN  PD3
#endif

#ifndef LCD_E_PORT
#define LCD_E_PORT PORTD
#endif
#ifndef LCD_E_PIN
#define LCD_E_PIN  PD4
#endif

#ifndef LCD_DB4_PORT
#define LCD_DB4_PORT PORTD
#endif
#ifndef LCD_DB4_PIN
#define LCD_DB4_PIN  PD5
#endif

#ifndef LCD_DB5_PORT
#define LCD_DB5_PORT PORTD
#endif
#ifndef LCD_DB5_PIN
#define LCD_DB5_PIN  PD6
#endif

#ifndef LCD_DB6_PORT
#define LCD_DB6_PORT PORTD
#endif
#ifndef LCD_DB6_PIN
#define LCD_DB6_PIN  PD7
#endif

#ifndef LCD_DB7_PORT
#define LCD_DB7_PORT PORTB
#endif
#ifndef LCD_DB7_PIN
#define LCD_DB7_PIN  PB0
#endif

#define DDR(x) (*(&(x) - 1))
#define PIN(x) (*(&(x) - 2))

#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN);
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN);

#define lcd_rw_high()   LCD_RW_PORT |=  _BV(LCD_RW_PIN);
#define lcd_rw_low()    LCD_RW_PORT &= ~_BV(LCD_RW_PIN);

#define lcd_e_delay()   _delay_us(LCD_DELAY_ENABLE_PULSE);
#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN);
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN);
#define lcd_e_toggle()  lcd_e_high(); lcd_e_delay(); lcd_e_low();

#define lcd_db4_high()  LCD_DB4_PORT |=  _BV(LCD_DB4_PIN);
#define lcd_db4_low()   LCD_DB4_PORT &= ~_BV(LCD_DB4_PIN);

#define lcd_db5_high()  LCD_DB5_PORT |=  _BV(LCD_DB5_PIN);
#define lcd_db5_low()   LCD_DB5_PORT &= ~_BV(LCD_DB5_PIN);

#define lcd_db6_high()  LCD_DB6_PORT |=  _BV(LCD_DB6_PIN);
#define lcd_db6_low()   LCD_DB6_PORT &= ~_BV(LCD_DB6_PIN);

#define lcd_db7_high()  LCD_DB7_PORT |=  _BV(LCD_DB7_PIN);
#define lcd_db7_low()   LCD_DB7_PORT &= ~_BV(LCD_DB7_PIN);

#define lcd_data_output() \
    DDR(LCD_DB4_PORT) |= _BV(LCD_DB4_PIN);\
    DDR(LCD_DB5_PORT) |= _BV(LCD_DB5_PIN);\
    DDR(LCD_DB6_PORT) |= _BV(LCD_DB6_PIN);\
    DDR(LCD_DB7_PORT) |= _BV(LCD_DB7_PIN);

#define lcd_data_input() \
    DDR(LCD_DB4_PORT) &= ~_BV(LCD_DB4_PIN);\
    DDR(LCD_DB5_PORT) &= ~_BV(LCD_DB5_PIN);\
    DDR(LCD_DB6_PORT) &= ~_BV(LCD_DB6_PIN);\
    DDR(LCD_DB7_PORT) &= ~_BV(LCD_DB7_PIN);

#define lcd_data_clear() \
    lcd_db4_low(); lcd_db5_low();\
    lcd_db6_low(); lcd_db7_low();\


#define lcd_data_inactive() \
    lcd_db4_high(); lcd_db5_high();\
    lcd_db6_high(); lcd_db7_high();

#define lcd_data_set_high(data) \
    if ((data) & 0x10) lcd_db4_high();\
    if ((data) & 0x20) lcd_db5_high();\
    if ((data) & 0x40) lcd_db6_high();\
    if ((data) & 0x80) lcd_db7_high();\

#define lcd_data_set_low(data) \
    if ((data) & 0x01) lcd_db4_high();\
    if ((data) & 0x02) lcd_db5_high();\
    if ((data) & 0x04) lcd_db6_high();\
    if ((data) & 0x08) lcd_db7_high();

#define lcd_data_get_high(data) \
    if (PIN(LCD_DB4_PORT) & _BV(LCD_DB4_PIN)) (data) |= 0x10;\
    if (PIN(LCD_DB5_PORT) & _BV(LCD_DB5_PIN)) (data) |= 0x20;\
    if (PIN(LCD_DB6_PORT) & _BV(LCD_DB6_PIN)) (data) |= 0x40;\
    if (PIN(LCD_DB7_PORT) & _BV(LCD_DB7_PIN)) (data) |= 0x80;

#define lcd_data_get_low(data) \
    if (PIN(LCD_DB4_PORT) & _BV(LCD_DB4_PIN)) (data) |= 0x01;\
    if (PIN(LCD_DB5_PORT) & _BV(LCD_DB5_PIN)) (data) |= 0x02;\
    if (PIN(LCD_DB6_PORT) & _BV(LCD_DB6_PIN)) (data) |= 0x04;\
    if (PIN(LCD_DB7_PORT) & _BV(LCD_DB7_PIN)) (data) |= 0x08;

void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_goto(uint8_t x, uint8_t y);
uint8_t lcd_getxy(void);
uint8_t lcd_gety(void);
uint8_t lcd_getx(void);
void lcd_clr(void);
void lcd_home(void);
void lcd_putc(char c);
void lcd_puts(const char *s);
void lcd_puts_p(PGM_P progmem_s);
void lcd_put_def(const uint8_t *c, uint8_t x, uint8_t y);
void lcd_put_def_p(const uint8_t *progmem_c, uint8_t x, uint8_t y);
uint8_t lcd_get_next_char_id(void);
uint8_t lcd_define_char(const uint8_t *c);
uint8_t lcd_define_char_id(const uint8_t *c, uint8_t id);
uint8_t lcd_define_char_p(const uint8_t *progmem_c);
uint8_t lcd_define_char_id_p(const uint8_t *progmem_c, uint8_t id);
void lcd_clear_line(uint8_t line);
void lcd_init(void);

#endif
