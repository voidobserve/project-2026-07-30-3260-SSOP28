#ifndef __AIP3368H_DISPLAY_H__
#define __AIP3368H_DISPLAY_H__

// #include "user_config.h"
#include "instrument.h"
#include "typedef.h"

#define AIP3368H_DISPLAY_TEST_ENABLE 1

// 用于建立指示灯和显存的映射关系：
typedef struct
{
    // display_buff[] 中对应元素索引
    u8 buff_index;
    // display_buff[] 中对应元素中的第 x 位（按二进制数的排列方式，从右往左数）
    u8 bit_offset;
} aip3368h_display_mapping_t;

// 数码管 A ~ G 段索引值
enum
{
    SEG_IDX_A = 0x00,
    SEG_IDX_B,
    SEG_IDX_C,
    SEG_IDX_D,
    SEG_IDX_E,
    SEG_IDX_F,
    SEG_IDX_G,
};
typedef u8 seg_idx_t;
 
void __aip3368h_display_engine_speed_gear__(u8 gear); 
void aip3368h_display_engine_speed_gear(u8 gear); 
void __aip3368h_display_engine_speed_scale_bar__(u8 scale); 
void aip3368h_display_engine_speed_scale_bar(u8 scale);

void aip3368h_display_x1000rpm_light(u8 is_display);
void aip3368h_display_gear_border(u8 is_display);

#if AIP3368H_DISPLAY_TEST_ENABLE

void aip3368h_display_engine_speed_gear_test_1ms_isr(void);
void aip3368h_display_engine_speed_scale_test_1ms_isr(void);

void aip3368h_display_light_blink_test_1ms_isr(void);
void aip3368h_display_test(void);

#endif

#endif