#include "siic.h"

/* 软件iic */

#define userconfig_delayus 1		 									  

#define sIIC_SDA_H HAL_GPIO_WritePin(I2C_SDA_GPIO_Port,I2C_SDA_Pin,GPIO_PIN_SET)
#define sIIC_SDA_L HAL_GPIO_WritePin(I2C_SDA_GPIO_Port,I2C_SDA_Pin,GPIO_PIN_RESET)
#define sIIC_SCL_H HAL_GPIO_WritePin(I2C_SCL_GPIO_Port,I2C_SCL_Pin,GPIO_PIN_SET)
#define sIIC_SCL_L HAL_GPIO_WritePin(I2C_SCL_GPIO_Port,I2C_SCL_Pin,GPIO_PIN_RESET)


static void SDA_IN(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = I2C_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

static void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = I2C_SDA_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(I2C_SDA_GPIO_Port, &GPIO_InitStruct);
}

static uint8_t READ_SDA(void)
{
	return HAL_GPIO_ReadPin(I2C_SDA_GPIO_Port,I2C_SDA_Pin);
}

static void siic_init(void)
{
	//如果IIC引脚没有初始化,此处可添加iic初始化相关内容
}


static void sIIC_Start(void)
{
	SDA_OUT();
  sIIC_SCL_L;
	sIIC_SDA_H;
	sIIC_SCL_H;
	DWT_Delay_us(userconfig_delayus);
	sIIC_SDA_L;
	DWT_Delay_us(userconfig_delayus);
  sIIC_SCL_L;
	DWT_Delay_us(userconfig_delayus);
}

static void sIIC_Stop(void)
{
    SDA_OUT();   // 设置 SDA 为输出
    sIIC_SCL_L;  // 拉低 SCL
    sIIC_SDA_L;  // 拉低 SDA
	
    sIIC_SCL_H;  // 将 SCL 置为高电平
	
	DWT_Delay_us(userconfig_delayus);
	
    sIIC_SDA_H;  // 在 SCL 为高时，将 SDA 拉高生成停止信号
    DWT_Delay_us(userconfig_delayus);
}

static uint8_t sIIC_WaitAck(uint32_t timeout)
{
    uint32_t time = 0;

    SDA_IN();    // 设置 SDA 为输入
    sIIC_SDA_H;  // 拉高 SDA
	
	DWT_Delay_us(userconfig_delayus);
	
    sIIC_SCL_H;  // 拉高 SCL，等待应答信号
	
	DWT_Delay_us(userconfig_delayus);
	
    while (READ_SDA()) {  // 检测从设备是否拉低 SDA，产生ACK信号
        time++;
        DWT_Delay_us(userconfig_delayus);
        if (time > timeout) {
            sIIC_Stop();  // 超时则发送停止信号
            return 0;     // 无应答
        }
    }

    sIIC_SCL_L;  // 拉低 SCL，继续传输
    SDA_OUT();
    return 1;    // 收到应答
}

static void sIIC_Ack(void)
{
    SDA_OUT();   // 设置 SDA 为输出
    sIIC_SCL_L;  // 拉低 SCL
    sIIC_SDA_L;  // 拉低 SDA，发送 ACK
	
	DWT_Delay_us(userconfig_delayus);

    sIIC_SDA_L;
    sIIC_SCL_H;  // 拉高 SCL，确认传输
	
	DWT_Delay_us(userconfig_delayus);

    sIIC_SCL_L;  // 拉低 SCL，结束 ACK 信号
    sIIC_SDA_H;
}

static void sIIC_NAck(void)
{
    SDA_OUT();   // 设置 SDA 为输出
    sIIC_SCL_L;  // 拉低 SCL
    sIIC_SDA_L;  // 拉低 SDA，发送 ACK
	
	DWT_Delay_us(userconfig_delayus);

    sIIC_SDA_H;
    sIIC_SCL_H;  // 拉高 SCL，确认传输
	
	DWT_Delay_us(userconfig_delayus);

    sIIC_SCL_L;  // 拉低 SCL，结束 ACK 信号
    sIIC_SDA_H;
}

static void sIIC_SendByte(uint8_t byte)
{
    uint8_t i;

    SDA_OUT();  // 设置 SDA 为输出
    sIIC_SCL_L; // 拉低 SCL

    for (i = 0; i < 8; i++) {
        if (byte & 0x80) {
            sIIC_SDA_H;  // 发送高位
        } else {
            sIIC_SDA_L;  // 发送低位
        }
        DWT_Delay_us(1);
        byte <<= 1;       // 移动到下一位
        sIIC_SCL_H;       // 拉高 SCL，传输当前位
		DWT_Delay_us(1);
        sIIC_SCL_L;       // 拉低 SCL，准备下一位
  
    }
}

