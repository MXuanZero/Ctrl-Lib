#ifndef __I2C_SOFT_H__
#define __I2C_SOFT_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "gpio.h"
#include "stm32f1xx_hal.h"

#define CPU_FREQUENCY_MHZ 1  // 4
#define MYI2C_SCL_PIN     GPIO_PIN_12
#define MYI2C_SCL_PORT    GPIOB
#define MYI2C_SDA_PIN     GPIO_PIN_13
#define MYI2C_SDA_PORT    GPIOB

#define SDA_Dout_LOW()  HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, GPIO_PIN_RESET)
#define SDA_Dout_HIGH() HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, GPIO_PIN_SET)
#define SDA_Data_IN()   HAL_GPIO_ReadPin(MYI2C_SDA_PORT, MYI2C_SDA_PIN)
#define SCL_Dout_LOW()  HAL_GPIO_WritePin(MYI2C_SCL_PORT, MYI2C_SCL_PIN, GPIO_PIN_RESET)
#define SCL_Dout_HIGH() HAL_GPIO_WritePin(MYI2C_SCL_PORT, MYI2C_SCL_PIN, GPIO_PIN_SET)
#define SCL_Data_IN()   HAL_GPIO_ReadPin(MYI2C_SCL_PORT, MYI2C_SCL_PIN)
#define SDA_Write(XX)   HAL_GPIO_WritePin(MYI2C_SDA_PORT, MYI2C_SDA_PIN, ((XX) ? GPIO_PIN_SET : GPIO_PIN_RESET))

void Soft_I2C_Init(void);
void Soft_I2C_Start(void);
void Soft_I2C_Stop(void);
void Soft_I2C_Send_Byte(uint8_t txd);
uint8_t Soft_I2C_Read_Byte(uint8_t ack);
void Soft_I2C_NAck(void);
void Soft_I2C_Ack(void);
uint8_t Soft_I2C_Wait_Ack(void);
void delay_xus(__IO uint32_t nTime);

/* i2c api*/
int8_t Soft_I2cMaster_Init(void);
int8_t Soft_I2cMaster_TransmitAck(uint8_t dev_addr, const uint8_t *buff, uint8_t len);
int8_t Soft_I2cMaster_TransmitNack(uint8_t dev_addr, const uint8_t *buff, uint8_t len);
int8_t Soft_I2cMaster_ReceiveNack(uint8_t reg_addr, uint8_t *buff, uint8_t len);

#define Delay_us(xx) delay_xus(xx)

#ifdef __cplusplus
}
#endif
#endif
