#include "DWT.h"

/**
 * @brief  DWT初始化状态标记
 * @note   0-未初始化 1-已初始化，防止DWT寄存器重复配置
 */
static uint8_t g_dwt_init_flag = 0;  

/**
 * @brief  初始化DWT（数据观察点和跟踪单元）延时功能
 * @note   1. 该函数仅首次调用时执行寄存器配置，避免重复操作
 *         2. 基于Cortex-M3内核的DWT模块实现高精度延时，需在HAL初始化后调用
 *         3. 核心是开启调试跟踪功能并启动CYCCNT周期计数器
 * @retval 无
 */
void DWT_Delay_Init(void)
{
    // 仅未初始化时执行配置
    if (g_dwt_init_flag == 0)
    {
        DWT_DEMCR |= DEMCR_TRCENA;       // 使能内核调试跟踪功能（DEMCR寄存器TRCENA位）
        DWT_CYCCNT = 0U;                 // 重置CYCCNT计数器为0，保证计时起点准确
        DWT_CTRL |= DWT_CTRL_CYCCNTENA;  // 使能CYCCNT计数器，开始周期计数
        g_dwt_init_flag = 1U;            // 标记初始化完成，避免重复配置
    }
}

/**
 * @brief  基于DWT实现微秒级精准延时
 * @param  us: 延时时长，单位：微秒（us）
 * @note   1. 时钟频率为72MHz时，1us = 72个CPU时钟周期（需与实际主频匹配）
 *         2. 利用CYCCNT计数器的差值判断延时是否完成，支持计数器溢出自动归零
 *         3. __NOP()空操作防止编译器优化空循环，保证延时逻辑有效
 *         4. 有效范围：0 ~ 59652321us（约59.6秒），超出范围会导致计时不准
 * @retval 无
 */
void DWT_Delay_us(uint32_t us)
{
    // 延时0us直接返回，避免无效计算
    if (us == 0U)
        return;

    uint32_t start_cnt = DWT_CYCCNT;                   // 记录延时起始时刻的计数器值
    const uint32_t clk_cycles = (uint32_t)(us * 72U);  // 计算需要等待的总时钟周期数（72MHz主频）

    // 循环等待，直到计数器差值达到目标周期数（支持计数器溢出归零的情况）
    while ((DWT_CYCCNT - start_cnt) < clk_cycles)
    {
        __NOP();  // 空操作指令，防止编译器优化掉延时循环（HAL库标准空操作宏）
    }
}

/**
 * @brief  基于DWT实现毫秒级精准延时
 * @param  ms: 延时时长，单位：毫秒（ms）
 * @note   1. 底层通过循环调用1000次微秒延时实现1ms延时
 *         2. 有效范围：0 ~ 59652ms（约59.6秒），超出范围会导致计时不准
 *         3. 相比传统SysTick延时，DWT延时精度更高且不占用中断资源
 * @retval 无
 */
void DWT_Delay_ms(uint32_t ms)
{
    // 逐毫秒延时，循环调用微秒延时函数
    for (uint32_t i = 0U; i < ms; i++)
    {
        DWT_Delay_us(1000U);  // 1毫秒 = 1000微秒，复用us级延时逻辑保证精度
    }
}

