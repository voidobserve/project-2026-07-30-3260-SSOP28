#include "aip3368h_display.h"
#include "aip3368.h"

#include "instrument.h"

#include "user_config.h"

/**
 * @brief 7段数码管段码定义 (a,b,c,d,e,f,g)
 *        对应二进制位: bit0=a, bit1=b, bit2=c, bit3=d, bit4=e, bit5=f, bit6=g
 */
static const u8 digit_segment_code[10] = {
    0x3F, // 0: abcdef
    0x06, // 1: bc
    0x5B, // 2: abdeg
    0x4F, // 3: abcdg
    0x66, // 4: bcfg
    0x6D, // 5: acdfg
    0x7D, // 6: acdefg
    0x07, // 7: abc
    0x7F, // 8: abcdefg
    0x6F  // 9: abcdfg
};

/*
    指示灯和显存的映射关系
    发动机转速对应的格数，从低到高
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t engine_speed_gear_map[] = {
    // 发动机转速，第 0 格指示灯（从左往右数，从0开始，白）
    {7, 5},  {7, 6},  {7, 7},  {7, 8},  {7, 9},

    {7, 10}, {7, 13}, {7, 15}, {6, 3},  {6, 4},

    {6, 5},  {6, 6},  {6, 7},  {6, 11}, {6, 10},

    {6, 9},  {6, 8},  {4, 9},  {4, 10}, {4, 11},

    {4, 12}, {4, 13}, {4, 0},  {4, 1},  {4, 2},

    {4, 3},  {4, 4},
};

/*
    指示灯和显存的映射关系
    发动机转速的对应的刻度条，从低到高
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t engine_speed_scale_map[] = {
    // 发动机转速刻度，第 0 个指示灯，对应 0 刻度（从左往右数，从0开始，白）
    {7, 3},  {7, 4},  {7, 11}, {7, 12}, {7, 14},

    {6, 2},  {6, 1},  {6, 0},  {6, 14}, {6, 15},

    {6, 13}, {6, 12}, {4, 8},  {4, 14}, {4, 15},

    {4, 5},  {4, 6},  {4, 7},
};

/*
    指示灯和显存的映射关系
    挡位边框对应的指示灯，从左侧的缺口处开始数，从0开始
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t gear_border_map[] = {
    // 挡位边框对应的指示灯
    {3, 10}, {0, 13}, {5, 1},  {5, 3},  {5, 4},

    {5, 6},  {5, 9},  {5, 12}, {5, 13}, {5, 15},

    {3, 15}, {3, 13},
};

/*
    指示灯和显存的映射关系
    挡位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t gear_map[] = {
    // a 段 ~ g 段
    {5, 5}, {5, 10}, {5, 14}, {3, 11}, {5, 0}, {5, 2}, {5, 11},
};

/*
    指示灯和显存的映射关系
    发动机转速面板上的分割线对应的指示灯（从左往右，从0开始）
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t engine_speed_split_line_map[] = {
    {7, 2}, {7, 0}, {0, 14}, {3, 9}, {3, 12}, {3, 14}, {3, 0}, {3, 1},
};

/*
    指示灯和显存的映射关系
    小时个位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t hour_bit_0_map[] = {
    {1, 15}, {1, 13}, {1, 10}, {1, 9}, {1, 11}, {1, 14}, {1, 12},
};

/*
    指示灯和显存的映射关系
    小时十位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t hour_bit_1_map[] = {
    {0, 1}, {0, 3}, {0, 6}, {0, 7}, {0, 5}, {0, 2}, {0, 4},
};

/*
    指示灯和显存的映射关系
    分钟个位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t minute_bit_0_map[] = {
    {3, 5}, {3, 6}, {2, 2}, {2, 1}, {2, 0}, {3, 4}, {3, 7},
};

/*
    指示灯和显存的映射关系
    分钟十位对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t minute_bit_1_map[] = {
    {1, 0}, {1, 2}, {1, 4}, {1, 7}, {1, 5}, {1, 1}, {1, 3},
};

/*
    指示灯和显存的映射关系
    油量格数上方的顶部标记对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t fuel_lev_upper_marker_map[] = {
    {2, 12}, {2, 13}, {2, 14}, {2, 15}, {2, 5}, {2, 4},
};

/*
    指示灯和显存的映射关系
    油量格数对应的指示灯
    对应的显存： aip3368h_engine_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t fuel_lev_map[] = {
    {2, 11}, {2, 10}, {2, 9}, {2, 8}, {2, 7}, {2, 6},
};

/*
    指示灯和显存的映射关系
    时速第 0 位对应的指示灯（从右往左，从0开始）
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t speed_bit_0_map[] = {
    // a 段 ~ g 段
    {0, 13}, {0, 14}, {0, 12}, {0, 8}, {5, 8}, {0, 10}, {0, 11},
};

/*
    指示灯和显存的映射关系
    时速第 1 位对应的指示灯（从右往左，从0开始）
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t speed_bit_1_map[] = {
    // a 段 ~ g 段
    {5, 5}, {5, 6}, {5, 9}, {5, 10}, {5, 12}, {5, 4}, {5, 11},
};

/*
    指示灯和显存的映射关系
    时速第 2 位对应的指示灯（从右往左，从0开始）
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t speed_bit_2_map[] = {
    // a 段 ~ g 段
    {5, 2}, {5, 3}, {5, 13}, {5, 15}, {5, 14}, {5, 1}, {5, 0},
};

/*
    指示灯和显存的映射关系
    时速面板上的分割线
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t speed_split_line_map[] = {
    {4, 5}, {4, 6}, {4, 7}, {4, 8}, {1, 7}, {5, 7}, {0, 9}, {0, 1},
};

/*
    指示灯和显存的映射关系
    里程对应的指示灯
    对应的显存： aip3368h_speed_panel_display_buff

    [0][0] ~ [0][6]，里程第 0 位数码管（从右往左，从0开始）
    [1][0] ~ [1][6]，里程第 1 位数码管（从右往左，从0开始）
*/
static const aip3368h_display_mapping_t mileage_map[][7] = {
    {{0, 7}, {0, 6}, {0, 5}, {1, 15}, {1, 12}, {1, 11}, {1, 14}},

    {{1, 8}, {1, 9}, {1, 10}, {2, 15}, {2, 13}, {1, 0}, {2, 14}},

    {{1, 5}, {1, 1}, {2, 12}, {2, 10}, {2, 9}, {1, 2}, {2, 11}},

    {{1, 6}, {1, 3}, {2, 8}, {3, 6}, {4, 14}, {4, 13}, {1, 4}},

    {{4, 9}, {4, 12}, {4, 15}, {3, 5}, {3, 4}, {4, 10}, {4, 11}},

    {{4, 3}, {4, 1}, {3, 3}, {3, 2}, {3, 1}, {4, 2}, {4, 0}},
};

