#include "menu.hpp"
#include "audio.hpp"
#include "brightness.hpp"
#include "interval.hpp"
#include "lcd.hpp"
#include "memory.hpp"
#include "radio.hpp"
#include "remote.hpp"
#include "ui.hpp"
#include "utility.hpp"
#include <avr/pgmspace.h>

namespace Menu {
namespace {
    inline constexpr auto INTERVAL_SPEED = Interval::speed_t::slower;

    const char muted_str[]    PROGMEM = "MUTED";
    const char volume_str[]   PROGMEM = "  VOLUME   ";
    const char external_str[] PROGMEM = "EXTERNAL";
    const char bass_str[]     PROGMEM = " BASS     ";
    const char treble_str[]   PROGMEM = " TREBLE   ";
    const char balance_str[]  PROGMEM = " BALANCE  ";
    const char settings_str[] PROGMEM = "SETTINGS";

    void external(const NEC::data_t& data);
    void radio(const NEC::data_t& data);
    void equalizer(const NEC::data_t& data);
    void settings(const NEC::data_t& data);

    void (*prev_screen)(const NEC::data_t&) = nullptr;
    void (*current_screen)(const NEC::data_t&) = external;

    bool _busy = false;

    void show_volume() {
        _busy = true;
        Interval::set(INTERVAL_SPEED, []() {
            if (_busy && current_screen != settings) {
                LCD::clear(1);
                _busy = false;
            }
            return true;
        });

        auto v = Audio::config().volume - 27;
        if (Audio::config().mute || v == 0) {
            strcpy_P(Memory::buffer, muted_str);
            LCD::puts(Memory::buffer, 1, true);
        } else {
            LCD::pos(0, 1);
            strcpy_P(Memory::buffer, volume_str);
            LCD::puts(Memory::buffer);
            LCD::putc(v >= 10 ? '0' + (v / 10) : ' ');
            LCD::putc('0' + (v % 10));
            LCD::put_space(3);
        }
    }

    void mute() {
        Audio::mute_toggle();
        show_volume();
    }

    void common(const NEC::data_t& data) {
        Remote::btn_t cmd = static_cast<Remote::btn_t>(data.cmd);

        switch (cmd) {
            case Remote::btn_t::plus:
            case Remote::btn_t::minus: {
                static uint8_t series = 0;
                if (data.repeat) {
                    if (++series == 1)
                        break;
                    else if (series == 2)
                        series = 1;
                } else {
                    series = 0;
                }

                Audio::volume(cmd == Remote::btn_t::plus);
                if (Audio::config().mute)
                    Audio::mute(false);

                show_volume();
                break; }

            case Remote::btn_t::play:
                if (!data.repeat)
                    mute();
                break;

            case Remote::btn_t::test:
            case Remote::btn_t::menu:
                if (!data.repeat) {
                    prev_screen = current_screen;
                    current_screen = cmd == Remote::btn_t::test ? equalizer : settings;
                    show();
                    return;
                }
                break;

            case Remote::btn_t::back: {
                auto fx = Interval::get(INTERVAL_SPEED);
                if (fx != nullptr)
                    fx();
            }
            default:
                return;
        }

        Audio::update();
    }

    void external(const NEC::data_t& data) {
        Remote::btn_t cmd = static_cast<Remote::btn_t>(data.cmd);
        strcpy_P(Memory::buffer, external_str);
        LCD::puts(Memory::buffer, 0, true);

        switch (cmd) {
            case Remote::btn_t::clear:
                if (!data.repeat) {
                    prev_screen = current_screen;
                    current_screen = radio;
                    Radio::unmute(false);
                    Audio::source(Audio::source_t::second);
                    LCD::clear();
                    Radio::show_info(true);
                    return;
                }
                break;

            default:
                common(data);
                return;
        }
    }