static uint8_t sIIC_ReadByte(uint8_t ack)
{
    uint8_t i, byte = 0;

    SDA_IN();  // 设置 SDA 为输入

    for (i = 0; i < 8; i++) {
        sIIC_SCL_L;  // 拉低 SCL
        DWT_Delay_us(1);	
        sIIC_SCL_H;  // 拉高 SCL，读取当前位
		DWT_Delay_us(1);
        byte <<= 1;
        if (READ_SDA()) {
            byte |= 0x01;  // 读取到高电平
        }
        DWT_Delay_us(1);
    }

    if (!ack) {
        sIIC_NAck();  // 如果不需要应答，发送 NACK
    } else {
        sIIC_Ack();   // 需要应答则发送 ACK
    }

    return byte;
}

static IIC_Status_t IIC_Master_Transmit(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    sIIC_Start();    // 发送起始信号

    sIIC_SendByte(dev_addr);        // 发送从设备地址（写模式）
    if (!sIIC_WaitAck(timeout)) {          // 等待应答
        sIIC_Stop();                // 如果没有应答，则停止通信
        return IIC_TIMEOUT;
    }

    for (uint16_t i = 0; i < size; i++) {
        sIIC_SendByte(data[i]);     // 逐字节发送数据
        if (!sIIC_WaitAck(timeout)) {      // 每发送一个字节后等待应答
            sIIC_Stop();            // 如果没有应答，则停止通信
            return IIC_TIMEOUT;
        }
    }

    sIIC_Stop();                    // 发送停止信号
    return IIC_OK;                       // 成功
}

static IIC_Status_t IIC_Master_Receive(uint16_t dev_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    sIIC_Start();    // 发送起始信号

    sIIC_SendByte(dev_addr | 0x01); // 发送从设备地址（读模式）
    if (!sIIC_WaitAck(timeout)) {          // 等待应答
        sIIC_Stop();                // 如果没有应答，则停止通信
        return IIC_TIMEOUT;
    }

    for (uint16_t i = 0; i < size; i++) {
        data[i] = sIIC_ReadByte(i == (size - 1) ? 0 : 1); // 逐字节读取数据，最后一个字节发送NACK
    }

    sIIC_Stop();                    // 发送停止信号
    return IIC_OK;                       // 成功
}


static IIC_Status_t IIC_Mem_Write(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    sIIC_Start();                  // 发送起始信号
    
    sIIC_SendByte(dev_addr);       // 发送从设备地址（写模式）
    if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT; // 等待应答

    sIIC_SendByte(mem_addr);       // 发送内存/寄存器地址
    if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT; // 等待应答

    for (uint16_t i = 0; i < size; i++) {
        sIIC_SendByte(data[i]);    // 逐字节发送数据
        if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT; // 等待应答
    }
    
    sIIC_Stop();                   // 发送停止信号
    return IIC_OK;                      // 写入成功
}

static IIC_Status_t IIC_Mem_Read(uint16_t dev_addr, uint16_t mem_addr, uint8_t *data, uint16_t size,uint32_t timeout)
{
    sIIC_Start();                   // 发送起始信号
    
    sIIC_SendByte(dev_addr);        // 发送从设备地址（写模式）
    if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT;  // 等待应答

    sIIC_SendByte(mem_addr);        // 发送内存/寄存器地址
    if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT;  // 等待应答
    
    sIIC_Start();                    // 重新启动信号，准备读数据
    sIIC_SendByte(dev_addr | 0x01); // 发送从设备地址（读模式）
    if (!sIIC_WaitAck(timeout)) return IIC_TIMEOUT;  // 等待应答

    for (uint16_t i = 0; i < size; i++) {
        data[i] = sIIC_ReadByte(i == (size - 1) ? 0 : 1); // 逐字节读取数据，最后一个字节发送NACK
    }

    sIIC_Stop();                    // 发送停止信号
    return IIC_OK;                       // 读取成功
}

static void iic_delayms(uint16_t ms)
{
	DWT_Delay_ms(ms);
}


//挂载驱动
IICInterface_t User_sIICDev = {
	.init = siic_init,
	.write = IIC_Master_Transmit , 
	.read = IIC_Master_Receive ,
	.write_reg = IIC_Mem_Write ,
	.read_reg = IIC_Mem_Read ,
	.delay_ms = iic_delayms
};