/*
    指示灯和显存的映射关系
    电池电量上方的顶部标记对应的指示灯
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t bat_lev_upper_marker_map[] = {
    {3, 15}, {3, 13}, {3, 8}, {2, 6}, {2, 2}, {2, 0},
};

/*
    指示灯和显存的映射关系
    电池电量对应的指示灯
    对应的显存： aip3368h_speed_panel_display_buff
*/
static const aip3368h_display_mapping_t bat_lev_map[] = {
    {3, 12}, {3, 11}, {3, 10}, {3, 9}, {2, 7}, {2, 5}, {2, 4}, {2, 3}, {2, 1},
};

/**
 * @brief 显示指定的发动机转速对应的格子（不会清空原来的显示）
 *
 * @param gear 挡位 0 ~ 26
 *      0:显示第 0 格指示灯
 *      1:显示第 1 格指示灯
 *
 */
void __aip3368h_display_engine_speed_gear__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[idx]
                                                     .buff_index] |=
            (0x01 << engine_speed_gear_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[idx]
                                                     .buff_index] &=
            ~(0x01 << engine_speed_gear_map[idx].bit_offset);
    }
}

/**
 * @brief 显示指定的发动机转速挡位
 *
 * @param gear 挡位 0 ~ 27
 *
 *      0：空挡，清空显示
 *      1：1挡，显示1格
 *
 */
void aip3368h_display_engine_speed_gear(u8 gear)
{
    // 清空原来的显示
    u8 i;

    for (i = 0; i < ARRAY_SIZE(engine_speed_gear_map); i++) {
        if (gear <= i) {
            // 如果传参的挡位，比当前遍历的挡位还要小，清空对应的显示
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i]
                                                         .buff_index] &=
                ~(0x01 << engine_speed_gear_map[i].bit_offset);
        } else if ((gear - i) > 0) {
            // 如果传参的挡位，比当前遍历的挡位还要大（至少要大于等于1）
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i]
                                                         .buff_index] |=
                (0x01 << engine_speed_gear_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示指定的发动机转速对应的刻度条（不会清空原来的显示）
 *
 * @param idx 指示灯索引 0 ~ 17
 *
 * @param is_display 是否显示
 *      0：不显示
 *      1：显示
 *
 */
void __aip3368h_display_engine_speed_scale_bar__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[idx]
                                                     .buff_index] |=
            (0x01 << engine_speed_scale_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[idx]
                                                     .buff_index] &=
            ~(0x01 << engine_speed_scale_map[idx].bit_offset);
    }
}

/**
 * @brief 显示指定的发动机转速对应的刻度条
 *
 * @param scale 刻度 0 ~ 18
 *      0：不显示
 *      1：显示第 0 个刻度指示灯
 *
 */
void aip3368h_display_engine_speed_scale_bar(u8 scale)
{
    // 清空原来的显示
    u8 i;

    for (i = 0; i < ARRAY_SIZE(engine_speed_scale_map); i++) {

        if (scale <= i) {
            // 如果传参的数值，比当前遍历的数值还要小，清空对应的显示
            aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[i]
                                                         .buff_index] &=
                ~(0x01 << engine_speed_scale_map[i].bit_offset);
        } else if ((scale - i) > 0) {
            // 如果传参的数值，比当前遍历的数值还要大（至少要大于等于1）
            aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[i]
                                                         .buff_index] |=
                (0x01 << engine_speed_scale_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示 x1000rpm 字样对应的指示灯
 *
 * @param is_display 是否显示
 *          0：不显示
 *          1：显示
 *
 */
void aip3368h_display_x1000rpm_light(u8 is_display)
{
    if (is_display) {
        // x1000RPM 字样，第 0 个指示灯
        aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 7;
        // x1000RPM 字样，第 1 个指示灯
        aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 8;
    } else {
        // x1000RPM 字样，第 0 个指示灯
        aip3368h_engine_speed_panel_display_buff[5] &= ~(0x01 << 7);
        // x1000RPM 字样，第 1 个指示灯
        aip3368h_engine_speed_panel_display_buff[5] &= ~(0x01 << 8);
    }
}

/**
 * @brief 显示挡位边框
 *
 * @param is_display 是否显示
 *          0：不显示
 *          1：显示
 *
 */
void aip3368h_display_gear_border(u8 is_display)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(gear_border_map); i++) {
        if (is_display) {
            aip3368h_engine_speed_panel_display_buff[gear_border_map[i]
                                                         .buff_index] |=
                (0x01 << gear_border_map[i].bit_offset);
        } else {
            aip3368h_engine_speed_panel_display_buff[gear_border_map[i]
                                                         .buff_index] &=
                ~(0x01 << gear_border_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示挡位数码管的指定段，不会清空原来的显示
 *
 */
void __aip3368h_display_gear_seg__(seg_idx_t seg_idx)
{
    aip3368h_engine_speed_panel_display_buff[gear_map[seg_idx].buff_index] |=
        (0x01 << gear_map[seg_idx].bit_offset);
}

/**
 * @brief 让挡位对应的数码管显示数字
 *
 * @param num 0 ~ 9
 *
 */
void __aip3368h_display_gear_num__(u8 num)
{
    u8 i;
    u8 segment_code;

    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[num];

    // 遍历 a ~ g 段数码管
    for (i = 0; i < 7; i++) {
        // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
        if (segment_code & (0x01 << i)) {
            aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] |=
                (0x01 << gear_map[i].bit_offset);
        } else {
            aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] &=
                ~(0x01 << gear_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示挡位
 *
 * @param gear 挡位 0 ~ 6、GEAR_UNKNOWN
 *          0：空挡，
 *          1：1挡，显示1
 *          GEAR_UNKNOWN ： 表示什么挡位都没有
 */
void aip3368h_display_gear(u8 gear)
{
    // 清空原来的显示
    u8 i;
    u8 segment_code;

    // 清空数码管的显示
    for (i = 0; i < ARRAY_SIZE(gear_map); i++) {
        aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] &=
            ~(0x01 << gear_map[i].bit_offset);
    }

    if (GEAR_NEUTRAL == gear) {
        aip3368h_display_n_light(1);
    } else {
        aip3368h_display_n_light(0);
    }

    if (GEAR_NEUTRAL == gear || GEAR_UNKNOWN == gear) {
        // 如果什么挡位都没有，或者是检测到了空挡，显示"-"
        __aip3368h_display_gear_seg__(SEG_IDX_G);
    } else {
        // 1 ~ 6 挡，显示对应数字

        // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
        segment_code = digit_segment_code[gear];

        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[gear_map[i].buff_index] |=
                    (0x01 << gear_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 发动机转速面板分割线上的指定指示灯
 *
 * @param idx 指示灯的索引 0 ~ ARRAY_SIZE(engine_speed_split_line_map) - 1
 *      0：第 0 个指示灯
 *      1：第 1 个指示灯
 *
 * @param is_display
 *
 */
void __aip3368h_display_engine_speed_split_line_light__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[engine_speed_split_line_map[idx]
                                                     .buff_index] |=
            (0x01 << engine_speed_split_line_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[engine_speed_split_line_map[idx]
                                                     .buff_index] &=
            ~(0x01 << engine_speed_split_line_map[idx].bit_offset);
    }
}

/**
 * @brief 左转向对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_left_turn_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 1;
    } else {
        aip3368h_engine_speed_panel_display_buff[7] &= ~(0x01 << 1);
    }
}

/**
 * @brief 发动机故障对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_engine_fault_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 15;
    } else {
        aip3368h_engine_speed_panel_display_buff[0] &= ~(0x01 << 15);
    }
}

/**
 * @brief N 字样对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_n_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 0;
    } else {

        aip3368h_engine_speed_panel_display_buff[0] &= ~(0x01 << 0);
    }
}

/**
 * @brief 电话图标对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_phone_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 8;
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 8);
    }
}

/**
 * @brief 远光灯（大灯）对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_high_beam_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 3;
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 3);
    }
}

/**
 * @brief 右转向对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_right_turn_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 2;
    } else {
        aip3368h_engine_speed_panel_display_buff[3] &= ~(0x01 << 2);
    }
}

/**
 * @brief 显示小时的指定位
 *
 * @param bit_x 小时的指定位，0：个位，1：十位
 *
 * @param num 要显示的数字 0 ~ 9
 *
 */
void __aip3368h_display_hour_digit__(u8 bit_x, u8 num)
{
    u8 i;
    u8 segment_code;
    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[num];

    if (0 == bit_x) {
        // 第 0 位(个位)
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i]
                                                             .buff_index] |=
                    (0x01 << hour_bit_0_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_0_map[i].bit_offset);
            }
        }
    } else {
        // 第 1 位（十位）
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i]
                                                             .buff_index] |=
                    (0x01 << hour_bit_1_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << hour_bit_1_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 显示分钟的指定位
 *
 * @param bit_x 分钟的指定位，0：个位，1：十位
 *
 * @param num 要显示的数字 0 ~ 9
 *
 */
void __aip3368h_display_minute_digit__(u8 bit_x, u8 num)
{
    u8 i;
    u8 segment_code;
    // 获取该数字对应的7段码 (要显示的数字 --> 七段码)
    segment_code = digit_segment_code[num];

    if (0 == bit_x) {
        // 第 0 位(个位)
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i]
                                                             .buff_index] |=
                    (0x01 << minute_bit_0_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_0_map[i].bit_offset);
            }
        }
    } else {
        // 第 1 位（十位）
        // 遍历 a ~ g 段数码管
        for (i = 0; i < 7; i++) {
            // 检查该段是否需要点亮 ( segment_code 的对应 bit 是否为1)
            if (segment_code & (0x01 << i)) {
                aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i]
                                                             .buff_index] |=
                    (0x01 << minute_bit_1_map[i].bit_offset);
            } else {
                aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i]
                                                             .buff_index] &=
                    ~(0x01 << minute_bit_1_map[i].bit_offset);
            }
        }
    }
}

