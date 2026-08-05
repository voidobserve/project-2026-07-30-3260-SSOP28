#ifndef __USER_INCLUDE_H__
#define __USER_INCLUDE_H__

#include "user_include.h"

#include <string.h> // memset() 
// #include "tk_set.h" // 包含触摸按键的初始化接口
 
#include "key_driver.h"

#if USER_DEBUG_ENABLE
#include "uart0.h" // 测试时使用
#endif
#include "tmr1.h"			   // 用于 扫描发动机转速、扫描时速、定时将里程写入flash 的定时器
#include "tmr2.h"			   // 用于定时扫描脉冲个数(时速、发动机转速)

#include "pin_level_scan.h"	   // 扫描引脚电平状态
#include "adc.h"			   // adc
#include "speed_scan.h"		   // 速度（时速）扫描
#include "engine_speed_scan.h" // 发动机转速扫描
#include "mileage.h"		   // 里程表的相关接口
// #include "ad_key.h"			   // 检测触摸IC发送过来的ad信号
#include "fuel_capacity.h" // 油量检测 
#include "io_key.h" 

#include "user_flash.h" 

#include "aip3368.h"
#include "aip3368h_display.h"

#include "instrument.h"		   // 存放各个功能的状态信息

#include "ui.h"

#endif