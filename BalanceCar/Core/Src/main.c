/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "dwt.h"
#include "oled.h"
#include "oledBmp.h"
#include "Battery.h"
#include "stdio.h"
#include "hcsr04.h"
#include "motor.h"
#include "encoder.h"
#include "mpu6050.h"
#include "pid.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

float Volt;
float Distance;
uint32_t Timer2_Count;

short Encode_Left, Encode_Right;    // 左右轮编码器
extern uint8_t Flag_Control;           // 平衡总开关
extern uint8_t Flag_Stop;              // 急停标志

extern float Roll,Pitch,Yaw; 

uint8_t Flag_Balance=0;	//平衡模式标志
extern uint8_t Flag_Avoid;	//避障功能标志

//蓝牙模块相关
uint8_t Uart_Buff[128]={0};
uint8_t Uart_Data;
uint8_t Backward_Flag = 0;
uint8_t Forward_Flag = 0;
uint8_t TurnLeft_Flag = 0;
uint8_t TurnRight_Flag = 0;

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void OLED_Show(void);
void Scan_Key(void);
void LED_Mode(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC2_Init();
  MX_TIM2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  DWT_Delay_Init();
  
  OLED_Init();
  
  MPU6050_initialize();
  
  if(DMP_Init()!=8)while(1);
  HAL_Delay(500);  
  
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
  HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
  
  HAL_NVIC_SetPriority(EXTI1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 10, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
  
  HAL_UART_Receive_IT(&huart3,Uart_Buff,1);

  OLED_Show();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Trigger_Hcsr04();
	  
	  Volt=ReadVolt();
	  OLED_Show();
	  
	  Scan_Key();
	  LED_Mode();
	  
	  HAL_Delay(10);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection  = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void OLED_Show()
{
	unsigned char Str_Buff[64];
	
	sprintf((char *)Str_Buff,"Uart_Data: %d  ",Uart_Data);
	OLED_ShowString(0,0,Str_Buff);
	
	sprintf((char *)Str_Buff,"Distance: %.2f  ",Distance);
	OLED_ShowString(0,2,Str_Buff);
	
	sprintf((char *)Str_Buff,"Pitch:%.2f ",Pitch);
	OLED_ShowString(0,4,Str_Buff);	
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	HAL_GPIO_TogglePin(Avoid_LED_GPIO_Port, Avoid_LED_Pin);
	
	if(GPIO_Pin == Echo_Pin)
	{
		if(HAL_GPIO_ReadPin(Echo_GPIO_Port,Echo_Pin)==GPIO_PIN_SET)
		{
			__HAL_TIM_SetCounter(&htim2,0);
			HAL_TIM_Base_Start(&htim2);
		}
		else
		{
			HAL_TIM_Base_Stop(&htim2);
			Timer2_Count = __HAL_TIM_GetCounter(&htim2);
			Distance = (Timer2_Count * 17.0) / 1000.0;
		}
	}
	else if(GPIO_Pin==MPU6050_INT_Pin)
	{
		static int countInto = 0;
		countInto++;
		if(countInto == 250)
		{
			HAL_GPIO_TogglePin(PID_LED_GPIO_Port,PID_LED_Pin);
			countInto=0;
		}
        
		motorControl();  
	}
}


void Scan_Key()
{
	if(HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin)==0)
	{
		HAL_Delay(10);
		if(HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin)==0)
		{
			Flag_Balance = !Flag_Balance;
			Flag_Control = Flag_Balance;   // 同步给PID
			Flag_Stop = 0;                 // 解除急停
		}
		while(HAL_GPIO_ReadPin(KEY_GPIO_Port,KEY_Pin)==0);
	}
}

void LED_Mode()
{
	if(Flag_Avoid == 0)
		HAL_GPIO_WritePin(Avoid_LED_GPIO_Port,Avoid_LED_Pin,GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(Avoid_LED_GPIO_Port,Avoid_LED_Pin,GPIO_PIN_RESET);
	
	if(Flag_Balance == 0)
		HAL_GPIO_WritePin(Balance_LED_GPIO_Port,Balance_LED_Pin,GPIO_PIN_SET);
	else
		HAL_GPIO_WritePin(Balance_LED_GPIO_Port,Balance_LED_Pin,GPIO_PIN_RESET);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART3){	//判断接收到的数据是否来自蓝牙模块
		Uart_Data = Uart_Buff[0];	//读取接收到的指令
		
		//初始化清零
		Forward_Flag=0;
		Backward_Flag=0;
		TurnLeft_Flag=0;
		TurnRight_Flag=0;
		
		switch(Uart_Data)
		{
			case 0x41: Forward_Flag=1; break;			//65	'A'
			
			case 0x45: Backward_Flag=1; break;			//69	'E'
			//66 67 68 'B' 'C' 'D' 都是右转 前右 正右 后右
			case 0x42:
			case 0x43:
			case 0x44: TurnRight_Flag=1;break;			
			//70 71 72 'F' 'G' 'H' 前左 正左 后左
			case 0x46:
			case 0x47:
			case 0x48: TurnLeft_Flag=1;break;		
			
			case 0x5A: break;							//90 'Z' 停止

			case 0x4F:Flag_Avoid = !Flag_Avoid; break;	//79  'O' 避障功能开关
			
			default:   break;
		}
		HAL_UART_Receive_IT(&huart3,Uart_Buff,1);
	}
    
}

int fputc(int ch,FILE *f)
{
	HAL_UART_Transmit(&huart1,(uint8_t*)&ch,1,HAL_MAX_DELAY);
	return ch;
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */

