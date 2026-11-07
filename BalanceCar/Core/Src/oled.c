#include "oled.h"
#include "oledFont.h"  // 引入OLED字库定义（包含ASCII/中文字库数组）
#include "gpio.h"      // 引入GPIO硬件驱动头文件

// ===================== OLED硬件参数宏定义 =====================
// OLED工作模式：4线SPI模式（软件模拟SPI）
#define SIZE            16          // 默认字体大小（16x16）
#define XLevelL         0x00        // X坐标低位配置
#define XLevelH         0x10        // X坐标高位配置
#define Max_Column      128         // OLED最大列数（128列）
#define Max_Row         64          // OLED最大行数（64行）
#define Brightness      0xFF        // OLED默认亮度（最大值）
#define X_WIDTH         128         // OLED屏幕宽度（像素）
#define Y_WIDTH         64          // OLED屏幕高度（像素）

// ===================== OLED引脚操作宏定义（软件SPI） =====================
#define OLED_RST_Clr() HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_RESET);   // 复位引脚拉低
#define OLED_RST_Set() HAL_GPIO_WritePin(OLED_RES_GPIO_Port,OLED_RES_Pin,GPIO_PIN_SET);     // 复位引脚拉高

#define OLED_RS_Clr() HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_RESET);       // DC引脚拉低（写命令）
#define OLED_RS_Set() HAL_GPIO_WritePin(OLED_DC_GPIO_Port,OLED_DC_Pin,GPIO_PIN_SET);         // DC引脚拉高（写数据）

#define OLED_SCLK_Clr()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port,OLED_SCL_Pin,GPIO_PIN_RESET);  // SCL时钟引脚拉低
#define OLED_SCLK_Set()  HAL_GPIO_WritePin(OLED_SCL_GPIO_Port,OLED_SCL_Pin,GPIO_PIN_SET);    // SCL时钟引脚拉高

#define OLED_SDIN_Clr() HAL_GPIO_WritePin(OLED_SDA_GPIO_Port,OLED_SDA_Pin,GPIO_PIN_RESET);   // SDA数据引脚拉低
#define OLED_SDIN_Set()  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port,OLED_SDA_Pin,GPIO_PIN_SET);    // SDA数据引脚拉高

#define OLED_CMD  0       // 写命令标识
#define OLED_DATA 1       // 写数据标识

// ===================== 函数声明 & 全局变量 =====================
void OLED_WR_Byte(uint8_t dat,uint8_t cmd);  // OLED写字节（命令/数据）声明

uint8_t OLED_GRAM[128][8];  // OLED显存数组（128列x8页，1页=8行，对应64行）

// ===================== 核心功能函数 =====================
/**
 * @brief  刷新显存到OLED屏幕
 * @note   1. OLED显存GRAM的内容不会自动显示，需调用此函数刷新
 *         2. 按页刷新：每页8行，共8页（0~7），逐页写入列数据
 *         3. 刷新流程：设置页地址→设置列地址→写入整页列数据
 * @retval 无
 */
void OLED_Refresh_Gram(void)
{
    uint8_t i,n;
    // 遍历8个页（0~7，对应Y轴0~63行）
    for(i=0;i<8;i++)
    {
        OLED_WR_Byte (0xb0+i,OLED_CMD);    // 设置页地址（0xB0~0xB7对应页0~7）
        OLED_WR_Byte (0x00,OLED_CMD);      // 设置列地址低位（0x00~0x0F）
        OLED_WR_Byte (0x10,OLED_CMD);      // 设置列地址高位（0x10~0x1F）
        // 写入当前页的128列数据
        for(n=0;n<128;n++)OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
    }
}

/**
 * @brief  向OLED写入一个字节（命令/数据）
 * @param  dat: 要写入的字节数据
 * @param  cmd: 写入类型标识（OLED_CMD=命令，OLED_DATA=数据）
 * @note   1. 软件模拟SPI时序：DC引脚区分命令/数据，SCLK时钟同步
 *         2. 高位先行（MSB First），逐位写入8位数据
 *         3. 时序逻辑：拉低时钟→设置数据→拉高时钟→移位数据
 * @retval 无
 */
