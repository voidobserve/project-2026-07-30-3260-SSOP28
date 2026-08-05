#ifndef __UART0_H__
#define __UART0_H__

#include "user_config.h"

// 串口0波特率
#define UART0_BAUDRATE (115200)

#if USER_DEBUG_ENABLE
void uart0_debug_init(void);
#endif

#endif
