#include "eeprom.hpp"
#include "i2c_master.hpp"
#include <stddef.h>
#include <util/delay.h>

namespace {
template <typename T = void>
void write_address(T address);

template<>
inline void write_address<uint8_t>(uint8_t address) {
    I2C::write(address);
}

template<>
inline void write_address<uint16_t>(uint16_t address) {
    I2C::write(address >> 8);
    I2C::write(address & 0xf);
}
}

template <typename T>
bool Eprom<T>::read_block(const void* address, void* dest, uint8_t size) {
    uint8_t* ptr = static_cast<uint8_t*>(dest);

    if (I2C::start(dev_addr, I2C::mode_t::write))
        return false;

    write_address<T>(static_cast<T>(reinterpret_cast<size_t>(address)));
    if (I2C::start(dev_addr, I2C::mode_t::read))
        return false;

    for (uint8_t i = 0; i < size - 1; i++)
        ptr[i] = I2C::read_ack();
    ptr[size - 1] = I2C::read_nack();
    I2C::stop();

    return true;
}

template <typename T>
uint8_t Eprom<T>::read(const void* address) {
    uint8_t out;
    read_block(address, &out, 1);
    return out;
}

template <typename T>
bool Eeprom<T>::write_block(const void* address, void* src, uint8_t size) {
    uint8_t* ptr = static_cast<uint8_t*>(src);

    if (I2C::start(dev_addr, I2C::mode_t::write))
        return false;

    write_address<T>(static_cast<T>(reinterpret_cast<size_t>(address)));
    for (uint8_t i = 0; i < size; i++)
        I2C::write(ptr[i]);
    I2C::stop();
    _delay_ms(WRITE_CYCLE_TIME);

    return true;
}

template <typename T>
inline bool Eeprom<T>::write(const void* address, uint8_t value) {
    return write_block(address, &value, 1);
}

template <typename T>
bool Eeprom<T>::update_block(const void* address, void* src, uint8_t size) {
    uint8_t* ptr = static_cast<uint8_t*>(src);
    for (uint8_t i = 0; i < size; i++) {
        auto current_addr = reinterpret_cast<const void*>(reinterpret_cast<size_t>(address) + i);
        if (Eprom<T>::read(current_addr) != ptr[i]) {
            if (write(current_addr, ptr[i]))
                return false;
        }
    }

    return true;
}

template <typename T>
inline bool Eeprom<T>::update(const void* address, uint8_t value) {
    return update_block(address, &value, 1);
}