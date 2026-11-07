#ifndef __OLED_H
#define __OLED_H	

#include <stdint.h>

// 刷新显存数据到OLED屏幕显示
void OLED_Refresh_Gram(void);

// 清空OLED全屏显示内容
void OLED_Clear(void);

// 在指定位置显示单个ASCII字符，size为字体大小(8/16)
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char size);

// 在指定位置显示无符号整数，len为显示位数，size为字体大小
void OLED_ShowNumber(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);

// 在指定位置显示ASCII字符串
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr);

// 在指定位置显示有符号短整数，len为显示位数，size为字体大小
void OLED_ShowShortNum(uint8_t x,uint8_t y,short num,uint8_t len,uint8_t size);

// OLED屏幕初始化配置
void OLED_Init(void);

// 显示浮点数，zs_num整数位数，xs_num小数位数
void OLED_ShowFloat(uint8_t show_x,uint8_t show_y,const float needtoshow,uint8_t zs_num,uint8_t xs_num);

// 设置OLED显示位置（x列，y页）
void OLED_Set_Pos(unsigned char x, unsigned char y) ;

// 向OLED写入命令/数据（cmd=0命令，cmd=1数据）
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);

// 在指定位置显示16x16点阵中文字符，no为字库索引
void OLED_ShowCHinese(unsigned char x,unsigned char y,unsigned char no);

// 在指定位置显示BMP位图，width/height为位图宽高
void OLED_DrawBMP(unsigned char x, unsigned char y,unsigned char width, unsigned char hight,unsigned char BMP[]);
#endif  
	 
