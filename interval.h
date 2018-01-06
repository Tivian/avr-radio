#ifndef INTERVAL_H_
#define INTERVAL_H_

#include "bool.h"

typedef void(*CALLBACK_FX)(void);

bool is_interval_set(void);
void set_interval(uint16_t time, CALLBACK_FX fx);
void clear_interval(void);

#endif