    void radio(const NEC::data_t& data) {
        Remote::btn_t cmd = static_cast<Remote::btn_t>(data.cmd);

        switch (cmd) {
            case Remote::btn_t::clear:
                if (!data.repeat) {
                    prev_screen = current_screen;
                    current_screen = external;
                    Radio::mute();
                    Audio::source(Audio::source_t::first);
                    LCD::clear();
                    show();
                    return;
                }
                break;

            case Remote::btn_t::prev:
                _busy = false;
                Radio::seek_down();
                break;

            case Remote::btn_t::next:
                _busy = false;
                Radio::seek_up();
                break;

            default: {
                static uint8_t series;
                auto digit = *cmd;
                if (digit != Remote::not_digit) {
                    if (!data.repeat) {
                        series = 0;
                        _busy = false;
                        Radio::channel(digit);
                    } else if (++series == 30) {
                        series = 0;
                        uint16_t freq = Radio::get_freq();
                        Radio::channel(digit, freq, Radio::channel(digit) == freq);
                    }
                } else {
                    common(data);
                    return;
                }
            }
        }
    }

    void equalizer(const NEC::data_t& data) {
        static Audio::eq_t to_change = Audio::eq_t::balance;
        bool changed = false, delta;
        int8_t value;

        Remote::btn_t cmd = static_cast<Remote::btn_t>(data.cmd);
        switch (cmd) {
            case Remote::btn_t::test:
                if (!data.repeat)
                    ++to_change;
                break;

            case Remote::btn_t::prev:
            case Remote::btn_t::next: {
                static uint8_t series = 0;
                if (data.repeat) {
                    if (++series == 1)
                        break;
                    else if (series == 2)
                        series = 1;
                } else {
                    series = 0;
                }

                delta = cmd == Remote::btn_t::next;
                changed = true;
                break; }

            case Remote::btn_t::menu:
                break;

            default:
                if (&data != &NEC::empty) {
                    to_change = Audio::eq_t::balance;
                    current_screen = prev_screen;
                    LCD::clear(1);
                    current_screen(data);
                    return;
                }
                break;
        }

        _busy = true;
        Interval::set(INTERVAL_SPEED, []() {
            if (_busy) {
                LCD::clear(1);
                to_change = Audio::eq_t::balance;
                current_screen = prev_screen;
                show();
                _busy = false;
            }
            return true;
        });

        if (changed) {
            Audio::equalize(to_change, delta);
            Audio::update();
        }

        const char* const eq_label[] = { bass_str, treble_str, balance_str };
        LCD::clear(1);
        strcpy_P(Memory::buffer, eq_label[*to_change]);
        LCD::puts(Memory::buffer);
        value = Audio::equalize(to_change);
        LCD::putc(value == 0 ? ' ' : value < 0 ? '-' : '+');
        value = std::abs(value);
        LCD::putc(value < 10 ? ' ' : '0' + (value / 10));
        LCD::putc('0' + (value % 10));
        if (to_change != Audio::eq_t::balance)
            LCD::puts("dB");
    }

    const uint8_t left_arrow[]  PROGMEM = { 0x03, 0x06, 0x0C, 0x18, 0x18, 0x0C, 0x06, 0x03 };
    const uint8_t right_arrow[] PROGMEM = { 0x18, 0x0C, 0x06, 0x03, 0x03, 0x06, 0x0C, 0x18 };
    const char brightness_str[] PROGMEM = "BRIGHTNESS";
    const char stereo_str[]     PROGMEM = "STEREO";
    const char mono_str[]       PROGMEM = "MONO";
    const char linear_str[]     PROGMEM = "LINEAR";
    const char pseudo_str[]     PROGMEM = "PSEUDO";
    const char spatial_str[]    PROGMEM = "SPATIAL";

    inline constexpr uint8_t left_arrow_id  = 0;
    inline constexpr uint8_t right_arrow_id = 1;

    struct {
        enum : uint8_t {
            brightness, stereo
        } choice;

        uint8_t stage = 0;

        inline constexpr bool operator!() {
            return !(choice == brightness && stage == 1);
        }

        inline constexpr const char* operator&() {
            switch (choice) {
                case brightness:
                    return brightness_str;
                case stereo:
                    return stereo_str;
                default:
                    return nullptr;
            }
        }

