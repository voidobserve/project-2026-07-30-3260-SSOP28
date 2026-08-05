#include "aip3368.h"

#include <string.h> // memset
#include "include.h"

#include "user_config.h" // USER_DEBUG_ENABLE

#if 1 // AIP3368H_MODULE

static volatile u16 aip3368h_refresh_cnt = 0;

// 时速面板对应的显存
volatile u16 aip3368h_speed_panel_display_buff[AIP3368H_SPEED_PANEL_IC_NUM] = {0};

// 发动机转速面板对应的显存
volatile u16 aip3368h_engine_speed_panel_display_buff[AIP3368H_ENGINE_SPEED_PANEL_IC_NUM] = {0};

// 放在1ms的定时器中
void aip3368h_refresh_time_add(void)
{
    // 防止计数溢出
    if (aip3368h_refresh_cnt < ((u16)-1))
    {
        aip3368h_refresh_cnt++;
    }
}

#if 0
/**
 * @brief 将数据发送给芯片
 *      由于两组级联的芯片共用时钟线，这里要将两组芯片的数据一起发送
 *
 */
static void aip3368h_module_send_data_to_one_dev(u16 dat_group_1, u16 dat_group_2)
{
    volatile u16 i;
    // PDM = 1;

    for (i = 0; i < 16; i++)
    {
        DIO_GROUP_1 = dat_group_1 & (u16)0x8000 ? 1 : 0;
        DIO_GROUP_2 = dat_group_2 & (u16)0x8000 ? 1 : 0;

        aip3368h_delay();
        DCK = 1;
        aip3368h_delay();
        dat_group_1 <<= 1;
        dat_group_2 <<= 1;
        DCK = 0;
        aip3368h_delay();
    }
    // PDM = 0;
}
#endif

static void aip3368h_module_send_data_to_all_dev(
    const u16 *buff_group_1,
    const u8 len_group_1,
    const u16 *buff_group_2,
    const u8 len_group_2)
{
    volatile u8 i;
    volatile u8 j;
    volatile u16 dat_group_1;
    volatile u16 dat_group_2;

    /*
        两组级联（时速面板 6 颗、发动机转速面板 8 颗）共用 DCK 和 LAT，
        因此一帧的时钟数必须等于最大的级联数（AIP3368_MAX_IC_NUM = 8 个字）。

        移位寄存器型级联：先发的数据会填充到最远端的芯片，
        发多了的字会从级联链末端移出而丢失（不是“截取前一段再转发”）。
        所以级联数较少的一组，数据必须从帧的“末尾”开始发送：
        字偏移 offset = AIP3368_MAX_IC_NUM - 本组级联数。
        时速面板 offset = 8 - 6 = 2，其 6 个字占帧的第 2~7 个位置，
        恰好落在 6 颗芯片上，前面 2 个字补 0（被移出丢失，无影响）。
        发动机转速面板 offset = 0，8 个字正好占满整帧。
    */
    volatile u8 offset_group_1 = AIP3368_MAX_IC_NUM - len_group_1;
    volatile u8 offset_group_2 = AIP3368_MAX_IC_NUM - len_group_2;

    // 开始
    DCK = 0;
    LAT = 0;
    aip3368h_delay();

    // 一帧完整数据
    for (i = 0; i < AIP3368_MAX_IC_NUM; i++)
    {
        // 本组数据在帧中的位置：i < offset 的部分补 0（会被移出级联链）
        dat_group_1 = 0;
        dat_group_2 = 0;

        if (i >= offset_group_1)
        {
            dat_group_1 = buff_group_1[i - offset_group_1];
        }

        if (i >= offset_group_2)
        {
            dat_group_2 = buff_group_2[i - offset_group_2];
        }

        for (j = 0; j < 16; j++)
        {
            DIO_GROUP_1 = dat_group_1 & (u16)0x8000 ? 1 : 0;
            DIO_GROUP_2 = dat_group_2 & (u16)0x8000 ? 1 : 0;

            aip3368h_delay();
            DCK = 1;
            aip3368h_delay();

            dat_group_1 <<= 1;
            dat_group_2 <<= 1;

            DCK = 0;
            aip3368h_delay();
        }
    }

    // 结束
    LAT = 1;
    aip3368h_delay();
    LAT = 0;
    aip3368h_delay();
    DIO_GROUP_1 = 0;
    DIO_GROUP_2 = 0;
}

#define AIP3368H_FLASH_TEST_ENABLE 0

