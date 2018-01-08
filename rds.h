#ifndef RDS_H_
#define RDS_H_

#define RDS_GROUP_TYPE_A 0
#define RDS_GROUP_TYPE_B 1

// Block A masks
#define RDS_COUNTRY_CODE_MASK 0xF000
#define RDS_PROGRAM_AREA_MASK 0x0F00
#define RDS_PROGRAM_REF_MASK  0x00FF

// Block A shifts
#define RDS_COUNTRY_CODE_SHIFT 12
#define RDS_PROGRAM_AREA_SHIFT 8
#define RDS_PROGRAM_REF_SHIFT  0

// Block B masks
#define RDS_GROUP_TYPE_MASK 0xF000
#define RDS_GROUP_VER_MASK  0x0800
#define RDS_TRAF_PROG_MASK  0x0400
#define RDS_PROG_TYPE_MASK  0x03E0

// Block B shifts
#define RDS_GROUP_TYPE_SHIFT 12
#define RDS_GROUP_VER_SHIFT  11
#define RDS_TRAF_PROG_SHIFT  10
#define RDS_PROG_TYPE_SHIFT  5

// Group 0 masks
#define RDS_GROUP_0_TA_MASK  0x0010
#define RDS_GROUP_0_MS_MASK  0x0008
#define RDS_GROUP_0_DI_MASK  0x0004
#define RDS_GROUP_0_IDX_MASK 0x0003

#define RDS_GROUP_0_ALT_FREQ_1_MASK 0xFF00
#define RDS_GROUP_0_ALT_FREQ_2_MASK 0x00FF

#define RDS_GROUP_0_CHAR_1_MASK 0xFF00
#define RDS_GROUP_0_CHAR_2_MASK 0xFF00

// Group 0 shifts
#define RDS_GROUP_0_TA_SHIFT  4
#define RDS_GROUP_0_MS_SHIFT  3
#define RDS_GROUP_0_DI_SHIFT  2
#define RDS_GROUP_0_IDX_SHIFT 0

#define RDS_GROUP_0_ALT_FREQ_1_SHIFT 8
#define RDS_GROUP_0_ALT_FREQ_2_SHIFT 0

#define RDS_GROUP_0_CHAR_1_SHIFT 8
#define RDS_GROUP_0_CHAR_2_SHIFT 0

typedef struct {
    uint16_t block_a;
    uint16_t block_b;
    uint16_t block_c;
    uint16_t block_d;
} RADIO_RDS_RAW;

typedef struct {
    uint8_t country_code;
    uint8_t program_area;
    uint8_t program_ref;
} RADIO_RDS_PIC;

typedef struct {
    uint8_t traffic_ann;
    uint8_t music_speech_switch;
    uint8_t alt_freq_1;
    uint8_t alt_freq_2;
    uint8_t DI;
    uint8_t index;
    char ch[2];
} RADIO_RDS_GROUP_0;

typedef struct {
    bool text_ab_flag;
    uint8_t index;
    char ch[4];
} RADIO_RDS_GROUP_2;

typedef union {
    RADIO_RDS_GROUP_0 basic_info;
    RADIO_RDS_GROUP_2 radio_text;
} RADIO_RDS_GROUP;

typedef struct {
    RADIO_RDS_PIC pic;
    uint8_t group_type;
    uint8_t group_version;
    uint8_t traffic_program;
    uint8_t program_type;
    RADIO_RDS_GROUP group;
    RADIO_RDS_RAW raw;
} RADIO_RDS;

#endif