/**
 * @brief 时间中间的冒号对应的指示灯（时间分隔符）
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_time_colon_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 6;
    } else {
        aip3368h_engine_speed_panel_display_buff[1] &= ~(0x01 << 6);
    }
}

/**
 * @brief 显示时间
 *
 * @param hour 小时
 *
 * @param min 分钟
 *
 */
void aip3368h_display_time(u8 hour, u8 min)
{
    u8 i;

    // 遍历 a ~ g 段数码管，清空显示
    for (i = 0; i < 7; i++) {
        aip3368h_engine_speed_panel_display_buff[hour_bit_0_map[i].buff_index] &=
            ~(0x01 << hour_bit_0_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[hour_bit_1_map[i].buff_index] &=
            ~(0x01 << hour_bit_1_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[minute_bit_0_map[i].buff_index] &=
            ~(0x01 << minute_bit_0_map[i].bit_offset);
        aip3368h_engine_speed_panel_display_buff[minute_bit_1_map[i].buff_index] &=
            ~(0x01 << minute_bit_1_map[i].bit_offset);
    }

    __aip3368h_display_hour_digit__(0, hour % 10);
    __aip3368h_display_hour_digit__(1, hour / 10);
    __aip3368h_display_minute_digit__(0, min % 10);
    __aip3368h_display_minute_digit__(1, min / 10);
}

/**
 * @brief 油量 E 字样对应的指示灯（油量 empty）
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_fuel_empty_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 8;
    } else {
        aip3368h_engine_speed_panel_display_buff[0] &= ~(0x01 << 8);
    }
}

/**
 * @brief 油量图标对应的指示灯
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_fuel_icon_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 8;
    } else {
        aip3368h_engine_speed_panel_display_buff[1] &= ~(0x01 << 8);
    }
}

/**
 * @brief 油量 Full 对应的指示灯
 *
 */
void aip3368h_display_fuel_full_light(u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 3;
    } else {
        aip3368h_engine_speed_panel_display_buff[2] &= ~(0x01 << 3);
    }
}

/**
 * @brief 油量格数上方的顶部标记，不会清空原来的显示
 *
 * @param idx 0 ~ 5
 *      0：显示第 0 个指示灯
 *      1：显示第 1 个指示灯
 *
 * @param is_display 是否显示
 *
 */
void __aip3368h_display_fuel_lev_upper_marker__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[fuel_lev_upper_marker_map[idx]
                                                     .buff_index] |=
            (0x01 << fuel_lev_upper_marker_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[fuel_lev_upper_marker_map[idx]
                                                     .buff_index] &=
            ~(0x01 << fuel_lev_upper_marker_map[idx].bit_offset);
    }
}

/**
 * @brief 油量格数上方的顶部标记
 *
 * @param is_display 是否显示
 *
 */
void aip3368h_display_fuel_lev_upper_marker(u8 is_display)
{
    u8 i; // 遍历第 0 ~ ARRAY_SIZE(fuel_lev_upper_marker_map) - 1 个指示灯
    if (is_display) {
        for (i = 0; i < ARRAY_SIZE(fuel_lev_upper_marker_map); i++) {
            aip3368h_engine_speed_panel_display_buff[fuel_lev_upper_marker_map[i]
                                                         .buff_index] |=
                (0x01 << fuel_lev_upper_marker_map[i].bit_offset);
        }
    } else {
        for (i = 0; i < ARRAY_SIZE(fuel_lev_upper_marker_map); i++) {
            aip3368h_engine_speed_panel_display_buff[fuel_lev_upper_marker_map[i]
                                                         .buff_index] &=
                ~(0x01 << fuel_lev_upper_marker_map[i].bit_offset);
        }
    }
}

