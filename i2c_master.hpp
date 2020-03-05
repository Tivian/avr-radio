#ifndef I2C_MASTER_HPP
#define I2C_MASTER_HPP

#include <avr/io.h>
#include <util/twi.h>

namespace I2C {
enum class mode_t : uint8_t { write = 0, read = 1 };
inline constexpr uint8_t STATUS_MASK = 0xF8;

void init(uint8_t twbr = 2, uint8_t twsr = 0);
uint8_t start(uint8_t address, mode_t mode);
uint8_t write(uint8_t data);
uint8_t read_ack();
uint8_t read_nack();
uint8_t status();
void wait();
void stop();
}

#endif