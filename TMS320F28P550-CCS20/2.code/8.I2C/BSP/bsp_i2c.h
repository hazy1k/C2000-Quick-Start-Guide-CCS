#ifndef BSP_I2C_H
#define BSP_I2C_H

#include "board.h"

/************************ I2C ************************/
//设置SDA输出模式
#define SDA_OUT()   {   GPIO_setDirectionMode(I2C_SDA, GPIO_DIR_MODE_OUT);}
//设置SDA输入模式
#define SDA_IN()    { 	GPIO_setDirectionMode(I2C_SDA, GPIO_DIR_MODE_IN); }

//获取SDA引脚的电平变化
#define SDA_GET()   ( GPIO_readPin(I2C_SDA) )
//SDA与SCL输出
#define SDA(x)      ( (x) ? (GPIO_writePin(I2C_SDA,1)) : (GPIO_writePin(I2C_SDA,0)) )
#define SCL(x)      ( (x) ? (GPIO_writePin(I2C_SCL,1)) : (GPIO_writePin(I2C_SCL,0)) )

void i2c_start(void);
void i2c_stop(void);
void i2c_send_ack(unsigned char ack);
unsigned char i2c_wait_ack(void);
void i2c_send_byte(uint8_t dat);
unsigned char i2c_read_byte_ack(unsigned char ack);

#endif