/**
 * @brief 油量格数对应的指示灯
 *
 * @param idx 0 ~ ARRYA_SIZE(fuel_lev_map)
 *      0：第 0 格
 *      1：第 1 格
 *
 * @param is_display 是否显示
 *
 */
void __aip3368h_display_fuel_lev__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_engine_speed_panel_display_buff[fuel_lev_map[idx].buff_index] |=
            (0x01 << fuel_lev_map[idx].bit_offset);
    } else {
        aip3368h_engine_speed_panel_display_buff[fuel_lev_map[idx].buff_index] &=
            ~(0x01 << fuel_lev_map[idx].bit_offset);
    }
}

/**
 * @brief 油量格数显示
 *
 * @param lev 0 ~ ARRYA_SIZE(fuel_lev_map)
 *      0：不显示
 *      1：显示第 0 格
 *      2：显示 0 ~ 1 格
 *      ARRYA_SIZE(fuel_lev_map)：显示 0 ~ ARRYA_SIZE(fuel_lev_map) - 1 格
 *
 */
void aip3368h_display_fuel_lev(u8 lev)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(fuel_lev_map); i++) {
        if (lev <= i) {
            // 如果当前传参的值，比当前遍历的值还要小，清空对应的显示
            aip3368h_engine_speed_panel_display_buff[fuel_lev_map[i].buff_index] &=
                ~(0x01 << fuel_lev_map[i].bit_offset);
        } else if ((lev - i) > 0) {
            // 如果当前传参的值，比当前遍历的值还要大（至少要大于等于1），显示对应的指示灯
            aip3368h_engine_speed_panel_display_buff[fuel_lev_map[i].buff_index] |=
                (0x01 << fuel_lev_map[i].bit_offset);
        }
    }
}

#if 0
/**
 * @brief 时速数码管对应的指示灯显示驱动
 *
 * @param bit_x 0 ~ 2，对应第 0 ~ 2 个数码管（从右往左，从0开始）
 *           0：第 0 个数码管
 *           1：第 1 个数码管
 *           2：第 2 个数码管
 *
 * @param seg_idx 0 ~ 6，对应数码管上的第 a ~ g 个段
 *
 * @param is_display 是否显示
 *
 */
void __aip3368h_display_speed_seg__(u8 bit_x, seg_idx_t seg_idx, u8 is_display)
{
    if (is_display) {
        switch (bit_x) {
        case 0:
            aip3368h_speed_panel_display_buff[speed_bit_0_map[seg_idx].buff_index] |=
                (0x01 << speed_bit_0_map[seg_idx].bit_offset);
            break;
        case 1:
            aip3368h_speed_panel_display_buff[speed_bit_1_map[seg_idx].buff_index] |=
                (0x01 << speed_bit_1_map[seg_idx].bit_offset);
            break;
        case 2:
            aip3368h_speed_panel_display_buff[speed_bit_2_map[seg_idx].buff_index] |=
                (0x01 << speed_bit_2_map[seg_idx].bit_offset);
            break;
        default:
            break;
        }

    } else {
        switch (bit_x) {
        case 0:
            aip3368h_speed_panel_display_buff[speed_bit_0_map[seg_idx].buff_index] &=
                ~(0x01 << speed_bit_0_map[seg_idx].bit_offset);
            break;
        case 1:
            aip3368h_speed_panel_display_buff[speed_bit_1_map[seg_idx].buff_index] &=
                ~(0x01 << speed_bit_1_map[seg_idx].bit_offset);
            break;
        case 2:
            aip3368h_speed_panel_display_buff[speed_bit_2_map[seg_idx].buff_index] &=
                ~(0x01 << speed_bit_2_map[seg_idx].bit_offset);
            break;
        default:
            break;
        }
    }
}
#endif

/**
 * @brief 控制时速某一位数码管显示的值
 *
 * @param bit_x
 *
 * @param num 0 ~ 9
 *      如果 is_display == 0，num 的值无效
 *
 * @param is_display
 *
 */
void aip3368h_display_speed_bit(u8 bit_x, u8 num, u8 is_display)
{
    u8 i;
    u8 segment_code;

    segment_code = digit_segment_code[num];

    // 遍历 a ~ g 段数码管
    for (i = 0; i < 7; i++) {
        // 检查该段是否需要点亮 (segment_code的对应bit是否为1)
        if (is_display && segment_code & (0x01 << i)) {
            // 如果需要点亮
            switch (bit_x) {
            case 0:
                aip3368h_speed_panel_display_buff[speed_bit_0_map[i].buff_index] |=
                    (0x01 << speed_bit_0_map[i].bit_offset);
                break;
            case 1:
                aip3368h_speed_panel_display_buff[speed_bit_1_map[i].buff_index] |=
                    (0x01 << speed_bit_1_map[i].bit_offset);
                break;
            case 2:
                aip3368h_speed_panel_display_buff[speed_bit_2_map[i].buff_index] |=
                    (0x01 << speed_bit_2_map[i].bit_offset);
                break;
            default:
                break;
            }
        } else {
            // 如果不需要点亮
            switch (bit_x) {
            case 0:
                aip3368h_speed_panel_display_buff[speed_bit_0_map[i].buff_index] &=
                    ~(0x01 << speed_bit_0_map[i].bit_offset);
                break;
            case 1:
                aip3368h_speed_panel_display_buff[speed_bit_1_map[i].buff_index] &=
                    ~(0x01 << speed_bit_1_map[i].bit_offset);
                break;
            case 2:
                aip3368h_speed_panel_display_buff[speed_bit_2_map[i].buff_index] &=
                    ~(0x01 << speed_bit_2_map[i].bit_offset);
                break;
            default:
                break;
            }
        }
    }
}

/**
 * @brief 时速数码管显示时速
 *
 * @param speed 0 ~ 999
 *      参数小于 10 ，只有第 0 位数码管在显示
 *      参数小于 100 ，只有第 0、1 位数码管在显示
 *
 */
