#include "bsp_w25qxx.h"
#include "spi.h"

uint8_t spi_read_write_byte(uint8_t dat)
{
        uint8_t data = 0;

        // Transmit data
        SPI_writeDataNonBlocking(FLASH_SPI_BASE, dat<<8);

        // Block until data is received and then return it
        data = SPI_readDataBlockingNonFIFO(FLASH_SPI_BASE);

        return data;
}

//读取芯片ID
//返回值如下:
//0XEF13,表示芯片型号为W25Q80
//0XEF14,表示芯片型号为W25Q16
//0XEF15,表示芯片型号为W25Q32
//0XEF16,表示芯片型号为W25Q64
//0XEF17,表示芯片型号为W25Q128
//读取设备ID
uint16_t w25qxx_read_id(void)
{
    uint16_t  temp = 0;
    //将CS端拉低为低电平
    SPI_CS(0);
    //发送指令90h
    spi_read_write_byte(0x90);//发送读取ID命令
    //发送地址  000000H
    spi_read_write_byte(0x00);
    spi_read_write_byte(0x00);
    spi_read_write_byte(0x00);

    //接收数据
    //接收制造商ID
    temp |= spi_read_write_byte(0xFF)<<8;
    //接收设备ID
    temp |= spi_read_write_byte(0xFF);
    //恢复CS端为高电平
    SPI_CS(1);
    //返回ID
    return temp;
}

//发送写使能
void w25qxx_write_enable(void)
{
    //拉低CS端为低电平
    SPI_CS(0);
    //发送指令06h
    spi_read_write_byte(0x06);
    //拉高CS端为高电平
    SPI_CS(1);
}
//器件忙判断
void w25qxx_wait_busy(void)
{
    unsigned char byte = 0;
    do
     {
        //拉低CS端为低电平
        SPI_CS(0);
        //发送指令05h
        spi_read_write_byte(0x05);
        //接收状态寄存器值
        byte = spi_read_write_byte(0Xff);
        //恢复CS端为高电平
        SPI_CS(1);
     //判断BUSY位是否为1 如果为1说明在忙，重新读写BUSY位直到为0
     }while( ( byte & 0x01 ) == 1 );
}

void w25qxx_erase_sector(uint32_t addr)
{
        //计算扇区号，一个扇区4KB=4096
        addr *= 4096;
        w25qxx_write_enable();  //写使能
        w25qxx_wait_busy();     //判断忙，如果忙则一直等待
        //拉低CS端为低电平
        SPI_CS(0);
        //发送指令20h
        spi_read_write_byte(0x20);
        //发送24位扇区地址的高8位
        spi_read_write_byte((uint8_t)((addr)>>16));
        //发送24位扇区地址的中8位
        spi_read_write_byte((uint8_t)((addr)>>8));
        //发送24位扇区地址的低8位
        spi_read_write_byte((uint8_t)addr);
        //恢复CS端为高电平
        SPI_CS(1);
        //等待擦除完成
        w25qxx_wait_busy();
}

void w25qxx_write(uint8_t* buffer, uint32_t addr, uint16_t numbyte)
{
    unsigned int i = 0;
    //擦除扇区数据
    w25qxx_erase_sector(addr/4096);
    //写使能
    w25qxx_write_enable();
    //忙检测
    w25qxx_wait_busy();
    //写入数据
    //拉低CS端为低电平
    SPI_CS(0);
    //发送指令02h
    spi_read_write_byte(0x02);
    //发送写入的24位地址中的高8位
    spi_read_write_byte((uint8_t)((addr)>>16));
    //发送写入的24位地址中的中8位
    spi_read_write_byte((uint8_t)((addr)>>8));
    //发送写入的24位地址中的低8位
    spi_read_write_byte((uint8_t)addr);
    //根据写入的字节长度连续写入数据buffer
    for(i=0;i<numbyte;i++)
    {
        spi_read_write_byte(buffer[i]);
    }
    //恢复CS端为高电平
    SPI_CS(1);
    //忙检测
    w25qxx_wait_busy();
}

void w25qxx_read(uint8_t* buffer,uint32_t read_addr,uint16_t read_length)
{
        uint16_t i;
        //拉低CS端为低电平
        SPI_CS(0);
        //发送指令03h
        spi_read_write_byte(0x03);
        //发送24位读取数据地址的高8位
        spi_read_write_byte((uint8_t)((read_addr)>>16));
        //发送24位读取数据地址的中8位
        spi_read_write_byte((uint8_t)((read_addr)>>8));
        //发送24位读取数据地址的低8位
        spi_read_write_byte((uint8_t)read_addr);
        //根据读取长度读取出地址保存到buffer中
        for(i=0;i<read_length;i++)
        {
            buffer[i]= spi_read_write_byte(0XFF);
        }
        //恢复CS端为高电平
        SPI_CS(1);
}
