#include "radio.hpp"
#include "i2c_master.hpp"
#include "lcd.hpp"
#include "memory.hpp"
#include <string.h>

#include <stdlib.h>

namespace Radio {
namespace {
const char megahertz_str[] PROGMEM = " MHz";
bool after_init;

void set_register(uint8_t reg, const uint16_t val) {
    I2C::start(RANDOM_ACCESS_ADDRESS, I2C::mode_t::write);
    I2C::write(reg);
    I2C::write(val >> 8);
    I2C::write(val & 0xff);
    I2C::stop();
}

uint16_t get_register(uint8_t reg) {
    uint16_t out;

    I2C::start(RANDOM_ACCESS_ADDRESS, I2C::mode_t::write);
    I2C::write(reg);

    I2C::start(RANDOM_ACCESS_ADDRESS, I2C::mode_t::read);
    out = I2C::read_ack();
    out <<= 8;
    out |= I2C::read_nack();
    I2C::stop();

    return out;
}

void update_register(uint8_t reg, uint16_t mask, uint16_t value) {
    set_register(reg, get_register(reg) & ~mask | value);
}
}

void init(uint16_t band) {
    set_register(RDA5807M_REG_CONFIG,
        RDA5807M_FLG_DHIZ   | RDA5807M_FLG_DMUTE | RDA5807M_FLG_BASS
      | RDA5807M_FLG_SEEKUP | RDA5807M_FLG_RDS   | RDA5807M_FLG_NEW
      | RDA5807M_FLG_ENABLE);

    update_register(RDA5807M_REG_TUNING, RDA5807M_BAND_MASK, band);
    _delay_ms(100);
    load();
    after_init = true;
}

void shutdown() {
    save();
    set_register(RDA5807M_REG_CONFIG, 0x00);
}

inline void load() {
    uint16_t freq;
    Memory::ram_b.read_block(FREQ_ADDRESS, &freq, sizeof(uint16_t));
    set_freq(freq);
}

inline void save() {
    uint16_t freq = get_freq();
    Memory::ram_b.update_block(FREQ_ADDRESS, &freq, sizeof(uint16_t));
}

bool volume_up() {
    const uint8_t volume = get_register(RDA5807M_REG_VOLUME) & RDA5807M_VOLUME_MASK;

    if (volume == RDA5807M_VOLUME_MASK) {
        return false;
    } else {
        update_register(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, volume + 1);
        return true;
    }
}

bool volume_down(bool also_mute) {
    const uint8_t volume = get_register(RDA5807M_REG_VOLUME) & RDA5807M_VOLUME_MASK;

    if (volume) {
        update_register(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, volume - 1);
        if (!(volume - 1) && also_mute)
            mute();
        return true;
    } else {
        return false;
    }
}

void seek_up(bool wrap) {
    update_register(RDA5807M_REG_CONFIG,
        (RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK | RDA5807M_FLG_SKMODE),
        (RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK | (wrap ? 0x00 : RDA5807M_FLG_SKMODE)));
}

void seek_down(bool wrap) {
    update_register(RDA5807M_REG_CONFIG,
        (RDA5807M_FLG_SEEKUP | RDA5807M_FLG_SEEK | RDA5807M_FLG_SKMODE),
        (0x00 | RDA5807M_FLG_SEEK | (wrap ? 0x00 : RDA5807M_FLG_SKMODE)));
}

void mute() {
    update_register(RDA5807M_REG_CONFIG, RDA5807M_FLG_DMUTE, 0x00);
}

void unmute(bool min_volume) {
    if (min_volume)
        update_register(RDA5807M_REG_VOLUME, RDA5807M_VOLUME_MASK, 0x01);
    update_register(RDA5807M_REG_CONFIG, RDA5807M_FLG_DMUTE, RDA5807M_FLG_DMUTE);
}

uint16_t get_band_spacing() {
    uint8_t band = get_register(RDA5807M_REG_TUNING) & (RDA5807M_BAND_MASK | RDA5807M_SPACE_MASK);
    const uint8_t space = band & RDA5807M_SPACE_MASK;

    if (band & RDA5807M_BAND_MASK == RDA5807M_BAND_EAST &&
        !(get_register(RDA5807M_REG_BLEND) & RDA5807M_FLG_EASTBAND65M))
        band = (band >> RDA5807M_BAND_SHIFT) + 1;
    else
        band >>= RDA5807M_BAND_SHIFT;

    return (space << 8) | band;
}

uint16_t get_freq() {
    const uint16_t space_band = get_band_spacing();

    return pgm_read_word(&band_lower_limits[space_band & 0xff]) +
        (get_register(RDA5807M_REG_STATUS) & RDA5807M_READCHAN_MASK) *
        pgm_read_byte(&channel_spacings[space_band >> 8]) / 10;
}

bool set_freq(uint16_t freq) {
    const uint16_t space_band = get_band_spacing();
    const uint16_t origin = pgm_read_word(&band_lower_limits[space_band & 0xff]);

    if (freq < origin || freq > pgm_read_word(&band_higher_limits[space_band & 0xff]))
        return false;

    freq -= origin;

    const uint8_t spacing = pgm_read_byte(&channel_spacings[space_band >> 8]);
    if (freq * 10 % spacing)
        return false;

    update_register(RDA5807M_REG_TUNING, RDA5807M_CHAN_MASK | RDA5807M_FLG_TUNE,
        ((freq * 10 / spacing) << RDA5807M_CHAN_SHIFT) | RDA5807M_FLG_TUNE);

    return true;
}

uint8_t get_rssi() {
    return (get_register(RDA5807M_REG_RSSI) & RDA5807M_RSSI_MASK) >> RDA5807M_RSSI_SHIFT;
}

uint16_t channel(uint8_t index, bool set) {
    uint16_t freq;
    if (Memory::ram_b.read_block((const void*) (index * sizeof(uint16_t)), &freq, sizeof(uint16_t))) {
        if (set && freq && freq != 0xffff)
            set_freq(freq);
    }

    return freq;
}

namespace {
void _channel(uint8_t i = 0) {
    uint16_t freq = get_freq();
    LCD::pos(14, 0);
    for (; i < 10; i++) {
        if (freq == channel(i, false)) {
            LCD::putc('#');
            LCD::putc('0' + i);
            break;
        }
    }

    if (i == 10)
        LCD::put_space(2);
}
}

void channel(uint8_t index, uint16_t freq, bool remove) {
    if (remove)
        freq = 0xffff;

    Memory::ram_b.update_block((const void*) (index * sizeof(uint16_t)), &freq, sizeof(uint16_t));
    _channel(index);
}

namespace {
constexpr uint8_t rssi_char[]   PROGMEM { 0x1F, 0x1E, 0x1E, 0x1C, 0x1C, 0x18, 0x18, 0x10 };
constexpr uint8_t stereo_char[] PROGMEM { 0x0C, 0x10, 0x08, 0x04, 0x18, 0x00, 0x00, 0x00 };

inline constexpr uint8_t rssi_id   = 0;
inline constexpr uint8_t stereo_id = 1;

struct {
    struct {
        uint16_t country : 4;
        uint16_t area    : 4;
        uint16_t program : 8;
    } pi;
    uint8_t group : 4;
    uint8_t ab    : 1;

