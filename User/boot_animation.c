#include "boot_animation.h"
#include "aip3368.h"
#include "aip3368h_display.h"
#include "include.h"
#include <string.h> // memset

#include "user_config.h"

volatile boot_animation_param_t boot_animation_param;

void boot_animation_time_base_add_1ms_isr(void)
{
    if (0 == boot_animation_param.is_in_boot_animation) {
        return;
    }

    boot_animation_param.time_base_cnt += 1;
}

void __boot_animation_phase_0__(void)
{
/*
    控制动画的时间间隔
    单个方向时，动画持续约1秒，有8个指示灯，将1秒分给8个指示灯用作时间间隔
*/
#define BOOT_ANIMATION_PHASE_0_PERIOD_OF_SPEED_SPLIT_LINE_LIGHT                \
    ((u16)1000 / (8 * 3))
// 发动机转速刻度条对应的动画时间间隔
#define BOOT_ANIMATION_PHASE_0_PERIOD_OF_ENGINE_SPEED_SCALE_BAR                \
    ((u16)1000 / (18 * 3))
// 油量格数上方的顶部标记的动画时间间隔
#define BOOT_ANIMATION_PHASE_0_PERIOD_OF_FUEL_UPPER_MARKER ((u16)1000 / (5 * 3))

    static volatile u8 dir_of_speed_panel = 0;        // 动画方向
    static volatile u8 dir_of_engine_speed_panel = 0; // 动画方向

    static volatile u8 idx_of_speed_panel =
        0; // 指示灯的索引值（在时速面板上使用）
    static volatile u8 idx_of_engine_speed_panel =
        0; // 指示灯的索引值（在发动机面板上使用）

    static volatile u8 step_of_speed_panel = 0;        // 当前xx的动画步骤
    static volatile u8 step_of_engine_speed_panel = 0; // 当前xx的动画步骤

    if (boot_animation_param.animation_phase != BOOT_ANIMATION_PHASE_0) {
        return;
    }

#if USER_DEBUG_ENABLE
    // printf("idx_of_speed_panel == %u\n", (u16)idx_of_speed_panel);
    // printf("step_of_speed_panel == %u\n", (u16)step_of_speed_panel);
#endif

    /*
        动画阶段：
        1. dir == 0，idx 从 0 加到 8 * 2
        2. dir 刚从 0 变为 1，idx 为 8 * 2，此时不显示
        3. dir == 1，idx 从 8 * 2 减到 0
        4. dir == 1，idx == 0，此时不显示
    */
    if (idx_of_speed_panel == 8 * 2) {
        /*
            刚进入时，
            dir 刚从 0 变为 1
            idx_of_speed_panel == 8 * 2，此时不显示
            在这里插入动画时间间隔
        */
        __aip3368h_display_speed_split_line_light__(8 - 1, 0);

        step_of_speed_panel++;
        if (step_of_speed_panel >= 255) {
            step_of_speed_panel = 0;
            idx_of_speed_panel--;
        }
    } else if (1 == dir_of_speed_panel && idx_of_speed_panel == 0) {
        /*
            dir == 1，idx_of_speed_panel == 0
            此时动画临近结束，要熄灭第 0 个指示灯
        */
        __aip3368h_display_speed_split_line_light__(0, 0);
        // boot_animation_param.animation_phase = BOOT_ANIMATION_PHASE_END;
    } else {
        step_of_speed_panel++;
        if (step_of_speed_panel >=
            BOOT_ANIMATION_PHASE_0_PERIOD_OF_SPEED_SPLIT_LINE_LIGHT) {
            step_of_speed_panel = 0;
            if (0 == dir_of_speed_panel) {
                // 时速面板上的分割线，7个灯组成流水，不堆积

                if (idx_of_speed_panel < 8) {
                    __aip3368h_display_speed_split_line_light__(
                        idx_of_speed_panel, 1);
                } else {
                    __aip3368h_display_speed_split_line_light__(
                        idx_of_speed_panel - 8, 0);
                }

                idx_of_speed_panel++;
                if (idx_of_speed_panel >= 8 * 2) {
                    dir_of_speed_panel = 1;
                }
            } else {
                if (idx_of_speed_panel >= 8) {
                    __aip3368h_display_speed_split_line_light__(
                        idx_of_speed_panel - 8, 1);
                } else {
                    __aip3368h_display_speed_split_line_light__(
                        idx_of_speed_panel, 0);
                }

                idx_of_speed_panel--;
            }
        }
    }

    if (idx_of_engine_speed_panel == 18 * 2) {
        /*
            dir 刚从 0 变为 1
            idx_of_engine_speed_panel == 18 * 2，此时不显示
            在这里插入动画时间间隔
        */
        __aip3368h_display_engine_speed_scale_bar__(18 - 1, 0);

        step_of_engine_speed_panel++;
        if (step_of_engine_speed_panel >= 255) {
            step_of_engine_speed_panel = 0;
            /*
                油量格数上方的顶部标记，最后一个指示灯的索引值 == 5
            */
            idx_of_engine_speed_panel = 5 * 2;
        }
    } else if (1 == dir_of_engine_speed_panel &&
               idx_of_engine_speed_panel == 0) {
        /*
            dir == 1，idx_of_engine_speed_panel == 0
        */
        __aip3368h_display_fuel_lev_upper_marker__(0, 0);
        boot_animation_param.animation_phase = BOOT_ANIMATION_PHASE_1;
    } else {
        step_of_engine_speed_panel++;

        if (0 == dir_of_engine_speed_panel &&
            (step_of_engine_speed_panel >=
             BOOT_ANIMATION_PHASE_0_PERIOD_OF_ENGINE_SPEED_SCALE_BAR)) {
            step_of_engine_speed_panel = 0;

            // 发动机转速的刻度线，流水动画，不堆积
            if (idx_of_engine_speed_panel < 18) {
                __aip3368h_display_engine_speed_scale_bar__(
                    idx_of_engine_speed_panel, 1);
            } else {
                __aip3368h_display_engine_speed_scale_bar__(
                    idx_of_engine_speed_panel - 18, 0);
            }

            idx_of_engine_speed_panel++;
            if (idx_of_engine_speed_panel >= 18 * 2) {
                dir_of_engine_speed_panel = 1;
            }
        } else if (step_of_engine_speed_panel >=
                   BOOT_ANIMATION_PHASE_0_PERIOD_OF_FUEL_UPPER_MARKER) {
            step_of_engine_speed_panel = 0;
            if (idx_of_engine_speed_panel >= 6) {
                __aip3368h_display_fuel_lev_upper_marker__(
                    idx_of_engine_speed_panel - 5, 1);
            } else {
                __aip3368h_display_fuel_lev_upper_marker__(
                    idx_of_engine_speed_panel, 0);
            }

            idx_of_engine_speed_panel--;
        }
    }
}