void aip3368h_display_speed(u16 speed)
{
    /*
        根据传参，先计算有多少位有效数字，
        再根据有效数字进行对应显示
    */
    // 传参的有效数据位：
    u8 valid_bits = 0;
    u16 tmp = 0; // 计算有效数据位使用的临时变量
    u8 i;

    tmp = speed;
    while (1) {
        valid_bits++; // 刚进入，默认至少有1位有效数据
        tmp /= 10;
        if (tmp == 0) {
            break;
        }
    }

#if USER_DEBUG_ENABLE
    // printf("valid_bits == %u\n", (u16)valid_bits);
#endif

    // 遍历第 0 ~ 3 位数码管
    for (i = 0; i < 3; i++) {
        if (i < valid_bits) {
            // 遍历第 0 ~ valid_bits（最大为3） 位数码管
            aip3368h_display_speed_bit(i, speed % 10, 1);
            speed /= 10;
        } else {
            // 不让当前位对应的数码管进行显示
            aip3368h_display_speed_bit(i, 0, 0);
        }
    }
}

/**
 * @brief mph 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_mph_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[0] |= (0x01 << 15);
    } else {
        aip3368h_speed_panel_display_buff[0] &= ~(0x01 << 15);
    }
}

/**
 * @brief km/h 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_kmh_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[0] |= (0x01 << 0);
    } else {
        aip3368h_speed_panel_display_buff[0] &= ~(0x01 << 0);
    }
}

/**
 * @brief 时速面板上的分割线对应的指示灯
 *
 * @param idx 0 ~ ARRAY_SIZE(speed_split_line_map) - 1
 *
 * @param is_display
 *
 */
void __aip3368h_display_speed_split_line_light__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[speed_split_line_map[idx].buff_index] |=
            (0x01 << speed_split_line_map[idx].bit_offset);
    } else {
        aip3368h_speed_panel_display_buff[speed_split_line_map[idx].buff_index] &=
            ~(0x01 << speed_split_line_map[idx].bit_offset);
    }
}

/**
 * @brief 时速面板上的分割线对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_speed_split_line_light(u8 is_display)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(speed_split_line_map); i++) {
        __aip3368h_display_speed_split_line_light__(i, is_display);
    }
}

/**
 * @brief TRIP 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_trip_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[4] |= (0x01 << 4);
    } else {
        aip3368h_speed_panel_display_buff[4] &= ~(0x01 << 4);
    }
}

/**
 * @brief ODO 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_odo_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[3] |= (0x01 << 0);
    } else {
        aip3368h_speed_panel_display_buff[3] &= ~(0x01 << 0);
    }
}

/**
 * @brief 里程 miles 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_miles_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[0] |= (0x01 << 2);
    } else {
        aip3368h_speed_panel_display_buff[0] &= ~(0x01 << 2);
    }
}

/**
 * @brief 里程 km 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_km_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[0] |= (0x01 << 3);
    } else {
        aip3368h_speed_panel_display_buff[0] &= ~(0x01 << 3);
    }
}

/**
 * @brief 里程对应的某一位数码管，显示对应数字或不显示
 *
 * @param bit_x
 * @param num
 * @param is_display
 */
void __aip3368h_display_mileage_bit_x__(u8 bit_x, u8 num, u8 is_display)
{
    u8 i;
    u8 segment_code = 0;
    segment_code = digit_segment_code[num];

    // 遍历数码管的 a ~ g 段
    for (i = 0; i < 7; i++) {
        if (is_display && (segment_code & (0x01 << i))) {
            // 如果当前位对应的数码管要显示，并且当前段对应的指示灯要点亮
            aip3368h_speed_panel_display_buff[mileage_map[bit_x][i].buff_index] |=
                (0x01 << mileage_map[bit_x][i].bit_offset);
        } else {
            aip3368h_speed_panel_display_buff[mileage_map[bit_x][i].buff_index] &=
                ~(0x01 << mileage_map[bit_x][i].bit_offset);
        }
    }
}

/**
 * @brief 里程的小数点
 *
 * @param is_display
 */
void aip3368h_display_mileage_point_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[1] |= (0x01 << 13);
    } else {
        aip3368h_speed_panel_display_buff[1] &= ~(0x01 << 13);
    }
}

/**
 * @brief 显示里程
 *      不包括里程的单位，km 或 miles
 *
 * @param mileage
 *          如果显示总里程，mileage == 12345，则显示 12345
 *          如果显示当前里程，mileage == 12345，则显示 1234.5
 *
 * @param mileage_unit_type
 *
 */
void aip3368h_display_mileage(u32 mileage,
                              mileage_display_mode_t mileage_display_mode)
{
    u8 i;

    if (mileage_display_mode == MILEAGE_DISPLAY_MODE_ODO) {
        // 显示总里程，关闭小数点对应的指示灯
        aip3368h_display_mileage_point_light(0);
    } else {
        // 显示当前里程（小里程），打开小数点对应的指示灯
        aip3368h_display_mileage_point_light(1);
    }

    // 遍历里程的第 0 ~ 5 位数码管
    for (i = 0; i < 6; i++) {
        __aip3368h_display_mileage_bit_x__(i, (mileage % 10), 1);
        mileage /= 10;
    }
}

/**
 * @brief 电池电量 8 字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_8_symbol_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[3] |= (0x01 << 14);
    } else {
        aip3368h_speed_panel_display_buff[3] &= ~(0x01 << 14);
    }
}

/**
 * @brief 电池图标对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_icon_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[3] |= (0x01 << 7);
    } else {
        aip3368h_speed_panel_display_buff[3] &= ~(0x01 << 7);
    }
}

/**
 * @brief 电池16字样对应的指示灯
 *
 * @param is_display
 *
 */
void aip3368h_display_battery_16_symbol_light(u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[0] |= (0x01 << 4);
    } else {
        aip3368h_speed_panel_display_buff[0] &= ~(0x01 << 4);
    }
}

/**
 * @brief 电池电量上方的顶部标记对应的指示灯
 *
 * @param idx 0 ~ 5，第 idx 个指示灯（从左往右，从0开始）
 *
 * @param is_display
 *
 */
void __aip3368h_display_bat_lev_upper_marker_light__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[bat_lev_upper_marker_map[idx]
                                              .buff_index] |=
            (0x01 << bat_lev_upper_marker_map[idx].bit_offset);
    } else {
        aip3368h_speed_panel_display_buff[bat_lev_upper_marker_map[idx]
                                              .buff_index] &=
            ~(0x01 << bat_lev_upper_marker_map[idx].bit_offset);
    }
}

/**
 * @brief 电池电量上方的顶部标记对应的指示灯
 *
 * @param is_display 直接控制整体是否显示
 *
 */
void aip3368h_display_bat_lev_upper_marker_light(u8 is_display)
{
    u8 i;
    for (i = 0; i < ARRAY_SIZE(bat_lev_upper_marker_map); i++) {
        __aip3368h_display_bat_lev_upper_marker_light__(i, is_display);
    }
}

/**
 * @brief 电池电量对应的指示灯
 *
 * @param idx 0 ~ 8，第 idx 个指示灯（从左往右，从0开始）
 *
 * @param is_display
 *
 */
