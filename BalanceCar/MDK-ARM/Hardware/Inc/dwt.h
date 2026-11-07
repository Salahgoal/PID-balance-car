#ifndef __DWT_H
#define __DWT_H

#include "stm32f1xx_hal.h"  // 引入STM32F1系列HAL库头文件

// DWT相关寄存器定义（基于Cortex-M3内核标准地址）
#define DWT_DEMCR       (*(volatile uint32_t *)0xE000EDFC)  // 调试异常和监控控制寄存器
#define DWT_CTRL        (*(volatile uint32_t *)0xE0001000)  // DWT控制寄存器
#define DWT_CYCCNT      (*(volatile uint32_t *)0xE0001004)  // 周期计数寄存器（记录CPU时钟周期数）
#define DEMCR_TRCENA    (1 << 24)                           // DEMCR寄存器：使能跟踪调试功能位
#define DWT_CTRL_CYCCNTENA (1 << 0)                         // DWT_CTRL寄存器：使能CYCCNT计数器位

/**
 * @brief  初始化DWT延迟功能（使能CYCCNT计数器）
 * @note   1. 需在HAL_Init()执行完成后、主循环前调用，确保内核调试功能已解锁
 *         2. 该函数会开启Cortex-M3内核的DWT调试模块，为高精度延时做准备
 * @retval 无
 */
void DWT_Delay_Init(void);

/**
 * @brief  基于DWT实现微秒级精准延时
 * @param  us: 延时时长（单位：微秒）
 * @note   1. 时钟频率为72MHz时，1微秒 = 72个CPU时钟周期
 *         2. 有效范围：0 ~ 59652321us（约59.6秒），超出范围会导致计时不准
 *         3. 仅支持内核时钟为72MHz的场景，时钟频率变更需同步调整计数值
 * @retval 无
 */
void DWT_Delay_us(uint32_t us);

/**
 * @brief  基于DWT实现毫秒级精准延时（底层调用微秒延时函数）
 * @param  ms: 延时时长（单位：毫秒）
 * @note   1. 1毫秒 = 1000微秒，底层通过循环调用DWT_Delay_us实现
 *         2. 有效范围：0 ~ 59652ms（约59.6秒），超出范围会导致计时不准
 * @retval 无
 */
void DWT_Delay_ms(uint32_t ms);

/**
 * @brief  获取当前DWT周期计数器的计数值
 * @note   CYCCNT是32位递增计数器，溢出后自动归零，可用于计算时间差
 * @return 当前CYCCNT寄存器值（32位无符号整数）
 */
static inline uint32_t DWT_Get_Cnt(void)
{
    return DWT_CYCCNT;
}

#endif  // __DWT_H
