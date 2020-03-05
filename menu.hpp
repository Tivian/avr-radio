#ifndef MENU_HPP
#define MENU_HPP

#include "nec.hpp"

namespace Menu {
bool busy();
bool showing_radio();
void set(bool to_radio = false);
void show();
void show(const NEC::data_t& data);
void reset();
}

#endif