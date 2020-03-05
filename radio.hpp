#ifndef RADIO_HPP
#define RADIO_HPP

#include <avr/pgmspace.h>
#include <stdint.h>

namespace Radio {
static constexpr uint8_t RANDOM_ACCESS_ADDRESS  = 0x11;
static constexpr uint8_t SEQUENTIAL_RDA_ADDRESS = 0x10;
static constexpr uint8_t SEQUENTIAL_TEA_ADDRESS = 0x60;
static const void* FREQ_ADDRESS = (const void*) 0xF0;

// Register addresses
#define RDA5807M_REG_CHIPID 0x00
#define RDA5807M_REG_CONFIG 0x02
#define RDA5807M_REG_TUNING 0x03
#define RDA5807M_REG_GPIO   0x04
#define RDA5807M_REG_VOLUME 0x05
#define RDA5807M_REG_I2S    0x06
#define RDA5807M_REG_BLEND  0x07
#define RDA5807M_REG_FREQ   0x08
#define RDA5807M_REG_STATUS 0x0A
#define RDA5807M_REG_RSSI   0x0B
#define RDA5807M_REG_RDSA   0x0C
#define RDA5807M_REG_RDSB   0x0D
#define RDA5807M_REG_RDSC   0x0E
#define RDA5807M_REG_RDSD   0x0F
#define RDA5807M_REG_SEEK   0x20

// Status bits (from the chip)
#define RDA5807M_STATUS_RDSR  0x8000
#define RDA5807M_STATUS_STC   0x4000
#define RDA5807M_STATUS_SF    0x2000
#define RDA5807M_STATUS_RDSS  0x1000
#define RDA5807M_STATUS_BLKE  0x0800
#define RDA5807M_STATUS_ST    0x0400
#define RDA5807M_STATUS_ABCDE 0x0010

// Flag bits (to the chip)
#define RDA5807M_FLG_DHIZ        0x8000
#define RDA5807M_FLG_DMUTE       0x4000
#define RDA5807M_FLG_MONO        0x2000
#define RDA5807M_FLG_BASS        0x1000
#define RDA5807M_FLG_RCLKNOCAL   0x0800
#define RDA5807M_FLG_RCLKDIRECT  0x0400
#define RDA5807M_FLG_SEEKUP      0x0200
#define RDA5807M_FLG_SEEK        0x0100
#define RDA5807M_FLG_SKMODE      0x0080
#define RDA5807M_FLG_RDS         0x0008
#define RDA5807M_FLG_NEW         0x0004
#define RDA5807M_FLG_RESET       0x0002
#define RDA5807M_FLG_ENABLE      0x0001
#define RDA5807M_FLG_DIRECT      0x0020
#define RDA5807M_FLG_TUNE        0x0010
#define RDA5807M_FLG_DE          0x0800
#define RDA5807M_FLG_SOFTMUTE    0x0200
#define RDA5807M_FLG_AFCD        0x0100
#define RDA5807M_FLG_EASTBAND65M 0x0200
#define RDA5807M_FLG_SOFTBLEND   0x0002
#define RDA5807M_FLG_FREQMODE    0x0001
#define RDA5807M_FLG_FMTRUE      0x0100
#define RDA5807M_FLG_FMREADY     0x0080
#define RDA5807M_FLG_BLOCKE      0x0010

// Masks and constants for configuration parameters
#define RDA5807M_CHIPID 0x58
#define RDA5807M_CLKMODE_MASK (0x0070)
#define RDA5807M_CLKMODE_32K (0x0 << 4)
#define RDA5807M_CLKMODE_12M (0x1 << 4)
#define RDA5807M_CLKMODE_13M (0x2 << 4)
#define RDA5807M_CLKMODE_19M (0x3 << 4)
#define RDA5807M_CLKMODE_24M (0x5 << 4)
#define RDA5807M_CLKMODE_26M (0x6 << 4)
#define RDA5807M_CLKMODE_38M (0x7 << 4)
#define RDA5807M_CHAN_MASK 0xFFC0
#define RDA5807M_CHAN_SHIFT 6
#define RDA5807M_BAND_MASK (0x000C)
#define RDA5807M_BAND_SHIFT 2
#define RDA5807M_BAND_WEST (0x0 << 2)
#define RDA5807M_BAND_JAPAN (0x1 << 2)
#define RDA5807M_BAND_WORLD (0x2 << 2)
#define RDA5807M_BAND_EAST (0x3 << 2)
#define RDA5807M_SPACE_MASK (0x0003)
#define RDA5807M_SPACE_100K 0x0
#define RDA5807M_SPACE_200K 0x1
#define RDA5807M_SPACE_50K 0x2
#define RDA5807M_SPACE_25K 0x3
#define RDA5807M_SEEKTH_MASK 0x7F00
#define RDA5807M_SEEKTH_SHIFT 8
#define RDA5807M_VOLUME_MASK (0x000F)
#define RDA5807M_VOLUME_SHIFT 0
#define RDA5807M_OPENMODE_MASK (0x6000)
#define RDA5807M_OPENMODE_WRITE (0x3 << 13)
#define RDA5807M_SOFTBLENDTH_MASK 0x7C00
#define RDA5807M_SOFTBLENDTH_SHIFT 10
#define RDA5807M_SEEKTHOLD_MASK (0x00FC)
#define RDA5807M_SEEKTHOLD_SHIFT 2
#define RDA5807M_SEEKMODE_MASK 0x7000
#define RDA5807M_SEEKMODE_OLD (0x1 << 12)
#define RDA5807M_READCHAN_MASK 0x03FF
#define RDA5807M_RSSI_MASK 0xFE00
#define RDA5807M_RSSI_SHIFT 9
#define RDA5807M_BLERA_MASK (0x000C)
#define RDA5807M_BLERA_SHIFT 2
#define RDA5807M_BLERA_0 (0x0 << 2)
#define RDA5807M_BLERA_12 (0x1 << 2)
#define RDA5807M_BLERA_35 (0x2 << 2)
#define RDA5807M_BLERA_U (RDA5807M_BLERA_12 | RDA5807M_BLERA_35)
#define RDA5807M_BLERB_MASK (0x0003)
#define RDA5807M_BLERB_SHIFT 0
#define RDA5807M_BLERB_0 0x0
#define RDA5807M_BLERB_12 0x1
#define RDA5807M_BLERB_35 0x2
#define RDA5807M_BLERB_U (RDA5807M_BLERB_12 | RDA5807M_BLERB_35)

constexpr uint16_t band_lower_limits[]  PROGMEM = { 8700, 7600, 7600, 6500, 5000 };
constexpr uint16_t band_higher_limits[] PROGMEM = { 10800, 9100, 10800, 7600, 6500 };
constexpr uint8_t channel_spacings[]    PROGMEM = { 100, 200, 50, 25 };

void init(uint16_t band = RDA5807M_BAND_WEST);
void shutdown();
void load();
void save();
bool volume_up();
bool volume_down(bool also_mute = true);
void seek_up(bool wrap = true);
void seek_down(bool wrap = true);
void mute();
void unmute(bool min_volume = true);
uint16_t get_band_spacing();
uint16_t get_freq();
bool set_freq(uint16_t freq);
uint8_t get_rssi();
uint16_t channel(uint8_t index, bool set = true);
void channel(uint8_t index, uint16_t freq, bool remove = false);
void show_info(bool update = false);
}

#endif