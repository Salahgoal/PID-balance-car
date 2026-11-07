#include "pid.h"
#include "mpu6050.h"
#include "encoder.h"
#include "tim.h"
#include "motor.h"
#include "stdlib.h"
#include "math.h"

#define Middle_Angle 0    //机械中值角度
int MOVE = 40;            //加减速、转向的基础幅值

/**Kp 太小：力气太小，扶不起来，直接倒；Kp 太大：力气太大，一扶正就晃，疯狂抖动
   Kd 太小：小车来回晃（像不倒翁）Kd 太大：小车变硬、发抖
	因此在直立环中，Balance_Kp → 扶直的力气，Balance_Kd → 防止晃动的刹车。
**/
float Balance_Kp=-260,Balance_Kd=-1.28;    //直立环PD参数
float Velocity_Kp=-180,Velocity_Ki=-0.9; //速度环PI参数

/** 	
	Kp 太小：转弯慢，推摇杆不动 Kp 太大：转弯太猛，直接转圈
	Kd是用来直行防止跑偏的
**/
float Turn_Kp=-70,Turn_Kd=0.6;            //转向环PD参数
uint8_t Flag_Stop = 0;                //停止标志位 1-停   0-平衡调控

extern short Encode_Left, Encode_Right;    //左右轮编码器计数值

uint8_t Flag_Avoid;          //避障标志位 1-开启避障 0-关闭避障
extern float Distance;       //避障检测的距离值 单位:cm

extern uint8_t Forward_Flag, Backward_Flag, TurnLeft_Flag, TurnRight_Flag;
uint8_t Flag_Control = 0;    //平衡控制标志   1进行控制   0不控制  

float Encoder_Integral=0;  //速度环积分项，静态变量保持值不丢失

/*
Input   : Angle:姿态角(Pitch俯仰角)  Gyro:角速度(X轴角速度)
Output  : balance:直立控制的PWM分量
功能说明: 小车核心直立环控制，采用PD算法抑制前倾后仰，无积分防止静差累积
*/
int Balance(float Angle,float Gyro)
{  
   float Angle_bias,Gyro_bias;
   int balance;
   Angle_bias=Middle_Angle-Angle;                        //计算平衡角度偏差 = 机械中值 - 当前俯仰角
   Gyro_bias=0-Gyro;                                     //计算角速度偏差，抑制倾角变化趋势
   //直立PD公式：Balance_Kp × 歪了多少度 + Balance_Kd × 歪的速度
   balance=Balance_Kp*Angle_bias+Balance_Kd*Gyro_bias; 
   return balance;
}

/*
Input   : encoder_left:左轮编码器读数  encoder_right:右轮编码器读数
Output  : 速度控制PWM 速度环补偿PWM分量
功能说明: 基于左右轮编码器差值做速度闭环，PI算法保证匀速行驶+消除静差，带积分限幅防飞车
*/
int Velocity(int encoder_left,int encoder_right)
{  
    static float velocity=0,Encoder_Least=0,Encoder_bias=0,Movement=0;
    //static float Encoder_Integral;  //速度环积分项，静态变量保持值不丢失

		//====================小车运动控制======================
		if(Forward_Flag == 0 && Backward_Flag == 0) Movement = 0;

		if(Forward_Flag == 1) Movement = MOVE;
		else if(Backward_Flag == 1) Movement = -MOVE;
	
		//避障触发逻辑 - 开启避障且检测距离小于20cm，执行后退
	  if(Flag_Avoid && Distance < 20) Movement = -MOVE;
	
    //================速度PI控制核心区=====================//	
		Encoder_Least =0-(encoder_left+encoder_right);  //速度偏差=目标速度(0)-实际速度(左右轮和值)
		
	  Encoder_bias *= 0.7;		                        //一阶低通滤波，滤除编码器高频噪声
		Encoder_bias += Encoder_Least*0.3;	            //滤波后更新速度偏差值
		Encoder_Integral += Encoder_bias;                //积分累积，消除速度静差，周期10ms
		Encoder_Integral=Encoder_Integral+Movement;     //叠加手动运动指令的速度幅值
	
		//积分限幅，防止积分饱和导致的飞车/失控
		if(Encoder_Integral>10000)  Encoder_Integral=10000;             
		if(Encoder_Integral<-10000) Encoder_Integral=-10000;            
	
		//速度环PI计算公式
		velocity=Encoder_bias*Velocity_Kp+Encoder_Integral*Velocity_Ki;     
		if(Flag_Stop==1){
			Encoder_Integral=0; 	//急停时清零积分，防止重启飞车
			Encoder_bias = 0;		//清零滤波后的偏差，防止残留
		}
	  return velocity; 
}

