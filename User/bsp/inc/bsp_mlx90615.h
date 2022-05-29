/*******************************************************************************
*   Filename:       bsp_mlx90615.h
*   Revised:        All copyrights reserved to Roger.
*   Date:           2022-05-04
*   Revision:       v1.0
*   Writer:	        Roger-WY.
*
*   Description:    非接触式红外温度传感器模块驱动(数字式)  头文件
*******************************************************************************/
#ifndef __BSP_MLX90615_H__
#define __BSP_MLX90615_H__

#include "stm32f10x.h"
#include "bsp_timer.h"

#define MLX90615_TEMP_READ_ERR_CODE -99

/* MLX90615 内部寄存器地址定义 */
/* Private defines -----------------------------------------------------------*/
/* DEFAULT SLAVE ADDRESS */
#define MLX90615_ADDR       0x5B
#define MLX90615_ADDR_WR   (MLX90615_ADDR << 1)


#define MLX90615_EEPROM_SA              0x10
#define MLX90615_EEPROM_PWMT_MIN        MLX90615_EEPROM_SA
#define MLX90615_EEPROM_PWMT_RNG        0x11
#define MLX90615_EEPROM_CONFIG          0x12
#define MLX90615_EEPROM_EMISSIVITY      0x13

#define MLX90615_RAW_IR_DATA            0x25
#define MLX90615_AMBIENT_TEMPERATURE    0x26
#define MLX90615_OBJECT_TEMPERATURE     0x27

#define MLX90615_SLEEP	                0xC6

// DEPRECATED! (just emissivity, not the whole EEPROM)
#define AccessEEPROM                    MLX90615_EEPROM_EMISSIVITY

#define Default_Emissivity              0x4000



/* MLX90615 供外部调用函数 */
int8_t bsp_Mlx90615Init(void);
int8_t bsp_Mlx90615ScanDevice(uint8_t *pSlaveAddr);
int8_t bsp_Mlx90615ReadTempAmbient(uint8_t slaveAddr, float *ao);
int8_t bsp_Mlx90615ReadTempObject(uint8_t slaveAddr, float *to);
int8_t bsp_Mlx90615ReadIrData(uint8_t slaveAddr, uint16_t *ir);
int8_t bsp_Mlx90615ReadEmissivity(uint8_t slaveAddr, float *emissivity);
int8_t bsp_Mlx90615SetEmissivity(uint8_t slaveAddr, float emissivity);
#endif
/***************************** (END OF FILE) **********************************/
