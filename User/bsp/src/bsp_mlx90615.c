/*******************************************************************************
*   Filename:       bsp_mlx90615.c
*   Revised:        All copyrights reserved to Roger.
*   Date:           2022-05-04
*   Revision:       v1.0
*   Writer:	        Roger-WY.
*
*   Description:    非接触式红外温度传感器模块驱动(数字式)
*                   
*
*   Notes:          
*   All copyrights reserved to Roger-WY
*******************************************************************************/
#include "bsp_mlx90615.h"


//----------------------------------------------------------------------------//
#define MLX90615_I2C_SCL_RCC     RCC_APB2Periph_GPIOC
#define MLX90615_I2C_SCL_PIN	 GPIO_Pin_4			/* 连接到SCL时钟线的GPIO */
#define MLX90615_I2C_SCL_PORT    GPIOC

#define MLX90615_I2C_SDA_RCC     RCC_APB2Periph_GPIOC
#define MLX90615_I2C_SDA_PIN	 GPIO_Pin_5			/* 连接到SDA数据线的GPIO */
#define MLX90615_I2C_SDA_PORT    GPIOC


/* 定义读写SCL和SDA的宏 */
#define MLX90615_I2C_SCL_1()     MLX90615_I2C_SCL_PORT->BSRR = MLX90615_I2C_SCL_PIN			/* SCL = 1 */
#define MLX90615_I2C_SCL_0()     MLX90615_I2C_SCL_PORT->BRR  = MLX90615_I2C_SCL_PIN			/* SCL = 0 */

#define MLX90615_I2C_SDA_1()     MLX90615_I2C_SDA_PORT->BSRR = MLX90615_I2C_SDA_PIN			/* SDA = 1 */
#define MLX90615_I2C_SDA_0()     MLX90615_I2C_SDA_PORT->BRR  = MLX90615_I2C_SDA_PIN			/* SDA = 0 */

#define MLX90615_I2C_SDA_READ()  ((MLX90615_I2C_SDA_PORT->IDR & MLX90615_I2C_SDA_PIN) != 0)	/* 读SDA口线状态 */
#define MLX90615_I2C_SCL_READ()  ((MLX90615_I2C_SCL_PORT->IDR & MLX90615_I2C_SCL_PIN) != 0)	/* 读SCL口线状态 */


//----------------------------------------------------------------------------//



//============================================================================//


/*
********************************************************************************
*	函 数 名: Mlx90615_i2c_Delay
*	功能说明: I2C总线位延迟，最快400KHz
*	形    参:  无
*	返 回 值: 无
********************************************************************************
*/
static void Mlx90615_i2c_Delay(void)
{
    volatile uint8_t i;

	for (i = 0; i < 50; i++);
}


static void Mlx90615_i2c_Start(void)
{
	/* 当SCL高电平时，SDA出现一个下跳沿表示I2C总线启动信号 */
	MLX90615_I2C_SDA_1();
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_1();
	Mlx90615_i2c_Delay();

	MLX90615_I2C_SDA_0();
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_0();
//	Mlx90615_i2c_Delay();
}

/*******************************************************************************
 * 名    称： i2c_Stop
 * 功    能： CPU发起I2C总线停止信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注： 停止时序
 *            SCL _____/ˉˉˉˉˉˉˉ
 *            SDA _________/ˉˉˉˉˉ
 *                       |   |
 *                       STOP
 *******************************************************************************/
static void Mlx90615_i2c_Stop(void)
{
	/* 当SCL高电平时，SDA出现一个上跳沿表示I2C总线停止信号 */

	MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();
    MLX90615_I2C_SDA_0();
    Mlx90615_i2c_Delay();

	MLX90615_I2C_SCL_1();
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SDA_1();
//	Mlx90615_i2c_Delay();
}