void __aip3368h_display_bat_lev_light__(u8 idx, u8 is_display)
{
    if (is_display) {
        aip3368h_speed_panel_display_buff[bat_lev_map[idx].buff_index] |=
            (0x01 << bat_lev_map[idx].bit_offset);
    } else {
        aip3368h_speed_panel_display_buff[bat_lev_map[idx].buff_index] &=
            ~(0x01 << bat_lev_map[idx].bit_offset);
    }
}

/**
 * @brief 根据传参，显示对应的电池电量等级（不包括低电量提示）
 *
 * @param bat_lev 1 ~ 8，第 idx 个指示灯（从左往右，从0开始）
 *              0：清空显示
 *              1：显示第 0 个指示灯
 *              2：显示第 1 个指示灯
 *
 */
void aip3368h_display_bat_lev_light(u8 bat_lev)
{
    u8 i;
    // 遍历整个电池电量等级对应的指示灯
    for (i = 0; i < ARRAY_SIZE(bat_lev_map); i++) {
        // 如果当前传参的值减去1，比当前遍历的值还要小，清空对应的显示
        if (bat_lev <= i) {
            __aip3368h_display_bat_lev_light__(i, 0);
        } else if (bat_lev >= (i + 1)) {
            // 例如，如果要点亮第 0 个指示灯，bat_lev至少要大于等于1
            __aip3368h_display_bat_lev_light__(i, 1);
        }
    }
}

#if AIP3368H_DISPLAY_TEST_ENABLE

#if 0
/**
 * @brief 测试发动机转速的格子显示
 *
 */
void aip3368h_display_engine_speed_gear_test_1ms_isr(void)
{
    static u8 gear = 0;
    static u8 dir = 0;  // 控制递增、递减
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 200) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_engine_speed_gear(gear);

    if (0 == dir) {
        gear++;
    } else {
        gear--;
    }

    if ((gear >= ARRAY_SIZE(engine_speed_gear_map)) || (gear == 0)) {
        dir = !dir;
    }
}

/**
 * @brief 测试发动机转速的刻度条显示
 *
 */
void aip3368h_display_engine_speed_scale_test_1ms_isr(void)
{
    static u8 scale = 0;
    static u8 dir = 0;  // 控制递增、递减
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 200) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_engine_speed_scale_bar(scale);

    if (0 == dir) {
        scale++;
    } else {
        scale--;
    }

    if ((scale >= ARRAY_SIZE(engine_speed_scale_map)) || (scale == 0)) {
        dir = !dir;
    }
}

void aip3368h_display_gear_test_1ms_isr(void)
{
    static u8 gear = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_gear(gear);

    gear++;
    if (gear > 6) {
        gear = 0;
    }
}

void aip3368h_display_engine_speed_split_line_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    if (idx > 0) {
        __aip3368h_display_engine_speed_split_line_light__(idx - 1, 0);
    } else {
        __aip3368h_display_engine_speed_split_line_light__(ARRAY_SIZE(engine_speed_split_line_map) - 1, 0);
    }

    __aip3368h_display_engine_speed_split_line_light__(idx, 1);

    idx++;
    if (idx >= ARRAY_SIZE(engine_speed_split_line_map)) {
        idx = 0;
    }
}

void aip3368h_display_time_test_1ms_isr(void)
{
    static u8 hour = 0;
    static u8 min = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_time(hour, min);

    // __aip3368h_display_min_digit__(0, min);
    // __aip3368h_display_min_digit__(1, min / 10);

    hour++;
    min++;
    if (hour >= 24) {
        hour = 0;
    }

    if (min >= 60) {
        min = 0;
    }
}


void aip3368h_display_fuel_lev_upper_marker_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    if (idx > 0) {
        __aip3368h_display_fuel_lev_upper_marker__(idx - 1, 0);
    } else {
        __aip3368h_display_fuel_lev_upper_marker__(ARRAY_SIZE(fuel_lev_upper_marker_map) - 1, 0);
    }

    __aip3368h_display_fuel_lev_upper_marker__(idx, 1);

    idx++;
    if (idx >= ARRAY_SIZE(fuel_lev_upper_marker_map)) {
        idx = 0;
    }
}


void aip3368h_display_fuel_lev_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;

    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

#if 0
    if (idx > 0) {
        __aip3368h_display_fuel_lev__(idx - 1, 0);
    } else {
        __aip3368h_display_fuel_lev__(ARRAY_SIZE(fuel_lev_map) - 1, 0);
    }

    __aip3368h_display_fuel_lev__(idx, 1);

    idx++;
    if (idx >= ARRAY_SIZE(fuel_lev_map)) {
        idx = 0;
    }
#else

    aip3368h_display_fuel_lev(idx);

    idx++;
    if (idx > ARRAY_SIZE(fuel_lev_map)) {
        idx = 0;
    }
#endif
}

void aip3368h_display_speed_test_1ms_isr(void)
{
    static u16 num = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;

#if 1
    // 测试 aip3368h_display_speed_bit() 时使用
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_speed_bit(0, num, 1);
    aip3368h_display_speed_bit(1, num, 1);
    aip3368h_display_speed_bit(2, num, 1);

    num++;
    if (num > 9) {
        num = 0;
    }
#else
    // 测试 aip3368h_display_speed() 时使用
    if (cnt < 50) {
        return;
    } else {
        cnt = 0;
    }

    aip3368h_display_speed(num);
    num++;
    if (num > 999) {
        num = 0;
    }
#endif
}



void aip3368h_display_speed_split_line_light_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    if (idx > 0) {
        __aip3368h_display_speed_split_line_light__(idx - 1, 0);
    } else {
        __aip3368h_display_speed_split_line_light__(ARRAY_SIZE(speed_split_line_map) - 1, 0);
    }

    __aip3368h_display_speed_split_line_light__(idx, 1);

    idx++;
    if (idx >= ARRAY_SIZE(speed_split_line_map)) {
        idx = 0;
    }
}

void aip3368h_display_mileage_test_1ms_isr(void)
{
    static u16 num = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;

    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    __aip3368h_display_mileage_bit_x__(0, num, 1);
    __aip3368h_display_mileage_bit_x__(1, num, 1);
    __aip3368h_display_mileage_bit_x__(2, num, 1);
    __aip3368h_display_mileage_bit_x__(3, num, 1);
    __aip3368h_display_mileage_bit_x__(4, num, 1);
    __aip3368h_display_mileage_bit_x__(5, num, 1);

    num++;
    if (num > 9) {
        num = 0;
    }
} 

