#ifndef __AIP3368H_DISPLAY_H__
#define __AIP3368H_DISPLAY_H__

// #include "user_include.h"
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

enum
{
    MILEAGE_UNIT_TYPE_KM = 0x00,
    MILEAGE_UNIT_TYPE_MILES,
};
typedef u8 mileage_unit_t;

enum
{
    MILEAGE_DISPLAY_MODE_ODO = 0x00,
    MILEAGE_DISPLAY_MODE_TRIP,
};
typedef u8 mileage_display_mode_t;

void __aip3368h_display_engine_speed_gear__(u8 idx, u8 is_display);
void aip3368h_display_engine_speed_gear(u8 gear);
void __aip3368h_display_engine_speed_scale_bar__(u8 idx, u8 is_display);
void aip3368h_display_engine_speed_scale_bar(u8 is_display);

void aip3368h_display_x1000rpm_light(u8 is_display);
void aip3368h_display_gear_border(u8 is_display);

void __aip3368h_display_gear_seg__(seg_idx_t seg_idx);
void __aip3368h_display_gear_num__(u8 num);
void aip3368h_display_gear(u8 gear);

void __aip3368h_display_engine_speed_split_line_light__(u8 idx, u8 is_display);

void aip3368h_display_left_turn_light(u8 is_display);
void aip3368h_display_engine_fault_light(u8 is_display);
void aip3368h_display_n_light(u8 is_display);
void aip3368h_display_phone_light(u8 is_display);
void aip3368h_display_high_beam_light(u8 is_display);
void aip3368h_display_right_turn_light(u8 is_display);

void __aip3368h_display_hour_digit__(u8 bit_x, u8 num);
void __aip3368h_display_minute_digit__(u8 bit_x, u8 num);
void aip3368h_display_time_colon_light(u8 is_display);
void aip3368h_display_time(u8 hour, u8 minute);
void aip3368h_display_time_digits_when_setting(u8 is_display, u8 num,
                                               u8 is_setting_min);                                               

void aip3368h_display_fuel_empty_light(u8 is_display);
void aip3368h_display_fuel_icon_light(u8 is_display);
void aip3368h_display_fuel_full_light(u8 is_display);

void __aip3368h_display_fuel_lev_upper_marker__(u8 idx, u8 is_display);
#if 0
void aip3368h_display_fuel_lev_upper_marker(u8 is_display);
#endif

void __aip3368h_display_fuel_lev__(u8 idx, u8 is_display);
void aip3368h_display_fuel_lev(u8 lev);

#if 0
void __aip3368h_display_speed_seg__(u8 bit_x, seg_idx_t seg_idx, u8 is_display);
#endif
void aip3368h_display_speed_bit(u8 bit_x, u8 num, u8 is_display);
void aip3368h_display_speed(u16 speed);

void aip3368h_display_mph_light(u8 is_display);
void aip3368h_display_kmh_light(u8 is_display);

void __aip3368h_display_speed_split_line_light__(u8 idx, u8 is_display);
#if 0
void aip3368h_display_speed_split_line_light(u8 is_display);
#endif

void aip3368h_display_trip_light(u8 is_display);
void aip3368h_display_odo_light(u8 is_display);
void aip3368h_display_miles_light(u8 is_display);
void aip3368h_display_km_light(u8 is_display);

void __aip3368h_display_mileage_bit_x__(u8 bit_x, u8 num, u8 is_display);
void aip3368h_display_mileage(u32 mileage,
                              mileage_display_mode_t mileage_display_mode);

void aip3368h_display_battery_8_symbol_light(u8 is_display);
void aip3368h_display_battery_icon_light(u8 is_display);
void aip3368h_display_battery_16_symbol_light(u8 is_display);

void __aip3368h_display_bat_lev_upper_marker_light__(u8 idx, u8 is_display);
#if 0
void aip3368h_display_bat_lev_upper_marker_light(u8 is_display);
#endif

void __aip3368h_display_bat_lev_light__(u8 idx, u8 is_display);
void aip3368h_display_bat_lev_light(u8 bat_lev);

#if AIP3368H_DISPLAY_TEST_ENABLE

#if 0
void aip3368h_display_engine_speed_gear_test_1ms_isr(void);
void aip3368h_display_engine_speed_scale_test_1ms_isr(void);
void aip3368h_display_gear_test_1ms_isr(void);
void aip3368h_display_engine_speed_split_line_test_1ms_isr(void);
void aip3368h_display_time_test_1ms_isr(void);
void aip3368h_display_fuel_lev_upper_marker_test_1ms_isr(void);
void aip3368h_display_fuel_lev_test_1ms_isr(void);
void aip3368h_display_speed_test_1ms_isr(void);
void aip3368h_display_speed_split_line_light_test_1ms_isr(void);
void aip3368h_display_mileage_test_1ms_isr(void);
void aip3368h_display_bat_lev_upper_marker_light_test_1ms_isr(void);
void aip3368h_display_bat_lev_light_test_1ms_isr(void);
void aip3368h_display_light_blink_test_1ms_isr(void);
#endif

#if 0
void aip3368h_display_test(void);
#endif

#endif

#endif