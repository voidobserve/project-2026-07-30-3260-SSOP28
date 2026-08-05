// user_config.h -- 自定义的配置文件
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// #include "include.h" // 使用芯片官方提供的头文件
// #include "typedef_struct.h" // 包含结构体类型和别名的定义

#define ARRAY_SIZE(arry) (sizeof(arry) / sizeof(arry[0]))

#define USER_DEBUG_ENABLE 1
#if USER_DEBUG_ENABLE
#define DEBUG_PIN 
#endif

#define TOUCH_KEY_ENABLE 0 // 是否使能触摸按键检测功能
#define AD_KEY_ENABLE 0	   // 是否使能ad按键检测功能
#define IO_KEY_ENABLE 1
#define SPEED_SCAN_ENABLE 1			// 是否使能速度检测功能
#define ENGINE_SPEED_SCAN_ENABLE 1	// 是否使能发动机转速检测功能
#define BATTERY_SCAN_ENABLE 1		// 是否使能电池检测功能
#define FUEL_CAPACITY_SCAN_ENABLE 1 // 是否使能油量检测功能

#define PIN_LEVEL_SCAN_ENABLE 1 // 是否使能引脚电平检测功能

#define TEMP_OF_WATER_SCAN_ENABLE 0 // 是否使能水温检测

#define USE_INTERNAL_FLASH_SAVE_DATA 1
 

 


// 主函数完成一次循环所需的时间，单位：ms (0--说明每次调用该函数的时间很短，可以忽略不计)(注意不能大于变量类型的大小)
// 功能全部开放后，每一轮的时间可能都不一样，不在时间要求高的场合使用(目前最短的一个周期是8ms)
// #define ONE_CYCLE_TIME_MS (8)
#if USER_DEBUG_ENABLE
#include <stdio.h>	//
#endif


#endif // end file
