#ifndef TIMEOUT_H_
#define TIMEOUT_H_

#include "bool.h"

typedef void(*CALLBACK_FX)(void);

bool is_timeout_set(void);
void set_timeout(uint8_t time, CALLBACK_FX fx);
void clear_timeout(void);

#endif
