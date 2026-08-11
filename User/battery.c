#include "battery.h"
#include "user_include.h"
#include "aip3368h_display.h"
#include "instrument.h"

#if BATTERY_SCAN_ENABLE

// 电池电压扫描时间计时（在定时器中累加）
static volatile u16 battery_scan_time_cnt = 0;
// 电池电压格子数更新的时间计时
static volatile u16 bat_grid_update_time_cnt = 0; //

// 滑动平均：
#define SAMPLE_COUNT 20 // 样本计数
static volatile u16 bat_adc_val_samples[SAMPLE_COUNT] = {0};
static volatile u8 bat_adc_val_sample_index = 0;

// 电池电压值的滑动平均
#define BAT_VOL_SAMPLE_COUNT 20 // 样本计数
static volatile u16 bat_vol_samples[BAT_VOL_SAMPLE_COUNT] = {0};
static volatile u8 bat_vol_sample_index = 0;

// 初始化 滑动平均 数组
static void __bat_adv_val_samples_init__(u16 adc_val)
{
    u8 i;
    for (i = 0; i < SAMPLE_COUNT; i++) {
        bat_adc_val_samples[i] = adc_val;
    }
}

/**
 * @brief 将数据放入滑动平均数组，由adc中断更新
 *
 * @param adc_val
 *
 */
void bat_adc_val_samples_update(u16 adc_val)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized) {
        is_initialized = 1;
        __bat_adv_val_samples_init__(adc_val);
        return; // 初始化数组之后，直接退出，下一次得到新数据才执行下面的操作
    }

    bat_adc_val_samples[bat_adc_val_sample_index] = adc_val;
    bat_adc_val_sample_index++;
    if (bat_adc_val_sample_index >= SAMPLE_COUNT) {
        bat_adc_val_sample_index = 0;
    }
}

// 从滑动平均数组中读出数据
u16 bat_adc_val_get_avg(void)
{
    u8 i;
    u32 sum = 0;
    for (i = 0; i < SAMPLE_COUNT; i++) {
        sum += bat_adc_val_samples[i];
    }

    return (sum / SAMPLE_COUNT);
}

// 初始化电池电压的滑动平均数组
void bat_vol_samples_init(u16 voltage)
{
    u8 i;
    for (i = 0; i < BAT_VOL_SAMPLE_COUNT; i++) {
        bat_vol_samples[i] = voltage;
    }
}

// 向电池电压的滑动平均数组中放入数据
void bat_vol_samples_update(u16 voltage)
{
    static u8 is_initialized = 0;
    if (0 == is_initialized) {
        is_initialized = 1;
        bat_vol_samples_init(voltage);
        return; // 初始化数组之后，直接退出，下一次得到新数据才执行下面的操作
    }

    bat_vol_samples[bat_vol_sample_index] = voltage;
    bat_vol_sample_index++;
    if (bat_vol_sample_index >= BAT_VOL_SAMPLE_COUNT) {
        bat_vol_sample_index = 0;
    }
}

// 从电池电压的滑动平均数组中取出平均值
u16 bat_vol_get_avg(void)
{
    u8 i;
    u32 sum = 0;
    for (i = 0; i < BAT_VOL_SAMPLE_COUNT; i++) {
        sum += bat_vol_samples[i];
    }

    return (sum / BAT_VOL_SAMPLE_COUNT);
}

void bat_scan_time_add(void)
{
    if (battery_scan_time_cnt < ((u16)-1)) {
        battery_scan_time_cnt++;
    }
}

void bat_grid_update_time_add(void)
{
    if (bat_grid_update_time_cnt < ((u16)-1)) {
        bat_grid_update_time_cnt++;
    }
}

/**
 * @brief 将电池电压转换为对应的格子数
 *
 * @param voltage 电池电压，单位：mV
 *
 * @return u8 电池格子数
 *          0：低电量提示
 *          1：没有这种情况
 *          2：2格，表示电池电压大于等于9V
 *          ...
 *          9：9格，表示电池电压大于等于16V
 *
 */
