#ifndef NEC_H_
#define NEC_H_

#include "bool.h"
#include <avr/io.h>

#ifndef VOLTAGE
#define VOLTAGE 5000
#endif

#if VOLTAGE == 5000
    #define TOLERANCE 250
    #define AGC_TYP 9000
    #define CMD_SPACE_TYP 4500
    #define REPEAT_SPACE_TYP 2250
    #define LONG_BURST_TYP 1680
    #define SHORT_BURST_TYP 560
#elif VOLTAGE == 3300
    #define TOLERANCE 231
    #define AGC_TYP 8325
    #define CMD_SPACE_TYP 4163
    #define REPEAT_SPACE_TYP 2081
    #define LONG_BURST_TYP 1554
    #define SHORT_BURST_TYP 518
#endif

#define AGC_MIN (AGC_TYP - TOLERANCE)
#define AGC_MAX (AGC_TYP + TOLERANCE)

#define CMD_SPACE_MIN (CMD_SPACE_TYP - TOLERANCE)
#define CMD_SPACE_MAX (CMD_SPACE_TYP + TOLERANCE)

#define REPEAT_SPACE_MIN (REPEAT_SPACE_TYP - TOLERANCE)
#define REPEAT_SPACE_MAX (REPEAT_SPACE_TYP + TOLERANCE)

#define LONG_BURST_MIN (LONG_BURST_TYP - TOLERANCE)
#define LONG_BURST_MAX (LONG_BURST_TYP + TOLERANCE)

#define SHORT_BURST_MIN (SHORT_BURST_TYP - TOLERANCE)
#define SHORT_BURST_MAX (SHORT_BURST_TYP + TOLERANCE)

#define DDR(x) (*(&(x) - 1))
#define PIN(x) (*(&(x) - 2))

typedef enum {
    IDLE,
    AGC,
    SPACE,
    DATA,
    END
} state_t;

typedef enum {
    ADDRESS,
    COMMAND
} data_t;

typedef enum {
    IDLE_BURST,
    AGC_BURST,      // 9000
    CMD_SPACE,      // 4500
    REPEAT_SPACE,   // 2250
    LONG_BURST,     // 1680
    SHORT_BURST,    // 560
    UNKNOWN
} event_t;

void nec_init(void);
void nec_reset(void);
uint8_t nec_is_ready(void);
uint8_t nec_get_command(void);
uint8_t nec_get_address(void);
uint8_t nec_get_last_error(void);
bool nec_is_repeat(void);

#endif
