# 第九章 SPI介绍及基础使用

## 1. F28P550的硬件I2C

TMS320F28P550中的串行外设接口 (SPI) 外设支持以下主要特性：

- SPIPOCI：SPI 外设输出/控制器输入引脚
- SPIPICO：SPI 外设输入/控制器输出引脚
- SPIPTE：SPI 外设发送使能引脚
- SPICLK：SPI 串行时钟引脚
- 两种工作模式：控制器和外设
- 波特率：125 个不同的可编程速率。可采用的最大波特率受限于 SPI 引脚上使用的 I/O 缓冲器的最大速度。
- 数据字长度：1 至 16 数据位
- 四种时钟方案（由时钟极性和时钟相位的位控制）包含：
  - 无相位延迟的下降沿：SPICLK 高电平有效。SPI 在 SPICLK 信号的下降沿上发送数据，在 SPICLK 信号的 上升沿上接收数据。
  - 有相位延迟的下降沿：SPICLK 高电平有效。SPI 在 SPICLK 信号下降沿提前半个周期发送数据，在 SPICLK 信号的下降沿上接收数据。
  - 无相位延迟的上升沿：SPICLK 低电平无效。SPI 在 SPICLK 信号的上升沿上发送数据，在 SPICLK 信号的 下降沿上接收数据。
  - 有相位延迟的上升沿：SPICLK 低电平无效。SPI 在 SPICLK 信号上升沿的半个周期之前发送数据，而在 SPICLK 信号的上升沿上接收数据。
- 同时接收和发送操作（可在软件中禁用发送功能）
- 发送器和接收器操作通过中断驱动或轮询算法完成
- 16 级发送/接收 FIFO
- DMA 支持
- 高速模式
- 延迟的发送控制
- 3 线 SPI 模式
- 在带有两个 SPI 模块的器件上实现数字音频接口接收模式的 SPIPTE 反转

**使用硬件SPI的优势：**

**支持中断和DMA：** 硬件SPI可以与中断控制器和DMA控制器配合使用，实现数据的高效处理和传输。

**硬件缓冲区：** 硬件SPI具有内部缓冲区，可以在主机和外设之间进行数据中转，提高数据的传输效率。

**高速传输：** 硬件SPI使用硬件模块进行数据传输，速度通常比软件实现的SPI更快。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/spi/spi_20250526_155732.png)

## 2. W25Q32介绍

W25Q32是一种常见的串行闪存器件，它采用SPI（Serial Peripheral Interface）接口协议，具有高速读写和擦除功能，可用于存储和读取数据。W25Q32芯片容量为32 Mbit（4 MB），其中名称后的数字代表不同的容量选项。不同的型号和容量选项可以满足不同应用的需求，比如W25Q16、W25Q64、W25Q128等。通常被用于嵌入式设备、存储设备、路由器等高性能电子设备中。 W25Q32闪存芯片的内存分配是按照扇区（Sector）和块（Block）进行的，每个扇区的大小为4KB，每个块包含16个扇区，即一个块的大小为64KB。

![](https://wiki.lckfb.com/storage/images/zh-hans/dmx/ccs-beginner/spi/spi_20240823_110150.png)

W25Q32存储芯片，其引脚的说明，见下表。

| CLK  | 从外部获取时间，为输入输出功能提供时钟                                                            |
| ---- | ------------------------------------------------------------------------------ |
| DI   | 标准SPI使用单向的DI，来串行的写入指令，地址，或者数据到FLASH中，在时钟的上升沿。                                  |
| DO   | 标准SPI使用单向的DO，来从处于下降边沿时钟的设备，读取数据或者状态。                                           |
| WP   | 防止状态寄存器被写入                                                                     |
| HOLD | 当它有效时允许设备暂停，低电平：DO引脚高阻态，DI CLK引脚的信号被忽略。高电平：设备重新开始，当多个设备共享相同的SPI信 号的时候该功能可能会被用到 |
| CS   | CS高电平的时候其他引脚成高阻态，处于低电平的时候，可以读写数据                                               |

它与开发板的连接如下：

| 开发板（主机） | W25Q32（从机）    | 说明        |
| ------- | ------------- | --------- |
| GPIO0   | CS(NSS)       | 片选线       |
| GPIO3   | CLK           | 时钟线       |
| GPIO1   | DO(IO1)(MISO) | 主机输入从机输出线 |
| GPIO2   | DI(IO0)(MOSI) | 主机输出从机输入线 |
| GND     | GND           | 电源线       |
| VCC     | 3V3           | 电源线       |

发板上默认已经为大家贴好了该存储芯片，大家只需要了解连接的是哪一个引脚即可。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/spi/spi_20250526_160059.png)

需要注意的是，我们使用的是硬件SPI方式驱动W25Q32，因此我们需要确定我们设置的引脚是否有硬件SPI外设接口。在数据手册中，GPIO0 ~ GPIO3 可以复用为SPIA的4根通信线。

这里需要注意的是PICO，表示的是外设输入控制器输出，即对应SPI中的MOSI。POCI表示外设输出控制器输入，即对应MISO。

## 3. 软件设计

### 3.1 SPI配置

打开工程下的 .syscfg 文件。找到 SPI 选项开始配置：

- 在SPI外设的配置中，重新命名叫 FLASH_SPI；

- 设置传输协议为 mode 0；

- 设置SPI模式为主机模式，即控制器模式；

- 设置通信速率为 1MHz；

- 设置参数数据位长度为 8位；

- 设置引脚配置自定义，关闭 PTE 的配置，然后设置各引脚。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/spi/spi_20250526_161011.png)

这里需要注意，由于大多数的SPI协议中，整个时序里在发送接收时片选是要一直拉低的，而SPI外设的片选在每次发送和接收完一帧后会拉高，所以CS片选线需要用MCU的IO口独立控制，没有办法使用SPI外设的CS管脚。**这里使用GPIO的方式（软件方式）去控制CS引脚的输出。**

新建一个GPIO。命名为FLASH_CS，引脚选择我们现在接入模块CS的引脚GPIO0。其配置如下：

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/spi/spi_20250526_161357.png)

### 3.2 W25Q32驱动

```c
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

```

```c
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

```

### 3.3 主函数测试

```c
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "bsp_w25qxx.h"
#include <stdio.h>
#include <string.h>

void main(void)
{

    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Board_init();
    C2000Ware_libraries_init();
    EINT;
    ERTM;

    int flash_id=0;
    char read_write_buff[10] = {0};
    char read_write_buff2[10] = {0};

    //读取器件ID
    flash_id = w25qxx_read_id();
    DEVICE_DELAY_US(500000);
    printf("flash_id=%X\r\n",flash_id);

    //往0地址写入5个字节数据,分别是"12345"
    w25qxx_write("12345", 0x00, 5);
    DEVICE_DELAY_US(500000);

    //读取0地址的5个字节数据到buff
    w25qxx_read(read_write_buff, 0x00, 5);
    //通过CIO输出
    printf("%s\r\n",read_write_buff);
    DEVICE_DELAY_US(500000);

    /* 字符串测试 */
    w25qxx_write("hello", 0x01, sizeof(read_write_buff2));
    DEVICE_DELAY_US(500000);
    w25qxx_read(read_write_buff2, 0x01, sizeof(read_write_buff2));
    printf("%s\r\n",read_write_buff2);
    DEVICE_DELAY_US(500000);

    while(1);
}

```

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/spi/spi_20250526_165122.png)


