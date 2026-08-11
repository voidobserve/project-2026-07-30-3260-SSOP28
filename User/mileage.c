// encoding UTF-8
// mileage.c
#include "mileage.h"

// 里程扫描所需的计数值,每隔一定时间将里程写入flash
volatile u16 mileage_save_time_cnt;
// 存放每次扫描时走过的路程（单位：毫米）-->用于里程表的计数
volatile u32 distance;

// 里程更新的时间计数,每隔一段时间更新一次当前里程（负责控制发送里程的周期）
volatile u16 mileage_update_time_cnt;

static void aip3368h_display_mileage_unit_lights(u8 distance_unit_type)
{
    if (DISTANCE_UNIT_TYPE_METRIC == distance_unit_type) {
        aip3368h_display_miles_light(0);
        aip3368h_display_km_light(1);
    } else if (DISTANCE_UNIT_TYPE_IMPERIAL == distance_unit_type) {
        aip3368h_display_km_light(0);
        aip3368h_display_miles_light(1);
    }
}

static void aip3368h_display_mileage_value(u32 mileage_value, u8 display_mode)
{
    u8 is_trip_mode = (MILEAGE_DISPLAY_MODE_TRIP == display_mode);

    if (DISTANCE_UNIT_TYPE_METRIC == instrument.save_info.distance_unit_type) {
        // 使用 公制 单位
        aip3368h_display_mileage(mileage_value / (is_trip_mode ? 100u : 1000u),
                                 display_mode);
    } else if (DISTANCE_UNIT_TYPE_IMPERIAL ==
               instrument.save_info.distance_unit_type) {
        // 使用 英制 单位
        // 1km == 0.621427mile
        // 0.621427 mile * 1610 == 1000.49747 m
        aip3368h_display_mileage(mileage_value / (is_trip_mode ? 161u : 1610u),
                                 display_mode);
    }

    aip3368h_display_mileage_unit_lights(
        instrument.save_info.distance_unit_type);
}

static void aip3368h_display_mileage_mode_lights(u8 is_display_total_mileage)
{
    aip3368h_display_trip_light(is_display_total_mileage ? 0 : 1);
    aip3368h_display_odo_light(is_display_total_mileage ? 1 : 0);
}

/**
 * @brief 刷新显示的里程（TOTAL 或 TRIP），会刷新单位、TRIP和ODO对应的指示灯
 *
 */
void aip3368h_display_mileage_refresh(void)
{
    u8 is_display_total_mileage = instrument.save_info.is_display_total_mileage;
    u32 mileage_value = is_display_total_mileage
                            ? instrument.save_info.total_mileage
                            : instrument.save_info.subtotal_mileage;
    u8 display_mode = is_display_total_mileage ? MILEAGE_DISPLAY_MODE_ODO
                                               : MILEAGE_DISPLAY_MODE_TRIP;

    aip3368h_display_mileage_value(mileage_value, display_mode);
    aip3368h_display_mileage_mode_lights(is_display_total_mileage);
}

// 总里程扫描
void mileage_scan(void)
{
    /*
        是否有里程数据需要保存的标志变量，
        0--没有里程变化，不需要保存，
        1--有里程变化，需要保存
        目前每过1m就会置位一次，保存之后清零
    */
    static volatile bit flag_is_any_mileage_save;

    // 每过1s，且里程有变化，就保存一次；这个里程变化的条件最好大于10m，否则会经常写入eeprom
    if ((mileage_save_time_cnt >= (u16)5 * 1000) && /* xx ms后 */
        flag_is_any_mileage_save)                   /* 里程有变化，需要保存 */
    {
        instrument_info_save_enable();
        flag_is_any_mileage_save = 0;
        mileage_save_time_cnt = 0;

        // printf("mile save\n");

        // printf("total_mileage %lu\n", instrument.save_info.total_mileage);
        // printf("sub_total_mileage %lu\n",
        // instrument.save_info.subtotal_mileage); printf("sub_total_mileage_2
        // %lu\n", instrument.save_info.subtotal_mileage_2);
    }

    if (distance >= 1000) // 1000mm -- 1m
    {
        // 如果走过的距离超过了1m，再进行保存（保存到变量）
        // 99 9999 KM
        if (instrument.save_info.total_mileage < (u32)(999999 * 1000)) {
            instrument.save_info.total_mileage++; // +1m
        }

        // 99999.9 KM
        if (instrument.save_info.subtotal_mileage < (u32)(999999 * 100)) {
            instrument.save_info.subtotal_mileage++; // +1m
        }

        distance -= 1000; // 剩下的、未保存的、不满1m的数据留到下一次再保存

        {
            static u8 cnt = 0;
            cnt++;
            // if (cnt >= 10) // cnt >= 10，说明走过了10m
            if (cnt >= 100) {
                cnt = 0;
                flag_is_any_mileage_save = 1; // 表示需要把里程输入写入到flash
            }
        }
    }
}

#if 1

/**
 * @brief 显示里程，由ui调用
 *
 */
void aip3368h_display_mileage_handle(void)
{
    static u8 is_initialized = 0;

    if (0 == is_initialized) {
        is_initialized = 1;
        // 显示单位
        if (DISTANCE_UNIT_TYPE_METRIC ==
            instrument.save_info.distance_unit_type) {
            // 公制单位，km
            aip3368h_display_miles_light(0);
            aip3368h_display_km_light(1);
        } else {
            // 英制单位：miles
            aip3368h_display_km_light(0);
            aip3368h_display_miles_light(1);
        }

        if (instrument.save_info.is_display_total_mileage) {
            // 显示总里程
            aip3368h_display_trip_light(0);
            aip3368h_display_odo_light(1);
        } else {
            // 显示当前里程（小计里程）
            aip3368h_display_odo_light(0);
            aip3368h_display_trip_light(1);
        }

        aip3368h_display_mileage_refresh();
    }

    if (mileage_update_time_cnt >= MILEAGE_UPDATE_TIME_MS) {
        // 每隔一段时间，更新里程
        mileage_update_time_cnt = 0;
        aip3368h_display_mileage_refresh();
    }
}
#endif
