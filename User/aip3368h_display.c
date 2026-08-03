#include "aip3368h_display.h"
#include "aip3368.h"

#include "user_config.h"
#include "user_include.h"

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

/**
 * @brief 显示指定的发动机转速对应的格子（不会清空原来的显示）
 *
 * @param gear 挡位 0 ~ 26
 *      0:显示第 0 格指示灯
 *      1:显示第 1 格指示灯
 *
 */
void __aip3368h_display_engine_speed_gear__(u8 gear)
{
    aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[gear].buff_index] |=
        (0x01 << engine_speed_gear_map[gear].bit_offset);
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
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i].buff_index] &=
                ~(0x01 << engine_speed_gear_map[i].bit_offset);
        } else if ((gear - i) > 0) {
            // 如果传参的挡位，比当前遍历的挡位还要大（至少要大于等于1）
            aip3368h_engine_speed_panel_display_buff[engine_speed_gear_map[i].buff_index] |=
                (0x01 << engine_speed_gear_map[i].bit_offset);
        }
    }
}

/**
 * @brief 显示指定的发动机转速对应的刻度条（不会清空原来的显示）
 *
 * @param scale 刻度 0 ~ 17
 *
 */
void __aip3368h_display_engine_speed_scale_bar__(u8 scale)
{
    aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[scale].buff_index] |=
        (0x01 << engine_speed_scale_map[scale].bit_offset);
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
            aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[i].buff_index] &=
                ~(0x01 << engine_speed_scale_map[i].bit_offset);
        } else if ((scale - i) > 0) {
            // 如果传参的数值，比当前遍历的数值还要大（至少要大于等于1）
            aip3368h_engine_speed_panel_display_buff[engine_speed_scale_map[i].buff_index] |=
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
            aip3368h_engine_speed_panel_display_buff[gear_border_map[i].buff_index] |=
                (0x01 << gear_border_map[i].bit_offset);
        } else {
            aip3368h_engine_speed_panel_display_buff[gear_border_map[i].buff_index] &=
                ~(0x01 << gear_border_map[i].bit_offset);
        }
    }

    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 10; // 挡位边框，第 0 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[0] |= 0x01 << 13; // 挡位边框，第 1 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 1;  // 挡位边框，第 2 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 3;  // 挡位边框，第 3 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 4;  // 挡位边框，第 4 个指示灯（从左侧的缺口处开始数，从0开始）

    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 6;  // 挡位边框，第 5 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 9;  // 挡位边框，第 6 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 12; // 挡位边框，第 7 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 13; // 挡位边框，第 8 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[5] |= 0x01 << 15; // 挡位边框，第 9 个指示灯（从左侧的缺口处开始数，从0开始）

    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 15; // 挡位边框，第 10 个指示灯（从左侧的缺口处开始数，从0开始）
    // aip3368h_engine_speed_panel_display_buff[3] |= 0x01 << 13; // 挡位边框，第 11 个指示灯（从左侧的缺口处开始数，从0开始）
}

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
    aip3368h_display_gear_border(is_display);

    is_display = !is_display;
}

void aip3368h_display_test(void)
{
#if 1
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
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 4;  // 油量格数上方的刻度条，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 5;  // 油量格数上方的刻度条，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 6;  // 油量格数，第 5 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 7;  // 油量格数，第 4 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 8;  // 油量格数，第 3 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 9;  // 油量格数，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 10; // 油量格数，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 11; // 油量格数，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 12; // 油量格数上方的刻度条，第 0 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 13; // 油量格数上方的刻度条，第 1 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 14; // 油量格数上方的刻度条，第 2 个指示灯（从左往右，从0开始）
    // aip3368h_engine_speed_panel_display_buff[2] |= 0x01 << 15; // 油量格数上方的刻度条，第 3 个指示灯（从左往右，从0开始）

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
#endif

// 所有灯整体闪烁
#if 0
    static u8 dir = 0;
    static u16 cnt = 0;
    cnt++;

    if (cnt >= 500)
    {
        cnt = 0;
    }
    else
    {
        return;
    }

    if (dir == 0)
    {
        memset(aip3368h_display_buff, 0x00, sizeof(aip3368h_display_buff));
        dir = 1;
    }
    else
    {
        memset(aip3368h_display_buff, (u8)0xFF, sizeof(aip3368h_display_buff));
        dir = 0;
    }
#endif
}