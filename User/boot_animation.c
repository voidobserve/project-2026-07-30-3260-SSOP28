#include "boot_animation.h"
#include "aip3368h_display.h"
#include "include.h"
#include "string.h" // memset

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
// 控制动画的时间间隔
#define BOOT_ANIMATION_PAHSE_0_PERIOD_OF_SPEED_SPLIT_LINE_LIGHT                \
    ((u16)1000 / (8 * 3))
// 发动机转速刻度条对应的动画时间间隔
#define BOOT_ANIMATION_PHASE_0_PERIOD_OF_ENGINE_SPEED_SCALE_BAR                \
    ((u16)1000 / (18 * 3))

    // static u8 is_initialized = 0;
    static volatile u8 dir = 0; // 动画方向

    static volatile u8 idx_of_speed_split_line_light = 0;
    static volatile u8 idx_of_engine_speed_scale_bar = 0;

    static volatile u8 step_of_speed_split_line_light = 0; // 当前xx的动画步骤
    static volatile u8 step_of_engine_speed_scale_bar = 0; // 当前xx的动画步骤

    if (boot_animation_param.animation_phase != BOOT_ANIMATION_PAHSE_0) {
        return;
    }

    // if (!is_initialized) {
    //     is_initialized = 1;
    //     // step = BOOT_ANIMATION_PAHSE_0_PERIOD;
    // }

    step_of_speed_split_line_light++;
    if (step_of_speed_split_line_light >= BOOT_ANIMATION_PAHSE_0_PERIOD_OF_SPEED_SPLIT_LINE_LIGHT) {
        step_of_speed_split_line_light = 0;
        if (0 == dir) {
            // 时速面板上的分割线，7个灯组成流水，不堆积

            if (idx_of_speed_split_line_light < 8) {
                __aip3368h_display_speed_split_line_light__(idx_of_speed_split_line_light, 1);
            } else {
                __aip3368h_display_speed_split_line_light__(idx_of_speed_split_line_light - 8, 0);
            }

            idx_of_speed_split_line_light++;
            if (idx_of_speed_split_line_light >= 8 * 2) {
                dir = 1;
            }
        } else {

            // TODO 有越界的情况
            if (idx_of_speed_split_line_light >= 8) {
                __aip3368h_display_speed_split_line_light__(idx_of_speed_split_line_light - 8, 1);
            } else {
                __aip3368h_display_speed_split_line_light__(idx_of_speed_split_line_light, 0);
            }

            idx_of_speed_split_line_light--;
            if (idx_of_speed_split_line_light == 0) {
                boot_animation_param.animation_phase = BOOT_ANIMATION_PAHSE_END;
            }
        }
    }

    step_of_engine_speed_scale_bar++;
    if (step_of_engine_speed_scale_bar >= BOOT_ANIMATION_PHASE_0_PERIOD_OF_ENGINE_SPEED_SCALE_BAR) {
        step_of_engine_speed_scale_bar = 0;
        if (0 == dir) {
            //
            if (idx_of_engine_speed_scale_bar < 18) {
                __aip3368h_display_engine_speed_scale_bar__(idx_of_engine_speed_scale_bar, 1);
            } else {
                __aip3368h_display_engine_speed_scale_bar__(idx_of_engine_speed_scale_bar - 18, 0);
            }

            idx_of_engine_speed_scale_bar++;
            // if (idx_of_engine_speed_scale_bar >= 18 * 2) {
            //     boot_animation_param.animation_phase = BOOT_ANIMATION_PAHSE_END;
            // }

        } else {
        }
    }
}

void __boot_animation_phase_1__(void)
{
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

        // TODO
        __boot_animation_phase_0__();

        aip3368h_module_display(); // 更新显示
    }
}
