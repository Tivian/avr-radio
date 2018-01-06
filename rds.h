#ifndef RDS_H_
#define RDS_H_

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
#define RDS_BLOCK_B_UNKNOWN 0x001F

// Block B shifts
#define RDS_GROUP_TYPE_SHIFT 12
#define RDS_GROUP_VER_SHIFT  11
#define RDS_TRAF_PROG_SHIFT  10
#define RDS_PROG_TYPE_SHIFT  5
#define RDS_BLOCK_B_SHIFT    0

typedef struct {
    uint8_t country_code;
    uint8_t program_area;
    uint8_t program_ref;
} RADIO_PIC;

typedef struct {
    RADIO_PIC pic;
    uint8_t group_type;
    uint8_t group_version;
    uint8_t traffic_program;
    uint8_t program_type;
    uint8_t block_b_add;
    uint16_t block_a;
    uint16_t block_b;
    uint16_t block_c;
    uint16_t block_d;
} RADIO_RDS;

#endif