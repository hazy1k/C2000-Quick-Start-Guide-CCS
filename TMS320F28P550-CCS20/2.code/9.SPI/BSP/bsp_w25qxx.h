#ifndef _BSP_W25QXX_H__
#define _BSP_W25QXX_H__

#include "board.h"

//CS引脚的输出控制
//x=0时输出低电平
//x=1时输出高电平
#define SPI_CS(x)  ( (x) ? GPIO_writePin(FLASH_CS,1) : GPIO_writePin(FLASH_CS,0) )

uint8_t spi_read_write_byte(uint8_t dat);//SPI读写一个字节
uint16_t w25qxx_read_id(void);//读取W25QXX的ID
void w25qxx_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte);      //W25QXX写数据
void w25qxx_read(uint8_t* buffer,uint32_t read_addr,uint16_t read_length);//W25QXX读数据

#endif