// 根据显存中的数据，更新显示
void aip3368h_module_display(void)
{
    // 刷新间隔 单位：ms
    if (aip3368h_refresh_cnt < 25)
    // if (aip3368h_refresh_cnt < 500)
    {
        return;
    }
    else
    {
        aip3368h_refresh_cnt = 0;
    }

#if USER_DEBUG_ENABLE
// printf("aip3368h_module_display\n");
#endif

#if AIP3368H_FLASH_TEST_ENABLE

    // printf("aip3368h_speed_panel_display_buff[0] == 0x%04x\n",
    //        aip3368h_speed_panel_display_buff[0]);

    // 闪烁测试
    if (aip3368h_speed_panel_display_buff[0] == (u16)0x0000)
    {
        memset(aip3368h_speed_panel_display_buff, 0xFF, sizeof(aip3368h_speed_panel_display_buff));
        memset(aip3368h_engine_speed_panel_display_buff, 0xFF, sizeof(aip3368h_engine_speed_panel_display_buff));
    }
    else
    {
        memset(aip3368h_speed_panel_display_buff, 0x00, sizeof(aip3368h_speed_panel_display_buff));
        memset(aip3368h_engine_speed_panel_display_buff, 0x00, sizeof(aip3368h_engine_speed_panel_display_buff));
    }

#endif

    aip3368h_module_send_data_to_all_dev(
        aip3368h_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM,
        aip3368h_engine_speed_panel_display_buff, AIP3368H_ENGINE_SPEED_PANEL_IC_NUM);
}

void aip3368h_module_init(void)
{
    // 显示驱动芯片有记忆功能（数据锁存），每次上电应该清空显存
    memset(aip3368h_speed_panel_display_buff, 0x00, sizeof(aip3368h_speed_panel_display_buff));
    memset(aip3368h_engine_speed_panel_display_buff, 0x00, sizeof(aip3368h_engine_speed_panel_display_buff));

    // DCK
    P1_MD0 &= ~GPIO_P11_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P11_MODE_SEL(0x01);
    FOUT_S11 = GPIO_FOUT_AF_FUNC;
    // DIO
    P0_MD0 &= ~GPIO_P03_MODE_SEL(0x03);
    P0_MD0 |= GPIO_P03_MODE_SEL(0x01);
    FOUT_S03 = GPIO_FOUT_AF_FUNC;

    P1_MD1 &= ~GPIO_P15_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P15_MODE_SEL(0x01);
    FOUT_S15 = GPIO_FOUT_AF_FUNC;
    // LAT
    P1_MD0 &= ~GPIO_P12_MODE_SEL(0x03);
    P1_MD0 |= GPIO_P12_MODE_SEL(0x01);
    FOUT_S12 = GPIO_FOUT_AF_FUNC;
    // PDM
    // PDM 改成 PWM 驱动
    P1_MD1 &= ~GPIO_P16_MODE_SEL(0x03);
    P1_MD1 |= GPIO_P16_MODE_SEL(0x01);
    FOUT_S16 = GPIO_FOUT_STMR0_PWMOUT; // 选择stmr0_pwmout

#define STMR0_PEROID_VAL (SYSCLK / 1 / 1000 - 1) // 周期值=系统时钟/分频/频率 - 1
    // STIMER0配置1kHz PWM
    STMR0_PSC = STMR_PRESCALE_VAL(0x00);                        // 不分频
    STMR0_PRH = STMR_PRD_VAL_H((STMR0_PEROID_VAL >> 8) & 0xFF); // 周期高八位寄存器
    STMR0_PRL = STMR_PRD_VAL_L((STMR0_PEROID_VAL >> 0) & 0xFF); // 周期低八位寄存器
    STMR0_CMPAH = STMR_CMPA_VAL_H(((0) >> 8) & 0xFF);           // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((0) >> 0) & 0xFF);           // 比较值

    STMR_PWMVALA = STMR_0_PWMVALA(0x00); // PWM输出值
    STMR_PWMEN |= STMR_0_PWM_EN(0x1);    // PWM输出使能
    STMR_CNTMD |= STMR_0_CNT_MODE(0x1);  // 选择连续计数模式
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);  // 自动装载使能
    STMR_CNTCLR |= STMR_0_CNT_CLR(0x1);  // 计数清零
    STMR_CNTEN |= STMR_0_CNT_EN(0x1);    // 计数使能

    DIO_GROUP_1 = 0;
    DIO_GROUP_2 = 0;
    DCK = 0;
    LAT = 0;
    // PDM = 0; // 由上电稳定之后设置亮度
    aip3368h_module_send_data_to_all_dev(
        aip3368h_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM,
        aip3368h_engine_speed_panel_display_buff, AIP3368H_SPEED_PANEL_IC_NUM);

    // TEST ONLY 样机使用最高亮度，实际使用时根据需要调整
    aip3368h_module_set_brightness(30);
}

/**
 * @brief 设置显示的亮度
 *
 * @param brightness 亮度值 0 ~ 100
 *          数值越大，亮度越高
 *
 */
void aip3368h_module_set_brightness(u8 brightness)
{
    // PDM脚是低电平使能，因此占空比越低，亮度越高
    u16 channel_duty = (u32)(100 - brightness) * STMR0_PEROID_VAL / 100;

    STMR0_CMPAH = STMR_CMPA_VAL_H(((channel_duty) >> 8) & 0xFF); // 比较值
    STMR0_CMPAL = STMR_CMPA_VAL_L(((channel_duty) >> 0) & 0xFF); // 比较值
    STMR_LOADEN |= STMR_0_LOAD_EN(0x1);                          // 自动装载使能
}

// void aip3368h_module_uninit(void)
// {
//     aip3368h_module_clear();    // 清屏
//     DIO = 0;
//     DCK = 0;
//     LAT = 0;
// }

#endif
