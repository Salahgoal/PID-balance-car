#include "Battery.h"
#include "main.h"

float ReadVolt()
{
	uint32_t Adc_Value;	//采样值
	float Convert_Volt;	//转换后的电压值
	float Acutal_Volt;	//电池实际的电压
	
	//	1.启动ADC2--等待转换成功--获取到采样值
	HAL_ADC_Start(&hadc2);
	HAL_ADC_PollForConversion(&hadc2,100);
	Adc_Value=HAL_ADC_GetValue(&hadc2);
	
	//	2.计算转换后的采样值	V=采样*参考电压3.3V / 最大采样值4095(因为是12位ADC，即2的12次方减1)
	Convert_Volt = (Adc_Value * 3.3f) / 4095.0f;
	
	//	3.计算电池的实际电压	V=Convert_Volt*(10000+1000) / 1000;
	Acutal_Volt=Convert_Volt * (10+1) / 1.0f;
	
	//	4.停止ADC2
	HAL_ADC_Stop(&hadc2);
	
	return Acutal_Volt;
}