/*
Input   : gyro:Z轴角速度(陀螺仪)
Output  : Turn control PWM 转向补偿PWM分量
功能说明: 基于Z轴角速度做转向闭环，PD算法保证转向精准无超调，直行时增强阻尼防跑偏
*/
int Turn(float gyro)
{
    static float Turn_Target,turn;
    float Kp=Turn_Kp,Kd;			//转向环比例+微分参数，微分系数动态调整
		
		//====================小车转向控制=======================
		if(TurnLeft_Flag == 1)   Turn_Target = MOVE / 2;
		else if(TurnRight_Flag == 1)  Turn_Target = -MOVE / 2;
		else Turn_Target = 0;
		
		if(Forward_Flag == 1 || Backward_Flag == 1) 
		{
			Kd = Turn_Kd;
		}
		else 
		{
			Kd = 0;
		}
	
		//===================转向PD控制区=================//
		turn=Turn_Target*Kp + gyro*Kd;   //转向PD公式，Z轴角速度负反馈抑制转向过度
		return turn;						               //返回转向PWM分量：正=左轮减速 负=右轮减速
}

/**************************************************************************
Function: motorControl 总控制函数
功能说明: 整合直立、速度、转向三环PID，计算最终左右轮PWM值，包含硬件保护逻辑
**************************************************************************/
void motorControl(void)
{
    Read_DMP();  // 读取陀螺仪DMP数据，更新全局变量Pitch等和gyro数组

    // 角度超限急停
    // 只要Pitch/Roll超过±45度（提起/摔倒），立刻急停清零积分
    if(fabs(Pitch) > 40 || fabs(Roll) > 40)
    {
        Flag_Stop = 1;
        Encoder_Integral = 0;  //清零积分
        Set_PWM(0, 0);  // 立刻停电机
        return;         // 直接退出，不执行后面的PID计算
    }

    // 自动解除急停
    // 当角度回到±20度以内，自动解除急停，恢复平衡
    if(Flag_Stop == 1 && fabs(Pitch) < 15 && fabs(Roll) < 15)
    {
        Flag_Stop = 0;
		Encoder_Integral = 0;	//恢复前再清零一次积分，清空残留
    }

    // 读取左右轮编码器值并清零计数器，下次重新计数
    Encode_Left = __HAL_TIM_GET_COUNTER(&htim3);   // 左轮编码器-定时器3
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    Encode_Right = __HAL_TIM_GET_COUNTER(&htim4);  // 右轮编码器-定时器4
    __HAL_TIM_SET_COUNTER(&htim4, 0);
	
	//编码器超速保护
    if(abs(Encode_Left) > 80 || abs(Encode_Right) > 80)
    {
        Flag_Stop = 1;
        Encoder_Integral = 0;
		Set_PWM(0,0);
		return ;
    }

    // 计算三环PWM分量
    int Balance_Pwm = Balance(Pitch, gyro[0]);     // 直立环：Pitch俯仰角 + X轴角速度
    int Velocity_Pwm = Velocity(Encode_Left, Encode_Right); // 速度环
    int Turn_Pwm = Turn(gyro[2]);                  // 转向环：Z轴角速度

    // 转向时关闭转向约束 直行时开启约束（走得更直）
    if(TurnLeft_Flag == 0 && TurnRight_Flag == 0)
    {
        Turn_Kd = 0.6;  
    }
    else if(TurnLeft_Flag == 1 || TurnRight_Flag == 1)
    {
        Turn_Kd = 0;
    }

    // 总PWM计算：直立+速度 为基础，转向分量左右轮相反抵消
    int LeftPWM  = Balance_Pwm + Velocity_Pwm + Turn_Pwm;
    int RightPWM = Balance_Pwm + Velocity_Pwm - Turn_Pwm;
	
	//PWM限幅
	LeftPWM = (LeftPWM > 7200) ? 7200 : (LeftPWM < -7200) ? -7200 : LeftPWM;
    RightPWM = (RightPWM > 7200) ? 7200 : (RightPWM < -7200) ? -7200 : RightPWM;

    // 输出PWM或急停
    if(Flag_Stop == 0 && Flag_Control == 1) Set_PWM(LeftPWM, RightPWM);  
    else Set_PWM(0, 0);
}

