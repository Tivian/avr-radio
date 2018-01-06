#include "radio.h"
#include "i2c_master.h"

static RADIO_BAND _band;
static RADIO_FREQ _freq;

static RADIO_FREQ _freqLow;
static RADIO_FREQ _freqHigh;
static RADIO_FREQ _freqSteps;

static uint16_t registers[16];

static void _read_registers();
static void _save_registers();
static void _save_register(uint8_t reg_nr);
static void _write16(uint16_t val);
static uint16_t _read16(bool last);

uint16_t radio_init(bool defaults, RADIO_FREQ default_freq) {
    uint16_t ret;
    i2c_init(RADIO_TWI_TWBR, RADIO_TWI_TWPS);

    if (ret = i2c_start(RADIO_SLA_SQE, I2C_WRITE))
        return (RADIO_SLA_SQE << 8) | ret;
    else
        i2c_stop();

    registers[RADIO_REG_CHIPID] = 0x5804; // 00 id
    registers[1] = 0x0000; // 01 not used
    registers[RADIO_REG_CTRL] = (RADIO_REG_CTRL_RESET | RADIO_REG_CTRL_ENABLE);
    radio_set_band(RADIO_BAND_FM);
    registers[RADIO_REG_R4] = RADIO_REG_R4_EM50; //  0x1800;  // 04 DE ? SOFTMUTE
    registers[RADIO_REG_VOL] = 0x9781;//0x9081; // 0x81D1;  // 0x82D1 / INT_MODE, SEEKTH=0110,????, Volume=1
    registers[6] = 0x0000;
    registers[7] = 0x0000;
    registers[8] = 0x0000;
    registers[9] = 0x0000;

    // reset the chip
    _save_registers();

    registers[RADIO_REG_CTRL] = RADIO_REG_CTRL_ENABLE;
    _save_register(RADIO_REG_CTRL);

    if (defaults) {
        radio_set_frequency(default_freq);
        radio_set_volume(RADIO_DEFAULT_VOLUME);
        radio_set_mono(false);
        radio_set_mute(false);
    }
    
    return 0;
}

void radio_show_info(void) {
    return;
}

// switch the power off
void radio_term() {
    radio_set_volume(0);
    registers[RADIO_REG_CTRL] = 0x0000; // all bits off
    _save_registers();
}

void radio_set_volume(uint8_t new_volume) {
    new_volume &= RADIO_REG_VOL_VOL;
    registers[RADIO_REG_VOL] &= (~RADIO_REG_VOL_VOL);
    registers[RADIO_REG_VOL] |= new_volume;
    _save_register(RADIO_REG_VOL);
}

void radio_set_bass_boost(bool switch_on) {
    if (switch_on)
        registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_BASS;
    else
        registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_BASS);

    _save_register(RADIO_REG_CTRL);
}

void radio_set_mono(bool switch_on) {
    registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_SEEK);
    if (switch_on)
        registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_MONO;
    else
        registers[RADIO_REG_CTRL] &= ~RADIO_REG_CTRL_MONO;

    _save_register(RADIO_REG_CTRL);
}

void radio_set_mute(bool switch_on) {
    if (switch_on) // now don't unmute
        registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_UNMUTE);
    else // now unmute
        registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_UNMUTE;
    
    _save_register(RADIO_REG_CTRL);
}

void radio_set_soft_mute(bool switch_on) {
    if (switch_on)
        registers[RADIO_REG_R4] |= (RADIO_REG_R4_SOFTMUTE);
    else
        registers[RADIO_REG_R4] &= (~RADIO_REG_R4_SOFTMUTE);

    _save_register(RADIO_REG_R4);
}

void radio_set_band(RADIO_BAND newBand) {
    uint16_t r = 0x00;

    if (newBand == RADIO_BAND_FM) {
        _freqLow = 8700;
        _freqHigh = 10800;
        _freqSteps = RADIO_FREQ_STEPS;
        r = RADIO_REG_CHAN_BAND_FM;
    } else if (newBand == RADIO_BAND_FMWORLD) {
        _freqLow = 7600;
        _freqHigh = 10800;
        _freqSteps = RADIO_FREQ_STEPS;
        r = RADIO_REG_CHAN_BAND_FMWORLD;
    }

    registers[RADIO_REG_CHAN] = (r | RADIO_REG_CHAN_SPACE_100);
}

