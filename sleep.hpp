#ifndef SLEEP_HPP
#define SLEEP_HPP

#include <stdint.h>
#include <avr/sleep.h>

namespace Sleep {
enum class mode_t : uint8_t {
    idle = SLEEP_MODE_IDLE,
    power_down = SLEEP_MODE_PWR_DOWN,
    power_save = SLEEP_MODE_PWR_SAVE,
    standby = SLEEP_MODE_STANDBY
};

extern volatile mode_t mode;
extern void (* volatile on_asleep)(void);
extern void (* volatile on_wakeup)(void);
extern bool (* volatile keep_sleeping)(void);

void init();
void activate();
bool is_sleeping();
bool awaken_by_button();
}

#endif