    struct {
        uint8_t segment   : 2;
        bool wait_for_new : 1;
        char program_name[9];
    } group0;
} rds;

struct {
    uint8_t strength : 3;
    bool stereo      : 1;
    uint16_t freq;

    uint8_t group   : 4;
    uint8_t segment : 2;
} status;
}

void show_info(bool update) {
    if (after_init) {
        after_init = false;
        update = true;
    }

    auto status_reg = get_register(RDA5807M_REG_STATUS);
    auto rssi = get_rssi();
    if (rssi > 63)
        rssi = 63;

    uint8_t strength = (7 - (rssi >> 3));
    if (update || status.strength != strength) {
        status.strength = strength;

        int8_t i;
        for (i = 7; i >= strength; i--)
            Memory::buffer[i] = pgm_read_byte(&rssi_char[i]);
        for (; i >= 0; i--)
            Memory::buffer[i] = 0x00;
        LCD::define_char_id((uint8_t*) Memory::buffer, rssi_id);

        LCD::pos(0, 0);
        LCD::putc(rssi_id);
    }

    bool is_stereo = status_reg & RDA5807M_STATUS_ST;
    if (update || status.stereo != is_stereo) {
        status.stereo = is_stereo;

        memcpy_P(Memory::buffer, stereo_char, 8);
        LCD::define_char_id((uint8_t*) Memory::buffer, stereo_id);
        LCD::pos(1, 0);
        LCD::putc(is_stereo ? stereo_id : ' ');
    }

    uint16_t freq = get_freq();
    if (update || status.freq != freq) {
        status.freq = freq;
        rds.group0.wait_for_new = true;
        LCD::clear(1);
        _channel();

        LCD::pos(3, 0);
        uint8_t integer_part = freq / 100;
        uint8_t fractional_part = freq % 100;
        uint8_t tmp = integer_part / 100;
        LCD::putc(tmp == 0 ? ' ' : '0' + tmp);
        LCD::putc('0' + ((integer_part / 10) % 10));
        LCD::putc('0' + (integer_part % 10));
        LCD::putc('.');
        LCD::putc('0' + (fractional_part / 10));
        strcpy_P(Memory::buffer, megahertz_str);
        LCD::puts((const char*) Memory::buffer);
    }

    bool rds_ready = (status_reg & RDA5807M_STATUS_RDSR) && !(status_reg & RDA5807M_STATUS_BLKE);
    auto rds_error_lvl = get_register(RDA5807M_REG_RSSI);
    bool rds_err = (rds_error_lvl & RDA5807M_BLERA_MASK) || (rds_error_lvl & RDA5807M_BLERB_MASK);
    if (rds_ready && !rds_err) {
        uint16_t raw_a = get_register(RDA5807M_REG_RDSA);
        uint16_t raw_b = get_register(RDA5807M_REG_RDSB);
        uint16_t raw_c = get_register(RDA5807M_REG_RDSC);
        uint16_t raw_d = get_register(RDA5807M_REG_RDSD);

        rds.group = (raw_b & 0xF000) >> 12;
        rds.ab = (raw_b & 0x0800) >> 11;

        if (rds.ab && raw_a != raw_c) {
            return;
        } else {
            rds.pi.country = (raw_a & 0xF000) >> 12;
            rds.pi.area = (raw_a & 0x0F00) >> 8;
            rds.pi.program = raw_a & 0xFF;
        }

        if (rds.pi.country != 3)
            return;

        switch (rds.group) {
            case 0:
                uint8_t old_segment = rds.group0.segment;
                rds.group0.segment = raw_b & 0x3;

                if (rds.group0.segment != old_segment) {
                    if (rds.group0.segment != 0 && rds.group0.wait_for_new)
                        return;

                    if (rds.group0.segment == 0) {
                        rds.group0.wait_for_new = false;
                        memset(rds.group0.program_name, 0, sizeof rds.group0.program_name);
                    } else if (rds.group0.segment != old_segment + 1) {
                        rds.group0.wait_for_new = true;
                        return;
                    }
                }

                rds.group0.program_name[2 * rds.group0.segment] = (raw_d & 0xFF00) >> 8;
                rds.group0.program_name[2 * rds.group0.segment + 1] = raw_d & 0xFF;

                if (rds.group0.segment != old_segment && rds.group0.segment == 3) {
                    rds.group0.wait_for_new = true;
                    rds.group0.program_name[8] = '\0';
                    LCD::puts(rds.group0.program_name, 1, true);
                }
                break;
        }
    }
}
}