void OLED_WR_Byte(uint8_t dat,uint8_t cmd)
{
    uint8_t i;
    // 设置DC引脚：高=数据，低=命令
    if(cmd)
    {
        OLED_RS_Set();
    }
    else
    {
        OLED_RS_Clr();
    }

    // 逐位写入8位数据（高位先行）
    for(i=0;i<8;i++)
    {
        OLED_SCLK_Clr();          // 拉低时钟，准备数据
        if(dat&0x80)              // 判断当前最高位（bit7）
        {
            OLED_SDIN_Set();      // 数据位为1，SDA拉高
        }
        else
        {
            OLED_SDIN_Clr();      // 数据位为0，SDA拉低
        }
        OLED_SCLK_Set();          // 拉高时钟，锁存当前位
        dat<<=1;                  // 数据左移，处理下一位
    }
    OLED_RS_Set();                // 恢复DC引脚为高电平
}

/**
 * @brief  清空OLED屏幕（清空显存并刷新）
 * @note   1. 先将显存数组全部置0，再调用刷新函数显示
 *         2. 清空范围：128列x8页（对应全屏64行）
 * @retval 无
 */
void OLED_Clear(void)
{
    uint8_t i,n;
    // 遍历所有显存地址，置0清空
    for(i=0;i<8;i++)for(n=0;n<128;n++)OLED_GRAM[n][i]=0X00;
    OLED_Refresh_Gram();          // 刷新显存到屏幕
}

/**
 * @brief  在OLED指定位置画点
 * @param  x: 点的X坐标（0~127）
 * @param  y: 点的Y坐标（0~63）
 * @param  t: 点的状态（1=点亮，0=熄灭）
 * @note   1. 超出坐标范围会直接返回，防止数组越界
 *         2. 显存按页存储：1页=8行，需计算点所在的页和位
 *         3. pos=页号，bx=页内位偏移，通过位操作设置/清除点
 * @retval 无
 */
void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t)
{
    uint8_t pos,bx,temp=0;
    if(x>127||y>63)return;        // 坐标越界，直接返回
    pos=7-y/8;                    // 计算点所在的页（Y轴反向，7-y/8适配屏幕显示方向）
    bx=y%8;                       // 计算点在页内的位偏移（0~7）
    temp=1<<(7-bx);               // 生成位操作掩码
    if(t)
        OLED_GRAM[x][pos]|=temp;  // 点亮：置对应位为1
    else
        OLED_GRAM[x][pos]&=~temp; // 熄灭：置对应位为0
}

/**
 * @brief  在OLED指定位置显示单个字符
 * @param  x: 字符左上角X坐标（0~127）
 * @param  y: 字符左上角Y坐标（0~63，按页对齐，步长为8）
 * @param  chr: 要显示的ASCII字符（仅支持空格~_）
 * @param  size: 字符大小（8=8x8字体，16=16x16字体）
 * @note   1. 字符集为ASCII，通过减去空格（' '）获取字库索引
 *         2. 16x16字体分上下两页显示，8x8字体仅显示一页
 *         3. X坐标超出屏幕时，自动换行到下一行
 * @retval 无
 */
void OLED_ShowChar(unsigned char x,unsigned char y,unsigned char chr,unsigned char size)
{
    unsigned char c=0,i=0;
    c=chr-' ';  // 计算字符在字库中的索引（空格为第一个字符）
    // X坐标越界，自动换行
    if(x>Max_Column-1){x=0;y=y+2;}

    // 显示16x16字体
    if(size ==16)
    {
        OLED_Set_Pos(x,y);
        // 显示字符上半部分（8行）
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i],OLED_DATA);
        // 显示字符下半部分（8行）
        OLED_Set_Pos(x,y+1);
        for(i=0;i<8;i++)
            OLED_WR_Byte(F8X16[c*16+i+8],OLED_DATA);
    }
    // 显示8x8字体
    else if(size ==8)
    {
        OLED_Set_Pos(x,y+1);
        for(i=0;i<6;i++)
            OLED_WR_Byte(F6x8[c][i],OLED_DATA);
    }
}

/**
 * @brief  计算m的n次方（用于数字显示的位权计算）
 * @param  m: 底数
 * @param  n: 指数
 * @retval 计算结果（m^n）
 * @note   仅用于OLED数字显示的辅助计算，无溢出保护
 */
uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
    uint32_t result=1;
    while(n--)result*=m;
    return result;
}

