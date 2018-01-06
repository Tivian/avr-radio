#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include <avr/sfr_defs.h>
#include <util/twi.h>
#include "bool.h"

#define I2C_STATUS_MASK 0xF8

#define i2c_start_condi() (TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN))
#define i2c_repeated_start_condi() (twi_start())
#define i2c_stop_condi() (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO))
#define i2c_ack_condi() (TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA))
#define i2c_nack_condi() (TWCR = _BV(TWINT) | _BV(TWEN))

#define i2c_status() ((TWSR & I2C_STATUS_MASK))
#define i2c_stop() (i2c_stop_condi())
#define i2c_set_data(data) (TWDR = (data))
#define i2c_get_data() (TWDR)
#define i2c_send_data() (TWCR = _BV(TWINT) | _BV(TWEN))
#define i2c_reset() (TWCR = 0)

typedef enum {
    I2C_WRITE,
    I2C_READ
} I2C_MODE;

bool i2c_init(uint8_t twbr, uint8_t twsr);
uint8_t i2c_start(uint8_t address, I2C_MODE mode);
uint8_t i2c_write(uint8_t data);
uint8_t i2c_read_ack(void);
uint8_t i2c_read_nack(void);
void i2c_wait(void);

#endif
