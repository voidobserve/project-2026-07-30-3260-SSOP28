#ifndef __BOOT_ANIMATION_H__
#define __BOOT_ANIMATION_H__

#include "typedef.h"

/*
    开机动画：
*/
enum
{
    BOOT_ANIMATION_PAHSE_0 = 0x00, 
    BOOT_ANIMATION_PAHSE_1,             
    BOOT_ANIMATION_PAHSE_2,             
    BOOT_ANIMATION_PAHSE_3,             
    BOOT_ANIMATION_PAHSE_4,             

    BOOT_ANIMATION_PAHSE_END, // 结束
};
typedef u8 boot_animation_phase_t;

typedef struct
{
    u8 is_in_boot_animation; // 是否处于开机动画中
    u8 time_base_cnt;        // 时间基准

    u16 total_time_cnt; // 记录整个开机动画的时间
    u16 phase_time_cnt; // 记录当前动画步骤的时间

    boot_animation_phase_t animation_phase; // 当前动画步骤
} boot_animation_param_t;

void boot_animation_time_base_add_1ms_isr(void);
void boot_animation_process(void);

#endif