void aip3368h_display_bat_lev_upper_marker_light_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    if (idx > 0) {
        __aip3368h_display_bat_lev_upper_marker_light__(idx - 1, 0);
    } else {
        __aip3368h_display_bat_lev_upper_marker_light__(ARRAY_SIZE(bat_lev_upper_marker_map) - 1, 0);
    }

    __aip3368h_display_bat_lev_upper_marker_light__(idx, 1);

    idx++;
    if (idx >= ARRAY_SIZE(bat_lev_upper_marker_map)) {
        idx = 0;
    }
}
    
    void aip3368h_display_bat_lev_light_test_1ms_isr(void)
{
    static u8 idx = 0;
    static u16 cnt = 0; // 计数，用于控制显示的频率
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

#if 0
    if (idx > 0) {
        __aip3368h_display_bat_lev_light__(idx - 1, 0);
    } else {
        __aip3368h_display_bat_lev_light__(ARRAY_SIZE(bat_lev_map) - 1, 0);
    }

    __aip3368h_display_bat_lev_light__(idx, 1);
#else
    aip3368h_display_bat_lev_light(idx);
#endif

    idx++;
    if (idx >= ARRAY_SIZE(bat_lev_map)) {
        idx = 0;
    }
}



// ==============================================================================
// ==============================================================================

// 简单的指示灯闪烁测试
void aip3368h_display_light_blink_test_1ms_isr(void)
{
    static u16 cnt = 0; // 计数，用于控制显示的频率
    static u8 is_display = 0;
    cnt++;
    if (cnt < 500) {
        return;
    } else {
        cnt = 0;
    }

    // aip3368h_display_x1000rpm_light(is_display);
    // aip3368h_display_gear_border(is_display);
    // aip3368h_display_left_turn_light(is_display);
    // aip3368h_display_engine_fault_light(is_display);
    // aip3368h_display_n_light(is_display);
    // aip3368h_display_phone_light(is_display);
    // aip3368h_display_high_beam_light(is_display);
    // aip3368h_display_right_turn_light(is_display);
    // aip3368h_display_time_colon_light(is_display);
    // aip3368h_display_fuel_empty_light(is_display);
    // aip3368h_display_fuel_icon_light(is_display);
    // aip3368h_display_fuel_full_light(is_display);
    // aip3368h_display_fuel_lev_upper_marker(is_display);
    // aip3368h_display_mph_light(is_display);
    // aip3368h_display_kmh_light(is_display);
    // aip3368h_display_speed_split_line_light(is_display);
    // aip3368h_display_trip_light(is_display);
    // aip3368h_display_odo_light(is_display);
    // aip3368h_display_miles_light(is_display);
    // aip3368h_display_km_light(is_display);
    // aip3368h_display_battery_8_symbol_light(is_display);
    // aip3368h_display_battery_icon_light(is_display);
    // aip3368h_display_battery_16_symbol_light(is_display);
    // aip3368h_display_bat_lev_upper_marker_light(is_display);

    is_display = !is_display;
}
#endif