/*******************************************************************************
 * 名    称： i2c_WaitAck
 * 功    能： CPU产生一个时钟，并读取器件的ACK应答信号
 * 入口参数： 无
 * 出口参数： 返回0表示正确应答，1表示无器件响应
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static uint8_t Mlx90615_i2c_WaitAck(void)
{
	uint8_t re;
    uint8_t TimeOutCnt = 20;  /* 超时计数器 */

	MLX90615_I2C_SDA_1();	/* CPU释放SDA总线 */
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_1();	/* CPU驱动SCL = 1, 此时器件会返回ACK应答 */
	Mlx90615_i2c_Delay();

    while(TimeOutCnt -- ) {
        if (MLX90615_I2C_SDA_READ())	{/* CPU读取SDA口线状态 */
            re = 1;
        } else {
            re = 0;
        }
    }
	MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();
	return re;
}

/*******************************************************************************
 * 名    称： i2c_Ack
 * 功    能： CPU产生一个ACK信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90615_i2c_Ack(void)
{
	MLX90615_I2C_SDA_0();	/* CPU驱动SDA = 0 */
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_1();	/* CPU产生1个时钟 */
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SDA_1();	/* CPU释放SDA总线 */
}

/*******************************************************************************
 * 名    称： i2c_NAck
 * 功    能： CPU产生1个NACK信号
 * 入口参数： 无
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90615_i2c_NAck(void)
{
	MLX90615_I2C_SDA_1();	/* CPU驱动SDA = 1 */
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_1();	/* CPU产生1个时钟 */
	Mlx90615_i2c_Delay();
	MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();
}

/*******************************************************************************
* 名    称： bsp_InitI2C
* 功    能： 配置I2C总线的GPIO，采用模拟IO的方式实现
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY
* 创建日期： 2018-06-29
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
static void Mlx90615_InitI2C(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(MLX90615_I2C_SCL_RCC | MLX90615_I2C_SDA_RCC, ENABLE);	/* 打开GPIO时钟 */

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;	        /* 开漏输出模式 */

	GPIO_InitStructure.GPIO_Pin = MLX90615_I2C_SCL_PIN;
	GPIO_Init(MLX90615_I2C_SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MLX90615_I2C_SDA_PIN;
	GPIO_Init(MLX90615_I2C_SDA_PORT, &GPIO_InitStructure);

	/* 给一个停止信号, 复位I2C总线上的所有设备到待机模式 */
	Mlx90615_i2c_Stop();
}

/*******************************************************************************
 * 名    称： i2c_SendByte
 * 功    能： CPU向I2C总线设备发送8bit数据
 * 入口参数： _ucByte ： 等待发送的字节
 * 出口参数： 无
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static void Mlx90615_i2c_SendByte(uint8_t _ucByte)
{
	uint8_t i;

	/* 先发送字节的高位bit7 */
	for (i = 0; i < 8; i++) {
        MLX90615_I2C_SCL_0();
		Mlx90615_i2c_Delay();

		if (_ucByte & 0x80) {
			MLX90615_I2C_SDA_1();
		} else {
			MLX90615_I2C_SDA_0();
		}

        _ucByte <<= 1;	/* 左移一个bit */

		Mlx90615_i2c_Delay();

		MLX90615_I2C_SCL_1();
		Mlx90615_i2c_Delay();
	}
    MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();

}

/*******************************************************************************
 * 名    称： Mlx90615_i2c_ReadByte
 * 功    能： CPU从I2C总线设备读取8bit数据
 * 入口参数： 无
 * 出口参数： 读到的数据
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
static uint8_t Mlx90615_i2c_ReadByte(void)
{
	uint8_t i;
	uint8_t value;

	/* 读到第1个bit为数据的bit7 */
	value = 0;
	for (i = 0; i < 8; i++) {
		value <<= 1;
		MLX90615_I2C_SCL_0();
		Mlx90615_i2c_Delay();
        MLX90615_I2C_SCL_1();
		Mlx90615_i2c_Delay();

		if (MLX90615_I2C_SDA_READ()) {
			value++;
		}
	}
    MLX90615_I2C_SCL_0();
	Mlx90615_i2c_Delay();

	return value;
}

/*******************************************************************************
 * 名    称： bsp_Mlx90615ScanDevice
 * 功    能： 检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
 * 入口参数：  _Address：设备的I2C总线地址
 * 出口参数： 返回值 0 表示正确， 返回1表示未探测到
 * 作　　者： Roger-WY
 * 创建日期： 2018-05-20
 * 修    改：
 * 修改日期：
 * 备    注：
 *******************************************************************************/
