#ifndef SCROLL_H_
#define SCROLL_H_

#include <avr/pgmspace.h>

void scroll_text(const char *s, uint16_t speed);
void scroll_text_p(PGM_P progmem_s, uint16_t speed);
void stop_scroll(void);

#endif