#if 0
void aip3368h_display_test(void)
{
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 0; // km/h 字样指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 1; // 时速面板上的分割线，第 7 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 2; // miles 字样指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 3; // km 字样指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 4; // 电池电量 16 字样指示灯
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 5; // 里程第 0 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 6; // 里程第 0 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 7; // 里程第 0 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 8; // 时速第 0 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 9; // 时速面板上的分割线，第 6 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 10; // 时速第 0 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 11; // 时速第 0 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 12; // 时速第 0 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 13; // 时速第 0 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 14; // 时速第 0 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[0] |= 0x01 << 15; // mph 字样指示灯

    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 0; // 里程第 1 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 1;  // 里程第 2 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 2;  // 里程第 2 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 3;  // 里程第 3 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 4;  // 里程第 3 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 5;  // 里程第 2 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 6;  // 里程第 3 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 7;  // 时速面板上的分割线，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 8;  // 里程第 1 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 9;  // 里程第 1 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 10; // 里程第 1 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 11; // 里程第 0 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 12; // 里程第 0 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 13; // 里程，小数点对应的指示灯
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 14; // 里程第 0 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[1] |= 0x01 << 15; // 里程第 0 位， d 段指示灯（从右往左，从0开始）

    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 0;  // 电池电量刻度条，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 1;  // 电池电量，第 8 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 2;  // 电池电量刻度条，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 3;  // 电池电量，第 7 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 4;  // 电池电量，第 6 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 5;  // 电池电量，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 6;  // 电池电量刻度条，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 7;  // 电池电量，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 8;  // 里程第 3 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 9;  // 里程第 2 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 10; // 里程第 2 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 11; // 里程第 2 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 12; // 里程第 2 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 13; // 里程第 1 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 14; // 里程第 1 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[2] |= 0x01 << 15; // 里程第 1 位， d 段指示灯（从右往左，从0开始）

    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 0;  // ODO 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 1;  // 里程第 5 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 2;  // 里程第 5 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 3;  // 里程第 5 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 4;  // 里程第 4 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 5;  // 里程第 4 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 6;  // 里程第 3 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 7;  // 电池图标对应的指示灯（红）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 8;  // 电池电量刻度条，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 9;  // 电池电量，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 10; // 电池电量，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 11; // 电池电量，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 12; // 电池电量，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 13; // 电池电量刻度条，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 14; // 电池电量 8 字样指示灯
    // aip3368h_speed_panel_display_buff[3] |= 0x01 << 15; // 电池电量刻度条，第 0 个指示灯（从左往右，从0开始）

    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 0;  // 里程第 5 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 1;  // 里程第 5 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 2;  // 里程第 5 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 3;  // 里程第 5 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 4;  // TRIP 字样对应的指示灯
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 5;  // 时速面板上的分割线，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 6;  // 时速面板上的分割线，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 7;  // 时速面板上的分割线，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 8;  // 时速面板上的分割线，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 9;  // 里程第 4 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 10; // 里程第 4 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 11; // 里程第 4 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 12; // 里程第 4 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 13; // 里程第 3 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 14; // 里程第 3 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[4] |= 0x01 << 15; // 里程第 4 位， c 段指示灯（从右往左，从0开始）

    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 0;  // 时速第 2 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 1;  // 时速第 2 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 2;  // 时速第 2 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 3;  // 时速第 2 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 4;  // 时速第 1 位， f 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 5;  // 时速第 1 位， a 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 6;  // 时速第 1 位， b 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 7;  // 时速面板上的分割线，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 8;  // 时速第 0 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 9;  // 时速第 1 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 10; // 时速第 1 位， d 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 11; // 时速第 1 位， g 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 12; // 时速第 1 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 13; // 时速第 2 位， c 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 14; // 时速第 2 位， e 段指示灯（从右往左，从0开始）
    // aip3368h_speed_panel_display_buff[5] |= 0x01 << 15; // 时速第 2 位， d 段指示灯（从右往左，从0开始）

    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 0;  // N 字样对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 1;  // 时间，小时十位，a 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 2;  // 时间，小时十位，f 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 3;  // 时间，小时十位，b 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 4;  // 时间，小时十位，g 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 5;  // 时间，小时十位，e 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 6;  // 时间，小时十位，c 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 7;  // 时间，小时十位，d 段指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 8;  // 油量 E 字样对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 9;  // NC
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 10; // NC
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 11; // NC
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 12; // NC
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 13; // 挡位边框，第 1 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 14; // 发动机转速面板分割线，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 15; // 发动机故障指示灯

    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 0;  // 时间，分钟十位，a 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 1;  // 时间，分钟十位，f 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 2;  // 时间，分钟十位，b 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 3;  // 时间，分钟十位，g 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 4;  // 时间，分钟十位，c 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 5;  // 时间，分钟十位，e 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 6;  // 时间分隔符对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 7;  // 时间，分钟十位，d 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 8;  // 油量图标对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 9;  // 时间，小时个位，d 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 10; // 时间，小时个位，c 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 11; // 时间，小时个位，e 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 12; // 时间，小时个位，g 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 13; // 时间，小时个位，b 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 14; // 时间，小时个位，f 段指示灯
    // aip3368h_engine_speed_panel_display_buff[1] |= 0x01 << 15; // 时间，小时个位，a 段指示灯

    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 0;  // 时间，分钟个位，e 段指示灯
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 1;  // 时间，分钟个位，d 段指示灯
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 2;  // 时间，分钟个位，c 段指示灯
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 3;  // 油量 F 字样对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 4;  // 油量格数上方的顶部标记，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 5;  // 油量格数上方的顶部标记，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 6;  // 油量格数，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 7;  // 油量格数，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 8;  // 油量格数，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 9;  // 油量格数，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 10; // 油量格数，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 11; // 油量格数，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 12; // 油量格数上方的顶部标记，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 13; // 油量格数上方的顶部标记，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 14; // 油量格数上方的顶部标记，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 15; // 油量格数上方的顶部标记，第 3 个指示灯（从左往右，从0开始）

    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 0;  // 发动机转速面板分割线，第 6 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 1;  // 发动机转速面板分割线，第 7 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 2;  // 右转向对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 3;  // 远光灯（大灯）对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 4;  // 时间，分钟个位，f 段指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 5;  // 时间，分钟个位，a 段指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 6;  // 时间，分钟个位，b 段指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 7;  // 时间，分钟个位，g 段指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 8;  // 电话对应的图标（可能是表示蓝牙连接状态，实际没有该功能）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 9;  // 发动机转速面板分割线，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 10; // 挡位边框，第 0 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 11; // 挡位，d 段指示灯
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 12; // 发动机转速面板分割线，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 13; // 挡位边框，第 11 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 14; // 发动机转速面板分割线，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 15; // 挡位边框，第 10 个指示灯（从左侧的缺口处开始数，从0开始）

    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 0;  // 发动机转速，第 22 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 1;  // 发动机转速，第 23 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 2;  // 发动机转速，第 24 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 3;  // 发动机转速，第 25 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 4;  // 发动机转速，第 26 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 5;  // 发动机转速刻度，第 15 个指示灯，对应 11.5 刻度（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 6;  // 发动机转速刻度，第 16 个指示灯，对应 12 刻度（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 7;  // 发动机转速刻度，第 17 个指示灯，对应 13 刻度（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 8;  // 发动机转速刻度，第 12 个指示灯，对应 9 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 9;  // 发动机转速，第 17 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 10; // 发动机转速，第 18 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 11; // 发动机转速，第 19 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 12; // 发动机转速，第 20 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 13; // 发动机转速，第 21 格指示灯（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 14; // 发动机转速刻度，第 13 个指示灯，对应 9.5 刻度（从左往右数，从0开始，红）
    // aip3368h_engine_speed_panel_display_buff[4] |= 0x01 << 15; // 发动机转速刻度，第 14 个指示灯，对应 10 刻度（从左往右数，从0开始，红）

    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 0;  // 挡位，e 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 1;  // 挡位边框，第 2 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 2;  // 挡位，f 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 3;  // 挡位边框，第 3 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 4;  // 挡位边框，第 4 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 5;  // 挡位，a 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 6;  // 挡位边框，第 5 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 7;  // x1000RPM 字样，第 0 个指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 8;  // x1000RPM 字样，第 1 个指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 9;  // 挡位边框，第 6 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 10; // 挡位，b 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 11; // 挡位，g 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 12; // 挡位边框，第 7 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 13; // 挡位边框，第 8 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 14; // 挡位，c 段指示灯
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 15; // 挡位边框，第 9 个指示灯（从左侧的缺口处开始数，从0开始）

    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 0;  // 发动机转速刻度，第 7 个指示灯，对应 5 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 1;  // 发动机转速刻度，第 6 个指示灯，对应 4 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 2;  // 发动机转速刻度，第 5 个指示灯，对应 3.5 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 3;  // 发动机转速，第 8 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 4;  // 发动机转速，第 9 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 5;  // 发动机转速，第 10 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 6;  // 发动机转速，第 11 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 7;  // 发动机转速，第 12 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 8;  // 发动机转速，第 16 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 9;  // 发动机转速，第 15 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 10; // 发动机转速，第 14 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 11; // 发动机转速，第 13 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 12; // 发动机转速刻度，第 11 个指示灯，对应 8 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 13; // 发动机转速刻度，第 10 个指示灯，对应 7 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 14; // 发动机转速刻度，第 8 个指示灯，对应 6 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[6] |= 0x01 << 15; // 发动机转速刻度，第 9 个指示灯，对应 6.5 刻度（从左往右数，从0开始，白）

    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 0;  // 发动机转速面板分割线，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 1;  // 左转向对应的指示灯
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 2;  // 发动机转速面板分割线，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 3;  // 发动机转速刻度，第 0 个指示灯，对应 0 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 4;  // 发动机转速刻度，第 1 个指示灯，对应 0.5 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 5;  // 发动机转速，第 0 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 6;  // 发动机转速，第 1 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 7;  // 发动机转速，第 2 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 8;  // 发动机转速，第 3 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 9;  // 发动机转速，第 4 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 10; // 发动机转速，第 5 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 11; // 发动机转速刻度，第 2 个指示灯，对应 1 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 12; // 发动机转速刻度，第 3 个指示灯，对应 2 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 13; // 发动机转速，第 6 格指示灯（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 14; // 发动机转速刻度，第 4 个指示灯，对应 3 刻度（从左往右数，从0开始，白）
    // aip3368h_engine_speed_panel_display_buff[7] |= 0x01 << 15; // 发动机转速，第 7 格指示灯（从左往右数，从0开始，白）
}
#endif

#endif