void __boot_animation_phase_1__(void)
{
    // 控制数码管切换显示数字的时间间隔
#define BOOT_ANIMATION_PHASE_1_PERIOD_OF_NUM 150
    /*
        控制发动机转速挡位的动画时间间隔
    */
#define BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_GEAR (1000 / 27)
    /*
        控制发动机转速刻度线的动画时间间隔
    */
#define BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_SCALE_BAR (1000 / 18)

/*
    控制发动机转速挡位的动画时间间隔，只在挡位下降的过程中使用
*/
#define BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_GEAR_DOWN                \
    (1000 / (27 * 3))

    static u8 is_initialized = 0;
    static volatile u8 num = 0;
    static volatile u8 step = 0;

    static volatile u8 step_of_engine_speed_gear =
        0; // 发动机转速挡位对应的动画步骤
    static volatile u8 step_of_engine_speed_scale_bar =
        0;                                     // 发动机转速刻度线对应的动画步骤
    static volatile u8 step_of_exit_phase = 0; // 退出当前动画阶段的步骤

    static volatile u8 idx_of_bat_lev = 0;
    static volatile u8 idx_of_scale_bar_light = 0; // 分割线对应的指示灯索引值
    static volatile u8 idx_of_upper_marker = 0;

    static volatile u8 idx_of_fuel_lev = 0; // 油量格数对应的指示灯索引值
    static volatile u8 idx_of_engine_speed_gear = 0;
    static volatile u8 idx_of_engine_speed_scale_bar =
        0; // 发动机转速刻度线对应的指示灯索引值

    static volatile u8 dir_of_engine_speed_gear =
        0; // 发动机转速挡位动画的方向，0：上升，1：下降

    if (boot_animation_param.animation_phase != BOOT_ANIMATION_PHASE_1) {
        return;
    }

    if (0 == is_initialized) {
        is_initialized = 1;
        idx_of_scale_bar_light = 7;
        idx_of_engine_speed_scale_bar = 17;

        aip3368h_display_mph_light(1);
        aip3368h_display_kmh_light(1);
        aip3368h_display_trip_light(1);
        aip3368h_display_odo_light(1);
        aip3368h_display_miles_light(1);
        aip3368h_display_km_light(1);
        aip3368h_display_battery_8_symbol_light(1);
        aip3368h_display_battery_icon_light(1);
        aip3368h_display_battery_16_symbol_light(1);

        aip3368h_display_left_turn_light(1);
        aip3368h_display_engine_fault_light(1);
        aip3368h_display_n_light(1);
        aip3368h_display_phone_light(1);
        aip3368h_display_high_beam_light(1);
        aip3368h_display_right_turn_light(1);

        aip3368h_display_time_colon_light(1);

        aip3368h_display_fuel_empty_light(1);
        aip3368h_display_fuel_icon_light(1);
        aip3368h_display_fuel_full_light(1);

        aip3368h_display_x1000rpm_light(1);
        aip3368h_display_gear_border(1);
    }

    step++;
    if (step >= BOOT_ANIMATION_PHASE_1_PERIOD_OF_NUM) {
        step = 0;
        aip3368h_display_speed_bit(0, num, 1);
        aip3368h_display_speed_bit(1, num, 1);
        aip3368h_display_speed_bit(2, num, 1);

        __aip3368h_display_mileage_bit_x__(0, num, 1);
        __aip3368h_display_mileage_bit_x__(1, num, 1);
        __aip3368h_display_mileage_bit_x__(2, num, 1);
        __aip3368h_display_mileage_bit_x__(3, num, 1);
        __aip3368h_display_mileage_bit_x__(4, num, 1);
        __aip3368h_display_mileage_bit_x__(5, num, 1);

        __aip3368h_display_bat_lev_light__(idx_of_bat_lev, 1);

        __aip3368h_display_gear_num__(num);
        __aip3368h_display_hour_digit__(0, num);
        __aip3368h_display_hour_digit__(1, num);
        __aip3368h_display_minute_digit__(0, num);
        __aip3368h_display_minute_digit__(1, num);

        __aip3368h_display_engine_speed_split_line_light__(
            idx_of_scale_bar_light, 1);
        __aip3368h_display_speed_split_line_light__(idx_of_scale_bar_light, 1);

        __aip3368h_display_fuel_lev_upper_marker__(idx_of_upper_marker, 1);
        __aip3368h_display_bat_lev_upper_marker_light__(idx_of_upper_marker, 1);

        __aip3368h_display_fuel_lev__(idx_of_fuel_lev, 1);

        if (num < 9) {
            num++;
        }

        if (idx_of_bat_lev < 8) {

            idx_of_bat_lev++;
        }

        if (idx_of_scale_bar_light > 0) {
            idx_of_scale_bar_light--;
        }

        if (idx_of_upper_marker < 5) {
            idx_of_upper_marker++;
        }

        if (idx_of_fuel_lev < 5) {
            idx_of_fuel_lev++;
        }
    }

    step_of_engine_speed_gear++;
    if (dir_of_engine_speed_gear == 0 && idx_of_engine_speed_gear == 26) {
        /*
            发动机转速挡位刚到达上限，保持显示一段时间，再切换方向
        */
        __aip3368h_display_engine_speed_gear__(26, 1);
        if (step_of_engine_speed_gear >= 255) {
            dir_of_engine_speed_gear = 1;
            step_of_engine_speed_gear = 0;
        }
    } else if (0 == dir_of_engine_speed_gear &&
               step_of_engine_speed_gear >=
                   BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_GEAR) {
        step_of_engine_speed_gear = 0;

        __aip3368h_display_engine_speed_gear__(idx_of_engine_speed_gear, 1);

        if (idx_of_engine_speed_gear < 26) {
            idx_of_engine_speed_gear++;
        }
    } else if (dir_of_engine_speed_gear == 1) {
        if (step_of_engine_speed_gear >=
            BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_GEAR_DOWN) {
            step_of_engine_speed_gear = 0;
            __aip3368h_display_engine_speed_gear__(idx_of_engine_speed_gear, 0);

            if (idx_of_engine_speed_gear > 0) {
                idx_of_engine_speed_gear--;
            }
        }
    }

    step_of_engine_speed_scale_bar++;
    if (step_of_engine_speed_scale_bar >=
        BOOT_ANIMATION_PHASE_1_PERIOD_OF_ENGINE_SPEED_SCALE_BAR) {
        step_of_engine_speed_scale_bar = 0;

        __aip3368h_display_engine_speed_scale_bar__(
            idx_of_engine_speed_scale_bar, 1);

        if (idx_of_engine_speed_scale_bar > 0) {
            idx_of_engine_speed_scale_bar--;
        }
    }

    // 判断所有子分区的动画是否都结束，再退出该动画阶段
    if (dir_of_engine_speed_gear == 1 && idx_of_engine_speed_gear == 0 &&
        /*  */
        idx_of_engine_speed_scale_bar == 0) {
        step_of_exit_phase++;
        if (step_of_exit_phase >= 255) {

            boot_animation_param.animation_phase = BOOT_ANIMATION_PHASE_END;
        }
    }
}

void boot_animation_process(void)
{
    memset(&boot_animation_param, 0x00, sizeof(boot_animation_param_t));
    boot_animation_param.is_in_boot_animation = 1;

    while (boot_animation_param.is_in_boot_animation) {
        WDT_KEY = WDT_KEY_VAL(0xAA); // 喂狗并清除 wdt_pending

        if (boot_animation_param.time_base_cnt < 1) {
            // 时间未到来
            continue;
        } else {
            boot_animation_param.time_base_cnt = 0;
        }

        __boot_animation_phase_0__();
        __boot_animation_phase_1__();

        if (BOOT_ANIMATION_PHASE_END == boot_animation_param.animation_phase) {
            // 开机动画结束，根据存储的参数，立即更新显示
            // 可以交给后续的功能来更新显示，不用在这里立即显示
            boot_animation_param.is_in_boot_animation = 0;
        }

        aip3368h_module_display(); // 更新显示
    }
}
