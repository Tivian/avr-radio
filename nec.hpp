#ifndef NEC_HPP
#define NEC_HPP

#include <stdint.h>

namespace NEC {
enum class state_t : uint8_t { idle, agc, space, data, end };
enum class event_t : uint8_t { idle, agc_space, cmd_space, repeat, logic_one, logic_zero };
enum class mode_t : uint8_t { address, command };

struct data_t {
    bool repeat;
    uint8_t addr;
    uint8_t inv_addr;
    uint8_t cmd;
    uint8_t inv_cmd;
};

inline constexpr uint16_t TOLERANCE = 25;
inline constexpr uint16_t AGC_TYP = 9000;
inline constexpr uint16_t CMD_SPACE_TYP = 4500;
inline constexpr uint16_t REPEAT_SPACE_TYP = 2250;
inline constexpr uint16_t LOGIC_ONE_TYP = 1680;
inline constexpr uint16_t LOGIC_ZERO_TYP = 560;

inline constexpr uint16_t AGC_MIN = AGC_TYP - 80 * TOLERANCE;
inline constexpr uint16_t AGC_MAX = AGC_TYP + 40 * TOLERANCE;

inline constexpr uint16_t CMD_SPACE_MIN = CMD_SPACE_TYP - 40 * TOLERANCE;
inline constexpr uint16_t CMD_SPACE_MAX = CMD_SPACE_TYP + 40 * TOLERANCE;

inline constexpr uint16_t REPEAT_SPACE_MIN = REPEAT_SPACE_TYP - 11 * TOLERANCE;
inline constexpr uint16_t REPEAT_SPACE_MAX = REPEAT_SPACE_TYP + 30 * TOLERANCE;

inline constexpr uint16_t LOGIC_ONE_MIN = LOGIC_ONE_TYP - 20 * TOLERANCE;
inline constexpr uint16_t LOGIC_ONE_MAX = LOGIC_ONE_TYP + 10 * TOLERANCE;

inline constexpr uint16_t LOGIC_ZERO_MIN = LOGIC_ZERO_TYP - 10 * TOLERANCE;
inline constexpr uint16_t LOGIC_ZERO_MAX = LOGIC_ZERO_TYP + 20 * TOLERANCE;

extern void (* volatile indicate)(void);

constexpr data_t empty { false, 0xFF, 0xFF, 0xFF, 0xFF };

void init();
void reset();
bool ready();
data_t& data();
}

#endif