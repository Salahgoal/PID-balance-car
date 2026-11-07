/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "adc.h"


/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* 全局 ADC2 句柄，提供给应用层（如 ReadVolt()）调用 HAL API 使用。 */
ADC_HandleTypeDef hadc2;

/* ADC2 init function */
void MX_ADC2_Init(void)
{

  /* 配置通道结构体（后续会用到 channel/rank/sampling time） */
  ADC_ChannelConfTypeDef sConfig = {0};

  /* --------- 通用 ADC 配置（适用于单通道、软件触发场景） --------- */
  hadc2.Instance = ADC2;                         // 使用 ADC2 外设
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;    // 不扫描多个通道（只测 PB0）
  hadc2.Init.ContinuousConvMode = DISABLE;      // 非连续（按需单次采样）
  hadc2.Init.DiscontinuousConvMode = DISABLE;   // 非不连续模式
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START; // 使用软件触发（HAL_ADC_Start）
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;   // 右对齐，便于直接以整数处理
  hadc2.Init.NbrOfConversion = 1;                // 单通道单次转换

  /* 初始化 ADC，若失败则进入统一错误处理（与工程风格一致） */
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /* --------- 配置具体通道（此处为 ADC2 的 IN8，对应 PB0） --------- */
  sConfig.Channel = ADC_CHANNEL_8;              // ADC2_IN8 -> PB0（硬件连线决定）
  sConfig.Rank = ADC_REGULAR_RANK_1;            // 规则序列中的第 1 位
  /*
   * 采样时间：ADC_SAMPLETIME_1CYCLE_5
   * 优点：采样快、延迟小；缺点：对高阻抗源（如分压网络阻值较大）可能不稳定。
   * 如果你在电池采样时看到波动/偏差，建议改为更长采样时间（如 7.5 或 239.5 周期）。
   */
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;

  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspInit 0 */

  /* USER CODE END ADC2_MspInit 0 */
    /* 使能 ADC2 时钟：ADC 外设需要时钟才能运行 */
    __HAL_RCC_ADC2_CLK_ENABLE();

    /* 使能 GPIOB 时钟并配置 PB0 为模拟输入（ADC2_IN8） */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /** ADC2 GPIO Configuration
     *  PB0  ------> ADC2_IN8
     *
     *  注意：模拟输入模式会禁用数字输入缓冲，降低噪声与功耗。
     */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;  // 模拟模式
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC2_MspInit 1 */

  /* USER CODE END ADC2_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC2)
  {
  /* USER CODE BEGIN ADC2_MspDeInit 0 */

  /* USER CODE END ADC2_MspDeInit 0 */
    /* 反初始化：关闭 ADC2 时钟并恢复 PB0 引脚设置（降低功耗/释放资源） */
    __HAL_RCC_ADC2_CLK_DISABLE();

    /** ADC2 GPIO 反初始化，恢复 PB0 为默认状态 */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0);

  /* USER CODE BEGIN ADC2_MspDeInit 1 */

  /* USER CODE END ADC2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