/**
 * @brief  在OLED指定位置显示字符串
 * @param  x: 字符串起始X坐标（0~127）
 * @param  y: 字符串起始Y坐标（0~63，按页对齐）
 * @param  chr: 要显示的ASCII字符串指针（以'\0'结尾）
 * @note   1. 逐个字符调用OLED_ShowChar显示，默认16x16字体
 *         2. 每个字符占8列，X坐标累加8，超出屏幕自动换行
 * @retval 无
 */
void OLED_ShowString(unsigned char x,unsigned char y,unsigned char *chr)
{
    unsigned char j=0;
    // 遍历字符串直到结束符
    while (chr[j]!='\0')
    {
        OLED_ShowChar(x,y,chr[j],16);  // 显示单个字符
        x+=8;                          // 字符宽度8列，X坐标偏移
        // X坐标越界，换行
        if(x>120){x=0;y+=2;}
        j++;
    }
}

/**
 * @brief  OLED初始化函数
 * @note   1. 初始化流程：复位→发送初始化命令→清屏→开启显示
 *         2. 基于SSD1306驱动芯片的命令集，适配128x64 OLED屏幕
 *         3. 关键配置：显示方向、扫描方式、亮度、时钟频率等
 *         4. 需在硬件初始化完成后调用（GPIO已配置）
 * @retval 无
 */
void OLED_Init(void)
{
    // 硬件复位
    OLED_RST_Clr();
    HAL_Delay(100);  // 复位保持100ms
    OLED_RST_Set();

    // ========== 发送初始化命令 ==========
    OLED_WR_Byte(0xAE,OLED_CMD); // 关闭显示（Display OFF）
    OLED_WR_Byte(0xD5,OLED_CMD); // 设置显示时钟分频比/振荡器频率
    OLED_WR_Byte(80,OLED_CMD);   // 分频比=0，振荡器频率=80（默认值）
    OLED_WR_Byte(0xA8,OLED_CMD); // 设置多路复用率
    OLED_WR_Byte(0X3F,OLED_CMD); // 多路复用率=63（1/64 Duty）
    OLED_WR_Byte(0xD3,OLED_CMD); // 设置显示偏移
    OLED_WR_Byte(0X00,OLED_CMD); // 显示偏移=0（无偏移）
    OLED_WR_Byte(0x40,OLED_CMD); // 设置显示起始行（0x40=0行）
    OLED_WR_Byte(0x8D,OLED_CMD); // 电荷泵设置
    OLED_WR_Byte(0x14,OLED_CMD); // 开启电荷泵（bit2=1）
    OLED_WR_Byte(0x20,OLED_CMD); // 设置内存地址模式
    OLED_WR_Byte(0x02,OLED_CMD); // 页地址模式（默认）

    OLED_WR_Byte(0xA1,OLED_CMD); // 设置段重映射（0→127，屏幕镜像翻转）
    OLED_WR_Byte(0xC8,OLED_CMD); // 设置COM扫描方向（反向，COM63→COM0）

    OLED_WR_Byte(0xDA,OLED_CMD); // 设置COM引脚硬件配置
    OLED_WR_Byte(0x12,OLED_CMD); // COM引脚配置：交替模式（bit4=1，bit5=0）
    OLED_WR_Byte(0x81,OLED_CMD); // 设置对比度
    OLED_WR_Byte(0xEF,OLED_CMD); // 对比度值=0xEF（亮度调节，0~255）
    OLED_WR_Byte(0xD9,OLED_CMD); // 设置预充电周期
    OLED_WR_Byte(0xf1,OLED_CMD); // 预充电周期：PHASE1=15，PHASE2=1
    OLED_WR_Byte(0xDB,OLED_CMD); // 设置VCOMH电压倍率
    OLED_WR_Byte(0x30,OLED_CMD); // VCOMH=0.83*VCC（bit6-4=011）

    OLED_WR_Byte(0xA4,OLED_CMD); // 全局显示开启（根据显存显示）
    OLED_WR_Byte(0xA6,OLED_CMD); // 正常显示（0=灭，1=亮）
    OLED_WR_Byte(0xAF,OLED_CMD); // 开启显示（Display ON）

    OLED_Clear(); // 初始化完成，清屏
}

/**
 * @brief  设置OLED显示位置（页+列）
 * @param  x: 列地址（0~127）
 * @param  y: 页地址（0~7）
 * @note   1. 页地址范围0~7（对应Y轴0~63行）
 *         2. 列地址分高低4位发送：高位=0x10+(x>>4)，低位=0x01+(x&0x0F)
 * @retval 无
 */
