#ifndef EEPROM_HPP
#define EEPROM_HPP

#include "utility.hpp"
#include <stdint.h>

template <typename T>
class Eprom {
public:
    Eprom(uint8_t dev_addr) : dev_addr(dev_addr) {}
    bool read_block(const void* address, void* dest, uint8_t size);
    uint8_t read(const void* address);
protected:
    uint8_t dev_addr;
};

template <typename T>
class Eeprom : public Eprom<T> {
public:
    static inline constexpr uint8_t WRITE_CYCLE_TIME = 25; //ms

    using Eprom<T>::Eprom;
    bool write_block(const void* address, void* src, uint8_t size);
    bool write(const void* address, uint8_t value);
    bool update_block(const void* address, void* src, uint8_t size);
    bool update(const void* address, uint8_t value);
private:
    using Eprom<T>::dev_addr;
};

#include "eeprom.tpp"

#endif