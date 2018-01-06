#ifndef RADIO_H_
#define RADIO_H_
/*
Connector:

 1 2 3 4 5
 x x 6 7 8
 _____________________
| 1 - ANT   | 5 - GND |
| 2 - RCK   | 6 - SDA |
| 3 - OUT L | 7 - SCL |
| 4 - OUT R | 8 - VCC |
|___________|_________|
*/

#include "bool.h"
#include "rds.h"

#define RADIO_TWI_TWBR 33
#define RADIO_TWI_TWPS 0

#define RADIO_FREQ_STEPS 10

#define RADIO_REG_CHIPID 0x00
#define RADIO_CHIPID 0x5804

#define RADIO_REG_CTRL      0x02
#define RADIO_REG_CTRL_OUTPUT 0x8000
#define RADIO_REG_CTRL_UNMUTE 0x4000
#define RADIO_REG_CTRL_MONO   0x2000
#define RADIO_REG_CTRL_BASS   0x1000
#define RADIO_REG_CTRL_SEEKUP 0x0200
#define RADIO_REG_CTRL_SEEK   0x0100
#define RADIO_REG_CTRL_RDS    0x0008
#define RADIO_REG_CTRL_NEW    0x0004
#define RADIO_REG_CTRL_RESET  0x0002
#define RADIO_REG_CTRL_ENABLE 0x0001

#define RADIO_REG_CHAN            0x03
#define RADIO_REG_CHAN_SPACE        0x0003
#define RADIO_REG_CHAN_SPACE_100    0x0000
#define RADIO_REG_CHAN_BAND         0x000C
#define RADIO_REG_CHAN_BAND_FM      0x0000
#define RADIO_REG_CHAN_BAND_FMWORLD 0x0008
#define RADIO_REG_CHAN_TUNE         0x0010
#define RADIO_REG_CHAN_NR           0x7FC0

#define RADIO_REG_R4        0x04
#define RADIO_REG_R4_EM50     0x0800
#define RADIO_REG_R4_SOFTMUTE 0x0200
#define RADIO_REG_R4_AFC      0x0100

#define RADIO_REG_VOL   0x05
#define RADIO_REG_VOL_VOL 0x000F

#define RADIO_REG_RA        0x0A
#define RADIO_REG_RA_RDS      0x8000
#define RADIO_REG_RA_RDSBLOCK 0x0800
#define RADIO_REG_RA_STEREO   0x0400
#define RADIO_REG_RA_NR       0x03FF

#define RADIO_REG_RB       0x0B
#define RADIO_REG_RB_FMTRUE  0x0100
#define RADIO_REG_RB_FMREADY 0x0080

#define RADIO_REG_RDSA 0x0C
#define RADIO_REG_RDSB 0x0D
#define RADIO_REG_RDSC 0x0E
#define RADIO_REG_RDSD 0x0F

#define RADIO_SLA_SQE 0x10
#define RADIO_SLA_RAN 0x11

#define RADIO_DEFAULT_VOLUME 4

typedef uint16_t RADIO_FREQ;

typedef enum {
    RADIO_BAND_NONE = 0,    // No band selected.
    RADIO_BAND_FM = 1,      // FM band 87.5 – 108 MHz (USA, Europe) selected.
    RADIO_BAND_FMWORLD = 2, // FM band 76 – 108 MHz (Japan, Worldwide) selected.
    RADIO_BAND_AM = 3,      // AM band selected.
    RADIO_BAND_KW = 4,      // KW band selected.
    RADIO_BAND_MAX = 4      // Maximal band enumeration value.
} RADIO_BAND;

typedef struct {
    RADIO_FREQ freq;
    uint8_t rssi;
    bool stereo;
    bool rds;
    bool tuned;
    bool mono;
} RADIO_INFO;

uint16_t radio_init(bool defaults, RADIO_FREQ default_freq);
void radio_term();
void radio_set_volume(uint8_t new_volume);
void radio_set_bass_boost(bool switch_on);
void radio_set_mono(bool switch_on);
void radio_set_mute(bool switch_on);
void radio_set_soft_mute(bool switch_on);
void radio_set_band(RADIO_BAND newBand);
RADIO_FREQ radio_get_frequency();
void radio_set_frequency(RADIO_FREQ new_freq);
void radio_set_band_frequency(RADIO_BAND new_band, RADIO_FREQ new_freq);
void radio_seek_up(bool to_next_sender);
void radio_seek_down(bool to_next_sender);
void radio_get_info(RADIO_INFO *info);
bool radio_get_rds(RADIO_RDS *rds);

#endif