void OLED_Set_Pos(unsigned char x, unsigned char y)
{
    OLED_WR_Byte(0xb0+y,OLED_CMD);                  // 设置页地址（0xB0~0xB7）
    OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);      // 设置列地址高位
    OLED_WR_Byte((x&0x0f)|0x01,OLED_CMD);           // 设置列地址低位
}

/**
 * @brief  在OLED指定位置显示单个中文字符（16x16）
 * @param  x: 中文字符左上角X坐标（0~127）
 * @param  y: 中文字符左上角Y坐标（0~63，按页对齐）
 * @param  no: 中文字符在字库中的索引（ZhongWen数组）
 * @note   1. 中文字库为16x16点阵，每个字符占32字节（2行×16字节）
 *         2. ZhongWen数组格式：[2*no] = 上半部分，[2*no+1] = 下半部分
 * @retval 无
 */
void OLED_ShowCHinese(unsigned char x,unsigned char y,unsigned char no)
{
    unsigned char t,adder=0;
    OLED_Set_Pos(x,y);
    // 显示中文字符上半部分（16字节，对应8行）
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(ZhongWen[2*no][t],OLED_DATA);
        adder+=1;
    }
    OLED_Set_Pos(x,y+1);
    // 显示中文字符下半部分（16字节，对应8行）
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(ZhongWen[2*no+1][t],OLED_DATA);
        adder+=1;
    }
}

/**
 * @brief  在OLED指定位置显示自定义16x16中文字符
 * @param  x: 字符左上角X坐标（0~127）
 * @param  y: 字符左上角Y坐标（0~63，按页对齐）
 * @param  arr: 16x16点阵数据数组（32字节）
 * @note   1. arr数组格式：前16字节=上半部分，后16字节=下半部分
 *         2. 支持自定义中文字符点阵，无需依赖固定字库
 * @retval 无
 */
void OLED_ShowCHineseArray(uint8_t x,uint8_t y,char arr[32])
{
    uint8_t t,adder=0;
    OLED_Set_Pos(x,y);
    // 显示上半部分（前16字节）
    for(t=0;t<16;t++)
    {
        OLED_WR_Byte(arr[t],OLED_DATA);
        adder+=1;
    }
    OLED_Set_Pos(x,y+1);
    // 显示下半部分（后16字节）
    for(t=16;t<32;t++)
    {
        OLED_WR_Byte(arr[t],OLED_DATA);
        adder+=1;
    }
}

/**
 * @brief  在OLED指定区域显示BMP位图（基础版）
 * @param  x0: 位图左上角X坐标
 * @param  y0: 位图左上角Y坐标（页地址，0~7）
 * @param  x1: 位图右下角X坐标
 * @param  y1: 位图右下角Y坐标（页地址，0~7）
 * @param  BMP: 位图点阵数据数组指针
 * @note   1. 位图数据按页存储，每页8行，需匹配OLED显存格式
 *         2. 仅支持单色位图（1bit/像素），数据长度=(x1-x0)*(y1-y0)*8
 * @retval 无
 */
void OLED_DrawBMP1(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[])
{
    unsigned int j=0;
    unsigned char x,y;

    // 计算需要显示的页数（不足1页按1页算）
    if(y1%8==0) y=y1/8;
    else y=y1/8+1;
    // 遍历每页
    for(y=y0;y<y1;y++)
    {
        OLED_Set_Pos(x0,y);
        // 遍历当前页的所有列
        for(x=x0;x<x1;x++)
        {
            OLED_WR_Byte(BMP[j++],OLED_DATA);
        }
    }
}

/**
 * @brief  在OLED指定位置显示BMP位图（简化版）
 * @param  x: 位图左上角X坐标
 * @param  y: 位图左上角Y坐标（页地址，0~7）
 * @param  width: 位图宽度（像素，0~128）
 * @param  hight: 位图高度（像素，0~64）
 * @param  BMP: 位图点阵数据数组指针
 * @note   1. 封装OLED_DrawBMP1，简化参数传递
 *         2. 高度按页对齐：hight/8 +1，适配64行屏幕
 * @retval 无
 */
void OLED_DrawBMP(unsigned char x, unsigned char y,unsigned char width, unsigned char hight,unsigned char BMP[])
{
    OLED_DrawBMP1(x, y, x + width, y + 1 + hight / 8, BMP);
}

