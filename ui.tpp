#ifndef UI_TPP
#define UI_TPP

#include "ui.hpp"

namespace UI {
template <> inline void power_led   <true>() { POWER_LED_PORT  |=  _BV(POWER_LED_PIN);  }
template <> inline void power_led  <false>() { POWER_LED_PORT  &= ~_BV(POWER_LED_PIN);  }
template <> inline void action_led  <true>() { ACTION_LED_PORT |=  _BV(ACTION_LED_PIN); }
template <> inline void action_led <false>() { ACTION_LED_PORT &= ~_BV(ACTION_LED_PIN); }
template <> inline void global_mute <true>() { MUTE_PORT       &= ~_BV(MUTE_PIN);       }
template <> inline void global_mute<false>() { MUTE_PORT       |=  _BV(MUTE_PIN);       }
}

#endif