// retrieve the real frequency from the chip after automatic tuning.
RADIO_FREQ radio_get_frequency() {
    // check register A
    i2c_start(RADIO_SLA_SQE, I2C_READ);
    registers[RADIO_REG_RA] = _read16(true);
    i2c_stop();

    uint16_t ch = registers[RADIO_REG_RA] & RADIO_REG_RA_NR;
    _freq = _freqLow + (ch * RADIO_FREQ_STEPS);  // assume 100 kHz spacing

    return _freq;
}

void radio_set_frequency(RADIO_FREQ new_freq) {
    uint16_t newChannel, regChannel = registers[RADIO_REG_CHAN] & (RADIO_REG_CHAN_SPACE | RADIO_REG_CHAN_BAND);

    if (new_freq < _freqLow) new_freq = _freqLow;
    if (new_freq > _freqHigh) new_freq = _freqHigh;
    newChannel = (new_freq - _freqLow) / RADIO_FREQ_STEPS;

    regChannel += RADIO_REG_CHAN_TUNE; // enable tuning
    regChannel |= newChannel << 6;

    // enable output and unmute
    registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_OUTPUT | RADIO_REG_CTRL_UNMUTE | RADIO_REG_CTRL_RDS | RADIO_REG_CTRL_ENABLE; //  | RADIO_REG_CTRL_NEW
    _save_register(RADIO_REG_CTRL);

    registers[RADIO_REG_CHAN] = regChannel;
    _save_register(RADIO_REG_CHAN);

    // adjust Volume
    _save_register(RADIO_REG_VOL);
}

void radio_set_band_frequency(RADIO_BAND new_band, RADIO_FREQ new_freq) {
    radio_set_band(new_band);
    radio_set_frequency(new_freq);
}

// start seek mode upwards
void radio_seek_up(bool to_next_sender) {
    // start seek mode
    registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_SEEKUP;
    registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_SEEK;
    _save_register(RADIO_REG_CTRL);

    if (!to_next_sender) { // stop scanning right now
        registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_SEEK);
        _save_register(RADIO_REG_CTRL);
    }
}

// start seek mode downwards
void radio_seek_down(bool to_next_sender) {
    registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_SEEKUP);
    registers[RADIO_REG_CTRL] |= RADIO_REG_CTRL_SEEK;
    _save_register(RADIO_REG_CTRL);

    if (!to_next_sender) { // stop scanning right now
        registers[RADIO_REG_CTRL] &= (~RADIO_REG_CTRL_SEEK);
        _save_register(RADIO_REG_CTRL);
    }
}

// Retrieve all the information related to the current radio receiving situation.
void radio_get_info(RADIO_INFO *info) {
    // read data from registers A .. F of the chip into class memory
    _read_registers();
    uint16_t channel = registers[RADIO_REG_RA] & RADIO_REG_RA_NR;
    _freq = _freqLow + (channel * RADIO_FREQ_STEPS);  // assume 100 kHz spacing

    info->freq = _freq;
    info->rssi = registers[RADIO_REG_RB] >> 10;
    if (registers[RADIO_REG_RA]   & RADIO_REG_RA_STEREO) info->stereo = true;
    if (registers[RADIO_REG_RA]   & RADIO_REG_RA_RDS)    info->rds    = true;
    if (registers[RADIO_REG_RB]   & RADIO_REG_RB_FMTRUE) info->tuned  = true;
    if (registers[RADIO_REG_CTRL] & RADIO_REG_CTRL_MONO) info->mono   = true;
}

