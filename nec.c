#include "nec.h"
#include <avr/interrupt.h>

static volatile struct {
    state_t state;
    bool repeat;
    bool ready;
    uint8_t address;
    uint8_t command;
    uint8_t old_address;
    uint8_t old_command;
    uint8_t error;
} global;

static event_t nec_get_event(uint16_t delay) {
    if (global.state == IDLE) {
        return IDLE_BURST;
    } else if (delay >= AGC_MIN && delay <= AGC_MAX) {
        return AGC_BURST;
    } else if (delay >= CMD_SPACE_MIN && delay <= CMD_SPACE_MAX) {
        return CMD_SPACE;
    } else if (delay >= REPEAT_SPACE_MIN && delay <= REPEAT_SPACE_MAX) {
        return REPEAT_SPACE;
    } else if (delay >= LONG_BURST_MIN && delay <= LONG_BURST_MAX) {
        return LONG_BURST;
    } else if (delay >= SHORT_BURST_MIN && delay <= SHORT_BURST_MAX) {
        return SHORT_BURST;
    } else {
        return UNKNOWN;
    }
}

static void nec_error(event_t event) {
    global.error = (global.state << 4) | event;
    nec_reset();
}

void nec_init(void) {
    global.address = 0;
    global.command = 0;
    nec_reset();

    DDRD &= ~_BV(PD2);
    MCUCR |= _BV(ISC00);
    GICR |= _BV(INT0);
    TCNT1 = 0;
    TCCR1B = _BV(CS11);
}

void nec_reset(void) {
    global.state = IDLE;
    global.repeat = false;
    global.ready = false;
    global.old_command = global.command;
    global.old_address = global.address;
    global.command = 0;
    global.address = 0;
    global.error = 0;
}

uint8_t nec_is_ready(void) {
    return global.ready;
}

uint8_t nec_get_command(void) {
    return global.command;
}

uint8_t nec_get_address(void) {
    return global.address;
}

uint8_t nec_get_last_error(void) {
    uint8_t _error = global.error;
    global.error = 0;
    return _error;
}

bool nec_is_repeat(void) {
    return global.repeat;
}

ISR(INT0_vect) {
    if (global.ready) {
        return;
    }

    static volatile uint8_t *p;
    static uint8_t bits, temp = 0, inverted = 0;
    static data_t data_type = ADDRESS;
    event_t event = nec_get_event(TCNT1);
    TCNT1 = 0;

    if (global.state == IDLE && event == IDLE_BURST) {
        global.state = AGC;
    } else if (global.state == AGC && event == AGC_BURST) {
        global.state = SPACE;
    } else if (global.state == SPACE && event == CMD_SPACE) {
        global.state = DATA;
        data_type = ADDRESS;
        inverted = 0;
        bits = 0;
        temp = 0;
        p = &global.address;
    } else if (global.state == SPACE && event == REPEAT_SPACE) {
        global.state = END;
        global.address = global.old_address;
        global.command = global.old_command;
        if (event == REPEAT_SPACE)
            global.repeat = true;
    } else if (global.state == DATA) {
        bits++;

        if (bits >= 16) {
            if (!inverted) {
                inverted = 1;
                *p = temp;
            } else if ((*p & temp) == 0x00) {
                inverted = 0;
                if (data_type == ADDRESS) {
                    data_type = COMMAND;
                    p = &global.command;
                } else {
                    global.state = END;
                }
            } else {
                nec_error(event);
            }

            bits = 0;
            temp = 0;
        } else if (bits % 2 == 0) {
            temp <<= 1;
            if (event == LONG_BURST) {
                temp |= 0x01;
            } else if (event != SHORT_BURST) {
                nec_error(event);
            }
        }
    } else if (global.state == END && event == SHORT_BURST) {
        global.ready = true;
    } else {
        nec_error(event);
    }
}

/*
-> IDLE
    1 54267 [>25000]

-> AGC_BURST
    1 9047 [9000]

-> CMD_SPACE
    1 4408 [4500]
OR
-> REPEAT_SPACE
    1 2196 [2250]

-> ADDRESS (00000000)
    1 648 [560]
    1 475 [560]
        0
    1 642
    1 481
        0
    1 645
    1 477
        0
    1 649
    1 473
        0
    1 643
    1 479
        0
    1 647
    1 477
        0
    1 650
    1 473
        0
    1 643
    1 479
        0

-> INV_ADDRESS (11111111)
    1 647
    1 1575 [1680]
        1
    1 641
    1 1582
        1
    1 645
    1 1580
        1
    1 648
    1 1576
        1
    1 650
    1 1574
        1
    1 642
    1 1582
        1
    1 644
    1 1579
        1
    1 647
    1 1577
        1

-> COMMAND (10101000)
    1 650
    1 1574
        1
    1 643
    1 480
        0
    1 646
    1 1577
        1
    1 649
    1 474
        0
    1 642
    1 1581
        1
    1 645
    1 479
        0
    1 648
    1 475
        0
    1 641
    1 481
        0

-> INV_COMMAND (01010111)
    1 645
    1 478
        0
    1 649
    1 1574
        1
    1 642
    1 481
        0
    1 646
    1 1576
        1
    1 640
    1 482
        0
    1 644
    1 1579
        1
    1 647
    1 1577
        1
    1 640
    1 1584
        1

-> END_BURST
    1 642 [560]
*/
