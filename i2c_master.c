#include "i2c_master.h"
#include <avr/io.h>

bool i2c_init(uint8_t twbr, uint8_t twsr) {
    if (TWBR != 0)
        return false;

    TWBR = twbr;
    TWSR |= twsr;

    return true;
}

uint8_t i2c_start(uint8_t address, I2C_MODE mode) {
    if (TWBR == 0)
        return 1;

    i2c_reset();
    i2c_start_condi();
    i2c_wait();
    
    uint8_t ret = i2c_status();
    if ((ret != TW_START) && (ret != TW_REP_START))
        return ret;

	ret = i2c_write((address << 1) | mode);
    return ((ret != TW_MT_SLA_ACK) && (ret != TW_MR_SLA_ACK)) ? ret : 0;
}

uint8_t i2c_write(uint8_t data) {
    if (TWBR == 0)
        return 1;

    i2c_set_data(data);
    i2c_send_data();
    i2c_wait();

    uint8_t ret = i2c_status();
    return ((ret != TW_MT_DATA_ACK) && (ret != TW_MR_DATA_ACK)) ? ret : 0;
}

uint8_t i2c_read_ack(void) {
    if (TWBR == 0)
        return 1;

    i2c_ack_condi();
    i2c_wait();

    return i2c_get_data();
}

uint8_t i2c_read_nack(void) {
    if (TWBR == 0)
        return 1;

    i2c_nack_condi();
    i2c_wait();

    return i2c_get_data();
}

void i2c_wait(void) {
    uint16_t timeout = 0;
    while (!(TWCR & _BV(TWINT)) && (timeout < (F_CPU / 5000))) {
        timeout++;
    }
}