bool radio_get_rds(RADIO_RDS *rds) {
    uint16_t newData;
    bool result = false;

    // check register A
    i2c_start(RADIO_SLA_SQE, I2C_READ);
    registers[RADIO_REG_RA] = _read16(true);
    i2c_stop();

    //rds->block_e_present = registers[RADIO_REG_RA] & RADIO_REG_RA_RDSBLOCK;
    if (registers[RADIO_REG_RA] & RADIO_REG_RA_RDS) {
        // check for new RDS data available
        i2c_start(RADIO_SLA_RAN, I2C_WRITE); // Device 0x11 for random access
        i2c_write(RADIO_REG_RDSA);           // Start at Register 0x0C

        i2c_start(RADIO_SLA_RAN, I2C_READ);  // Retransmit device address with READ, followed by 8 bytes
        newData = _read16(false);
        if (newData != registers[RADIO_REG_RDSA]) {
            registers[RADIO_REG_RDSA] = rds->block_a = newData;
            rds->pic.country_code = (registers[RADIO_REG_RDSA] & RDS_COUNTRY_CODE_MASK) >> RDS_COUNTRY_CODE_SHIFT;
            rds->pic.program_area = (registers[RADIO_REG_RDSA] & RDS_PROGRAM_AREA_MASK) >> RDS_PROGRAM_AREA_SHIFT;
            rds->pic.program_ref  = (registers[RADIO_REG_RDSA] & RDS_PROGRAM_REF_MASK)  >> RDS_PROGRAM_REF_SHIFT;
            result = true;
        }

        newData = _read16(false);
        if (newData != registers[RADIO_REG_RDSB]) {
            registers[RADIO_REG_RDSB] = rds->block_b = newData;
            rds->group_type      = (registers[RADIO_REG_RDSB] & RDS_GROUP_TYPE_MASK) >> RDS_GROUP_TYPE_SHIFT;
            rds->group_version   = (registers[RADIO_REG_RDSB] & RDS_GROUP_VER_MASK)  >> RDS_GROUP_VER_SHIFT;
            rds->traffic_program = (registers[RADIO_REG_RDSB] & RDS_TRAF_PROG_MASK)  >> RDS_TRAF_PROG_SHIFT;
            rds->program_type    = (registers[RADIO_REG_RDSB] & RDS_PROG_TYPE_MASK)  >> RDS_PROG_TYPE_SHIFT;
            rds->block_b_add     = (registers[RADIO_REG_RDSB] & RDS_BLOCK_B_UNKNOWN) >> RDS_BLOCK_B_SHIFT;
            result = true;
        }

        newData = _read16(false);
        if (newData != registers[RADIO_REG_RDSC]) {
            registers[RADIO_REG_RDSC] = rds->block_c = newData;
            result = true;
        }

        newData = _read16(true);
        if (newData != registers[RADIO_REG_RDSD]) {
            registers[RADIO_REG_RDSD] = rds->block_d = newData;
            result = true;
        }

        i2c_stop();
    }

    return result;
}

// Load all status registers from to the chip
// registers 0A through 0F
// using the sequential read access mode.
static void _read_registers() {
    i2c_start(RADIO_SLA_SQE, I2C_READ);
    for (uint8_t i = 0; i < 6; i++)
        registers[0xA + i] = _read16(i == 5);
    i2c_stop();
}

// Save writable registers back to the chip
// The registers 02 through 06, containing the configuration
// using the sequential write access mode.
static void _save_registers() {
    i2c_start(RADIO_SLA_SQE, I2C_WRITE);
    for (uint8_t i = 2; i <= 6; i++)
        _write16(registers[i]);
    i2c_stop();
}

// Save one register back to the chip
static void _save_register(uint8_t reg_nr) {
    i2c_start(RADIO_SLA_RAN, I2C_WRITE);
    i2c_write(reg_nr);
    _write16(registers[reg_nr]);
    i2c_stop();
}

// write a register value using 2 bytes into the Wire.
static void _write16(uint16_t val) {
    i2c_write(val >> 8);
    i2c_write(val & 0xFF);
}

// read a register value using 2 bytes in a row
static uint16_t _read16(bool last) {
    uint8_t byte = i2c_read_ack();
    uint16_t result = byte << 8;
    byte = last ? i2c_read_nack() : i2c_read_ack();
    result |= byte;

    return result;
}
