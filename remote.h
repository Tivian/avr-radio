#ifndef REMOTE_H_
#define REMOTE_H_
/**
 *  51      71
 *  11  01  61
 *  70  54  48
 *  34  4c  58
 *  18  0c  3d
 *  08  1c  2d
 *  21  25  29
 */

#define REMOTE_POWER    0x51
#define REMOTE_MENU     0x71
#define REMOTE_TEST     0x11
#define REMOTE_PLUS     0x01
#define REMOTE_RETURN   0x61
#define REMOTE_BACKWARD 0x70
#define REMOTE_PLAY     0x54
#define REMOTE_FORWARD  0x48
#define REMOTE_ZERO     0x34
#define REMOTE_MINUS    0x4C
#define REMOTE_CLEAR    0x58
#define REMOTE_ONE      0x18
#define REMOTE_TWO      0x0C
#define REMOTE_THREE    0x3D
#define REMOTE_FOUR     0x08
#define REMOTE_FIVE     0x1C
#define REMOTE_SIX      0x2D
#define REMOTE_SEVEN    0x21
#define REMOTE_EIGHT    0x25
#define REMOTE_NINE     0x29
#define REMOTE_BLANK    0x00

#define remote_is_digit(cmd)\
    ((cmd) == REMOTE_ZERO  || (cmd) == REMOTE_ONE    || (cmd) == REMOTE_TWO   ||\
     (cmd) == REMOTE_THREE || (cmd) == REMOTE_FOUR   || (cmd) == REMOTE_FIVE  ||\
     (cmd) == REMOTE_SIX   || (cmd) == REMOTE_SEVEN  || (cmd) == REMOTE_EIGHT ||\
     (cmd) == REMOTE_NINE)

#define remote_get_digit(cmd)\
    ((cmd) == REMOTE_NINE  ? 9 : (cmd) == REMOTE_EIGHT ? 8 : (cmd) == REMOTE_SEVEN ? 7 :\
     (cmd) == REMOTE_SIX   ? 6 : (cmd) == REMOTE_FIVE  ? 5 : (cmd) == REMOTE_FOUR  ? 4 :\
     (cmd) == REMOTE_THREE ? 3 : (cmd) == REMOTE_TWO   ? 2 : (cmd) == REMOTE_ONE   ? 1 :\
     (cmd) == REMOTE_ZERO  ? 0 : 0)

#endif