int8_t bsp_Mlx90615ScanDevice(uint8_t *pSlaveAddr)
{
	uint8_t ucAck;
    
    Mlx90615_InitI2C();
	if (MLX90615_I2C_SDA_READ() && MLX90615_I2C_SCL_READ()) {
        
        for(uint8_t i = 0; i < 128; i++)
        {
            Mlx90615_i2c_Start();		/* 发送启动信号 */
            Mlx90615_i2c_SendByte(i << 1);
            ucAck = Mlx90615_i2c_WaitAck();	/* 检测设备的ACK应答 */
            Mlx90615_i2c_Stop();			/* 发送停止信号 */
            if(ucAck == 0)
            {
                *pSlaveAddr = i;    //找到设备并获取从机地址
                return 0;
            }
        }
        return -1;          //没有找到设备

	}
	return -2;	            /* I2C总线异常 */
}

//============================================================================//
static const uint8_t crc_table[] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
    0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
    0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
    0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
    0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
    0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
    0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
    0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
    0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
    0xfa, 0xfd, 0xf4, 0xf3
};

uint8_t CRC8_Calc (uint8_t *p, uint8_t len) 
{
    uint16_t i;
    uint16_t crc = 0x0;

    while (len--) {
            i = (crc ^ *p++) & 0xFF;
            crc = (crc_table[i] ^ (crc << 8)) & 0xFF;
    }

    return (crc & 0xFF);
}

/*******************************************************************************
* 名    称： bsp_Mlx90615Init
* 功    能： Mlx90615传感器的初始化
* 入口参数： 无
* 出口参数： 无
* 作　　者： Roger-WY.
* 创建日期： 2018-08-08
* 修    改：
* 修改日期：
* 备    注：
*******************************************************************************/
int8_t bsp_Mlx90615Init(void)
{
    int8_t ucError      = 0;

    Mlx90615_InitI2C();

    return (ucError);

}

int8_t bsp_Mlx90615WriteReg(uint8_t devAddr, uint8_t regAddr, uint16_t data) 
{
    uint8_t ucAck = 0;
    uint8_t  i = 0;
    uint8_t crcSendBuffer[5] = {0x00,0x00,0x00,0x00,0x00}; //used to save the data to send
   
    crcSendBuffer[0] = devAddr << 1;
    crcSendBuffer[1] = regAddr;
    crcSendBuffer[2] =  (uint8_t)data;
    crcSendBuffer[3] =  (uint8_t)(data >> 8);
    crcSendBuffer[4] = CRC8_Calc(crcSendBuffer,4);
    
    Mlx90615_i2c_Start();
    
    for(i = 0; i < 5; i++)
    {
        Mlx90615_i2c_SendByte(crcSendBuffer[i]);
        ucAck = Mlx90615_i2c_WaitAck();
        if(ucAck) {         /* 如果Mlx90615，没有应答 */
            goto cmd_fail;	/* 器件无应答 */
        }
    }
    
	/* 发送I2C总线停止信号 */
	Mlx90615_i2c_Stop();
	return 0;	/* 执行成功 */

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	Mlx90615_i2c_Stop();
	return -1;

}