u8 bat_vol_to_grid(u16 voltage)
{
    u8 ret = 0; // 默认为0格，对应低电量提示

    if (voltage >= BAT_VOLTAGE_OF_FULL_GRID) {
        ret = 9;
    } else if (voltage >= BAT_VOLTAGE_OF_LOW_GRID) {
        // 表达式中的 1，表示至少要点亮1格
        ret = 2 + (voltage - BAT_VOLTAGE_OF_LOW_GRID) / 1000;
    } else {
        ret = 0;
    }

    return ret;
}

void bat_scan(void)
{
    u16 adc_val;
    u16 voltage; // 存放电压值，单位：mV

    static u8 is_initialized = 0;           // 是否初始化过
    static volatile u8 bat_grid_of_lag = 0; // 需要延迟显示的电池格子数
    u8 bat_grid = 0;                        // 电池格子数
    u8 bat_grid_diff = 0;                   // 电池格子数差值

    if (battery_scan_time_cnt >= BAT_SCAN_PERIOD) {
        battery_scan_time_cnt = 0;
    } else {
        // 没有到扫描周期，直接返回
        return;
    }

    adc_val = bat_adc_val_get_avg();
    voltage = ADC_VAL_TO_BAT_VOLTAGE(adc_val);

#if USER_DEBUG_ENABLE
// printf("adc_val == %u\n", adc_val);
// printf("voltage == %u\n", voltage);
#endif

    if (is_initialized == 0) {
        is_initialized = 1;
        bat_vol_samples_init(voltage); // 初始化电压值对应的滑动平均数组
        bat_grid = bat_vol_to_grid(voltage);
        bat_grid_of_lag = bat_grid;

        if (0 == bat_grid_of_lag) {
            // 电池格子数为0，打开低电量提示
            instrument.flag_is_in_warning_of_low_battery = 1;
        }

        // 如果 0 == bat_grid_of_lag，让显示函数里面清空显示
        aip3368h_display_bat_lev_light(bat_grid_of_lag);
    } else {
        // 如果已经初始化过，则更新电压值对应的滑动平均数组
        bat_vol_samples_update(voltage);
    }

    voltage = bat_vol_get_avg(); // 获取电压值的滑动平均
#if USER_DEBUG_ENABLE
    // printf("avg voltage == %u\n", voltage);
#endif

    bat_grid = bat_vol_to_grid(voltage);
    if (bat_grid != bat_grid_of_lag) {
        // 电池格子数有变化，需要更新显示
        if (bat_grid > bat_grid_of_lag) {
            bat_grid_diff = bat_grid - bat_grid_of_lag;
        } else {
            bat_grid_diff = bat_grid_of_lag - bat_grid;
        }
    }

    /*
        没有到电池电压格子数的更新时间，直接返回
        1. 差值大于等于2，说明电池电压变化幅度较大，按照更新周期较小的来更新
        2. 差值小于等于1，说明电池电压变化幅度不大，按照更新周期较大的来更新
    */
    // if ((bat_grid_diff >= 2 &&
    //      bat_grid_update_time_cnt >= BAT_DISPLAY_UPDATE_PERIOD) ||
    //     (bat_grid_diff <= 1 && bat_grid_update_time_cnt >=
    //                                BAT_DISPLAY_UPDATE_PERIOD_WHEN_BAT_CLOSE))
    //                                {
    //     return;
    // }

    bat_grid_update_time_cnt = 0;

    if (bat_grid_of_lag < bat_grid) {
        bat_grid_of_lag++;
    } else if (bat_grid_of_lag > bat_grid) {
        bat_grid_of_lag--;
    }

#if USER_DEBUG_ENABLE
    // printf("bat_grid == %u\n", (u16)bat_grid);
    // printf("bat_grid_of_lag == %u\n", (u16)bat_grid_of_lag);
#endif

    if (0 == bat_grid_of_lag) {
        // 电池格子数为0，打开低电量提示

        if (0 == instrument.flag_is_in_warning_of_low_battery) {
            // 如果之前不处于低电量提示，先关闭对应的指示灯
            aip3368h_display_bat_lev_light(0);
        }

        instrument.flag_is_in_warning_of_low_battery = 1;
    } else {
        instrument.flag_is_in_warning_of_low_battery = 0;
        aip3368h_display_bat_lev_light(bat_grid_of_lag);
    }
}
#endif // BATTERY_SCAN_ENABLE