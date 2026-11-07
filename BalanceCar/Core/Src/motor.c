#include "motor.h"

void Set_PWM(int leftMotor,int rightMotor)
{
	//左右轮调速安全设计
	if(leftMotor>7200)	leftMotor=7200;
	if(leftMotor<-7200)	leftMotor=-7200;
	
	if(rightMotor>7200)	rightMotor=7200;
	if(rightMotor<-7200)	rightMotor=-7200;
	
	if(leftMotor > 0)	//左 正转
	{
		HAL_GPIO_WritePin(BIN1_GPIO_Port,BIN1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port,BIN2_Pin,GPIO_PIN_SET);
	}
	else //左 反转
	{
		HAL_GPIO_WritePin(BIN1_GPIO_Port,BIN1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(BIN2_GPIO_Port,BIN2_Pin,GPIO_PIN_RESET);
	}
	
	__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_1,abs(leftMotor));	//7200
	
	
	if(rightMotor > 0)	//右 正转
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_SET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_RESET);
	}
	else //右 反转
	{
		HAL_GPIO_WritePin(AIN1_GPIO_Port,AIN1_Pin,GPIO_PIN_RESET);
		HAL_GPIO_WritePin(AIN2_GPIO_Port,AIN2_Pin,GPIO_PIN_SET);
	}
	
	__HAL_TIM_SetCompare(&htim1,TIM_CHANNEL_4,abs(rightMotor));	//7200
}

