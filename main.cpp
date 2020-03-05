#include "audio.hpp"
#include "brightness.hpp"
#include "i2c_master.hpp"
#include "interval.hpp"
#include "lcd.hpp"
#include "nec.hpp"
#include "menu.hpp"
#include "memory.hpp"
#include "radio.hpp"
#include "sleep.hpp"
#include "ui.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>

namespace {
    bool init = true;
    bool radio_update = false;
}

int main(void) {
    Brightness::init();
    I2C::init(33);
    LCD::init();
    NEC::init();
    Sleep::init();
    UI::init();
    Interval::init(Interval::clock_speed_t::slowest);

    Sleep::on_asleep = []() {
        if (!init) {
            Memory::save();
            Radio::shutdown();
        } else {
            init = false;
        }

        UI::power_led<false>();
        UI::global_mute<true>();
        Brightness::value = 0x00;
        NEC::indicate = nullptr;
        LCD::clear();
        Menu::reset();
        UI::action_led<false>();
    };
    Sleep::on_wakeup = []() {
        UI::power_led<true>();
        _delay_ms(1000);
        Memory::load();
        Radio::init();
        UI::global_mute<false>();
        NEC::indicate = UI::action_led_blink;
        Menu::show();
    };
    Sleep::keep_sleeping = []() -> bool {
        while (!NEC::ready() && !Sleep::awaken_by_button());

        if (Sleep::awaken_by_button()) {
            return false;
        } else {
            auto& nec_data = NEC::data();
            NEC::reset();
            return nec_data.repeat || nec_data.cmd != 0x45;
        }
    };

    LCD::clear();
    Sleep::activate();
    Audio::init();

    sei();
    for (;;) {
        if (NEC::ready()) {
            auto& nec_data = NEC::data();
            NEC::reset();

            Menu::show(nec_data);
            if (!nec_data.repeat && nec_data.cmd == 0x45)
                Sleep::activate();
        }

        if (!Menu::busy() && Menu::showing_radio()) {
            Radio::show_info(radio_update);
            radio_update = false;
        } else {
            radio_update = true;
        }
    }

    return 0;
}