        inline const char* operator*() {
            switch (choice) {
                case brightness: {
                    const auto& val = Brightness::value;
                    Memory::buffer[0] = val < 100 ? ' ' : '0' + (val / 100);
                    Memory::buffer[1] = val < 10 ? ' ' : '0' + ((val % 100) / 10);
                    Memory::buffer[2] = '0' + (val % 10);
                    Memory::buffer[3] = '\0';
                    return nullptr; }
                case stereo:
                    using namespace Audio;
                    switch (config().stereo) {
                        case stereo_t::mono:
                            return mono_str;
                        case stereo_t::linear:
                            return linear_str;
                        case stereo_t::pseudo:
                            return pseudo_str;
                        case stereo_t::spatial:
                            return spatial_str;
                        default:
                            return nullptr;
                    }
                default:
                    return nullptr;
            }
        }

        inline constexpr void operator++() {
            switch (choice) {
                case brightness:
                    if (stage == 0) {
                        choice = stereo;
                    } else if (Brightness::value != 0xff) {
                        ++(Brightness::value);
                    }
                    break;
                case stereo:
                    if (stage == 0) {
                        choice = brightness;
                    } else {
                        using namespace Audio;
                        switch (config().stereo) {
                            case stereo_t::mono:
                                config().stereo = stereo_t::linear;
                                break;

                            case stereo_t::linear:
                                config().stereo = stereo_t::pseudo;
                                break;

                            case stereo_t::pseudo:
                                config().stereo = stereo_t::spatial;
                                break;

                            case stereo_t::spatial:
                            default:
                                config().stereo = stereo_t::mono;
                        }
                        update();
                    }
                    break;
            }
        }

        inline constexpr void operator--() {
            switch (choice) {
                case brightness:
                    if (stage == 0) {
                        choice = stereo;
                    } else if (Brightness::value != 0x00) {
                        --(Brightness::value);
                    }
                    break;
                case stereo:
                    if (stage == 0) {
                        choice = brightness;
                    } else {
                        using namespace Audio;
                        switch (config().stereo) {
                            case stereo_t::mono:
                                config().stereo = stereo_t::spatial;
                                break;

                            case stereo_t::spatial:
                                config().stereo = stereo_t::pseudo;
                                break;

                            case stereo_t::pseudo:
                                config().stereo = stereo_t::linear;
                                break;

                            case stereo_t::linear:
                            default:
                                config().stereo = stereo_t::mono;
                        }
                        update();
                    }
                    break;
            }
        }
    } setting;

    void settings(const NEC::data_t& data) {
        if (data.repeat && !setting)
            return;

        auto ptr = setting.stage == 0 ? settings_str : *setting;
        if (ptr != nullptr)
            strcpy_P(Memory::buffer, ptr);
        LCD::puts(Memory::buffer, setting.stage != 0, true);

        strcpy_P(Memory::buffer, &setting);
        LCD::puts(Memory::buffer, setting.stage == 0, true);

        memcpy_P(Memory::buffer, left_arrow, 8);
        LCD::define_char_id((uint8_t*) Memory::buffer, left_arrow_id);
        LCD::pos(0, 1);
        LCD::putc(left_arrow_id);

        memcpy_P(Memory::buffer, right_arrow, 8);
        LCD::define_char_id((uint8_t*) Memory::buffer, right_arrow_id);
        LCD::pos(15, 1);
        LCD::putc(right_arrow_id);

        Remote::btn_t cmd = static_cast<Remote::btn_t>(data.cmd);
        switch (cmd) {
            case Remote::btn_t::prev:
                --setting;
                break;
            
            case Remote::btn_t::next:
                ++setting;
                break;

            case Remote::btn_t::play:
                if (!data.repeat)
                    setting.stage = (setting.stage == 0);
                break;

            case Remote::btn_t::back:
            case Remote::btn_t::menu:
                if (setting.stage == 0) {
                    current_screen = prev_screen;
                    LCD::clear(1);
                    show();
                } else {
                    setting.stage = 0;
                    break;
                }
            default:
                return;
        }

        show();
    }
}

bool busy() {
    return _busy;
}

bool showing_radio() {
    return current_screen == radio;
}

void set(bool to_radio) {
    current_screen = to_radio ? radio : external;
}

void show() {
    show(NEC::empty);
}

void show(const NEC::data_t& data) {
    current_screen(data);
}

void reset() {
    setting.stage = 0;
    _busy = false;
}
}