int8_t bsp_Mlx90615ReadReg(uint8_t devAddr, uint8_t regAddr,uint16_t *pReadData)
{
    uint8_t ucAck = 0;
    uint8_t ValBuf[6] = {0};
    uint8_t prcRegVal = 0;
    
    ValBuf[0] = devAddr << 1;
    ValBuf[1] = regAddr;
    ValBuf[2] = (devAddr << 1) | 0x01;
    

    Mlx90615_i2c_Start();
    Mlx90615_i2c_SendByte(ValBuf[0]);
    ucAck = Mlx90615_i2c_WaitAck();
    if(ucAck) {         /* 如果LM75X，没有应答 */
        goto cmd_fail;	/* 器件无应答 */
    }
    Mlx90615_i2c_SendByte(ValBuf[1]);
    ucAck = Mlx90615_i2c_WaitAck();
    if(ucAck) {         /* 如果LM75X，没有应答 */
        goto cmd_fail;	/* 器件无应答 */
    }    
    //------------------------------------------------------------------------//
    Mlx90615_i2c_Start();
    Mlx90615_i2c_SendByte(ValBuf[2]);
    ucAck = Mlx90615_i2c_WaitAck();
    if(ucAck) {         /* 如果LM75X，没有应答 */
        goto cmd_fail;	/* 器件无应答 */
    }

    ValBuf[3] = Mlx90615_i2c_ReadByte();
    Mlx90615_i2c_Ack();
    ValBuf[4] = Mlx90615_i2c_ReadByte();
    Mlx90615_i2c_Ack();
    ValBuf[5] = Mlx90615_i2c_ReadByte();
    Mlx90615_i2c_Ack();

	/* 发送I2C总线停止信号 */
	Mlx90615_i2c_Stop();

    prcRegVal = CRC8_Calc(ValBuf,5);
    if(prcRegVal == ValBuf[5])
    {
        *pReadData = (ValBuf[4] << 8) + ValBuf[3];
        return 0;	/* 执行成功 */
    }
    else
    {
        return -2;  //校验不正确
    }

cmd_fail: /* 命令执行失败后，切记发送停止信号，避免影响I2C总线上其他设备 */
	/* 发送I2C总线停止信号 */
	Mlx90615_i2c_Stop();
	return -1;
}

/* 读取环境温度 */
int8_t bsp_Mlx90615ReadTempAmbient(uint8_t slaveAddr, float *ao) 
{
    int8_t ret = 0;
    uint16_t data = 0;

	ret = bsp_Mlx90615ReadReg(slaveAddr,MLX90615_AMBIENT_TEMPERATURE,&data);

	if( 0 == ret)
    {
        *ao = (data * 0.02) - 273.15;
    }
    else
    {
        *ao = MLX90615_TEMP_READ_ERR_CODE;
    }

	return ret;
}

/* 读取目标温度 */
int8_t bsp_Mlx90615ReadTempObject(uint8_t slaveAddr, float *to) 
{
    int8_t ret = 0;
    uint16_t data = 0;
    
    ret = bsp_Mlx90615ReadReg(slaveAddr,MLX90615_OBJECT_TEMPERATURE,&data);
	if( 0 == ret)
    {
        *to = (data * 0.02) - 273.15;
    }
    else
    {
        *to = MLX90615_TEMP_READ_ERR_CODE;
    }

	return ret;
}

int8_t bsp_Mlx90615ReadIrData(uint8_t slaveAddr, uint16_t *ir)
{
    int8_t ret = 0;
    uint16_t data = 0;
    ret = bsp_Mlx90615ReadReg(slaveAddr,MLX90615_RAW_IR_DATA,&data);
    if(ret == 0)
    {
        *ir = data;
    }
    else
    {
        *ir = (uint16_t)MLX90615_TEMP_READ_ERR_CODE;
    }
    return (ret);
}

int8_t bsp_Mlx90615ReadEmissivity(uint8_t slaveAddr, float *emissivity)
{
    int8_t ret = 0;
    uint16_t data = 0;
    ret = bsp_Mlx90615ReadReg(slaveAddr,MLX90615_EEPROM_EMISSIVITY,&data);
    
    if(data < 32768)
    {
        *emissivity = (float)data / 0x4000;
    } 
    else
    {
        *emissivity = (32768 - (float)data) / 0x4000;   
    }
    
    return (ret);
}

int8_t bsp_Mlx90615SetEmissivity(uint8_t slaveAddr, float emissivity)
{
    int8_t ret = 0;
    uint16_t newE = 0;
    
    float temp = 0;
    
    if(emissivity > 1.0 || emissivity < 0.05)
    {
        return -6;
    }
    
    temp = emissivity * 0x4000 + 0.5;
    newE = (uint16_t)temp;
    
    ret = bsp_Mlx90615WriteReg(slaveAddr, MLX90615_EEPROM_EMISSIVITY, 0x0000);
    
    if(ret == 0)
    {
        ret = bsp_Mlx90615WriteReg(slaveAddr, MLX90615_EEPROM_EMISSIVITY, newE);       
    }     
    
    
    return (ret);
}

/***************************** (END OF FILE) **********************************/
