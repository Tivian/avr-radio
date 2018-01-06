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
