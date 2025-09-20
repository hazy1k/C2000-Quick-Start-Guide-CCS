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

## 4. C2000 SPI 模块相关函数总结 (基于DriverLib)

> **注**：本总结以TMS320F2837x/Dx系列为例。C2000的SPI模块是一个高度可配置的外设，支持主从模式、多种时钟极性和相位，并通常包含FIFO，以实现高效的数据传输。

### 4.1 SPI模块概述与核心概念

SPI总线通常使用四根线：主出从入（MOSI）、主入从出（MISO）、串行时钟（SPICLK）和从机选择（SPISTE）。它通过简单的移位寄存器原理实现全双工通信。

- **核心通信流程**：  
  **主设备产生SPICLK** -> **数据在主从设备的移位寄存器中同步移入移出**

- **主要组件与特性说明**：
  
  | **组件/特性**      | **功能**                    | **C2000特点**                   |
  | -------------- | ------------------------- | ----------------------------- |
  | **时钟发生器**      | 产生SPICLK时钟。在主模式下，时钟频率可配置。 | 时钟源为LSPCLK，通过分频器产生目标SPICLK频率。 |
  | **数据寄存器和FIFO** | 暂存要发送或接收的数据。              | 包含TX和RX FIFO，可减少中断开销，实现连续数据流。 |
  | **操作模式**       | 支持主模式(Master)和从模式(Slave)。 | 可动态配置，但一次通信中角色固定。             |
  | **时钟格式**       | 通过极性和相位控制数据采样和锁存的边沿。      | 可配置CPOL和CPHA，以匹配各种SPI从设备的要求。  |
  | **中断系统**       | 标志通信事件（发送空、接收满、超时等）。      | 通过PIE管理，可灵活使能/禁止不同中断源。        |
  | **DMA支持**      | 可与DMA控制器联动，自动传输大量数据。      | 极大减轻CPU负担，是实现高速数据流的关键。        |

### 4.2 SPI初始化与配置

- **核心配置流程**（四步关键操作）：
  
  1. **使能SPI外设时钟**（配置LSPCLK，通常已在时钟初始化中完成）
  
  2. **配置GPIO复用为SPI功能**（将特定引脚设置为SIMO/SOMI/CLK/STE）
  
  3. **初始化SPI模块参数**（模式、时钟格式、波特率）
  
  4. **（可选）配置中断和FIFO**

- **基础配置示例**（配置SPI-A为主机，CPOL=0 CPHA=0，1Mbps速率）：
  
  ```c
  #include "driverlib.h"
  
  // 假设LSPCLK = 50MHz (SYSCLK=200MHz / LSPCLKDIV=4)
  #define LSPCLK_FREQ_HZ  50000000
  #define SPI_BITRATE     1000000   // 1 Mbps
  
  void main(void) {
      // 1. 初始化器件、GPIO
      Device_init();
      Device_initGPIO();
      // ... 确保LSPCLK已正确分频 ...
  
      // 2. 配置GPIO引脚为SPI功能 (以SPIA为例)
      // GPIO16: SPISIMOA (主出从入)
      // GPIO17: SPISOMIA (主入从出)
      // GPIO18: SPICLKA  (串行时钟)
      // GPIO19: SPISTEA  (从机选择，可选)
      GPIO_setPinConfig(GPIO_16_SPISIMOA);
      GPIO_setPinConfig(GPIO_17_SPISOMIA);
      GPIO_setPinConfig(GPIO_18_SPICLKA);
      // 如果需要手动控制片选，SPISTE可配置为普通GPIO
      GPIO_setPinConfig(GPIO_19_GPIO19);
      GPIO_setDirectionMode(19, GPIO_DIR_MODE_OUT);
      GPIO_writePin(19, 1); // 默认拉高，不选中
  
      // 3. 初始化SPI模块：配置为主机、设置波特率和时钟格式
      SPI_disableModule(SPIA_BASE); // 先禁用模块
      SPI_setConfig(SPIA_BASE, LSPCLK_FREQ_HZ, SPI_PROT_POL0PHA0, SPI_MODE_MASTER, SPI_BITRATE, SPI_DATA_LEN_8);
      SPI_enableModule(SPIA_BASE);  // 使能SPI模块
  
      // 4. （可选）使能FIFO
      SPI_enableFIFO(SPIA_BASE);
      SPI_clearRxFIFO(SPIA_BASE);
      SPI_clearTxFIFO(SPIA_BASE);
  
      // 后续进行数据收发操作...
  }
  ```

- **关键配置函数详解**：
  
  | **函数 (DriverLib)**                                          | **说明**                    | **参数示例与解释**                                                                                                                                                                                                                                 |
  | ----------------------------------------------------------- | ------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
  | `SPI_setConfig(base, lspclk, prot, mode, bitrate, wordlen)` | **SPI核心配置函数**。            | `lspclk`: LSPCLK频率 (Hz)<br>`prot`: 时钟格式 (`SPI_PROT_POL0PHA0`, `POL0PHA1`, `POL1PHA0`, `POL1PHA1`)<br>`mode`: `SPI_MODE_MASTER` 或 `SPI_MODE_SLAVE`<br>`bitrate`: 目标SPICLK频率 (bps)<br>`wordlen`: `SPI_DATA_LEN_8` 或 `SPI_DATA_LEN_16` (数据位长度) |
  | `SPI_disableModule(base)` / `enableModule`                  | **禁用/使能SPI模块**。           | 配置前必须先禁用模块。                                                                                                                                                                                                                                 |
  | `SPI_enableFIFO(base)` / `disableFIFO`                      | **使能/禁用FIFO功能**。**推荐使能**。 |                                                                                                                                                                                                                                             |
  | `SPI_clearRxFIFO(base)` / `clearTxFIFO`                     | **清除RX/TX FIFO指针**。       | 使能FIFO后或通信开始前应调用。                                                                                                                                                                                                                           |
  | `SPI_setEmulationMode(base, mode)`                          | **设置仿真挂起时的行为**。           | `SPI_EMULATION_STOP_AT_NEXT_CHAR` (推荐)，防止调试时SPI失控。                                                                                                                                                                                          |

- **时钟格式（CPOL与CPHA）**：
  
  | **模式** | **宏**               | **CPOL** | **CPHA** | **时钟空闲状态** | **数据采样边沿**     |
  | ------ | ------------------- | -------- | -------- | ---------- | -------------- |
  | Mode 0 | `SPI_PROT_POL0PHA0` | 0        | 0        | Low        | Rising (奇数边沿)  |
  | Mode 1 | `SPI_PROT_POL0PHA1` | 0        | 1        | Low        | Falling (偶数边沿) |
  | Mode 2 | `SPI_PROT_POL1PHA0` | 1        | 0        | High       | Falling (奇数边沿) |
  | Mode 3 | `SPI_PROT_POL1PHA1` | 1        | 1        | High       | Rising (偶数边沿)  |
  
  - **必须根据从设备的数据手册选择正确的模式**，否则无法正常通信。

### 4.3 SPI数据收发函数

SPI是全双工的，读写通常同时发生。

- **阻塞式函数 (Polling)**：适合简单应用或单次传输。
  
  | **函数**                                  | **原型**         | **说明**                            |
  | --------------------------------------- | -------------- | --------------------------------- |
  | `SPI_transmitData(base, data)`          | `(base, data)` | **阻塞式发送一个数据**。等待TXFIFO有空位后写入。     |
  | `uint16_t SPI_receiveData(base)`        | `(base)`       | **阻塞式接收一个数据**。等待RXFIFO有数据后读取。     |
  | `SPI_writeDataNonBlocking(base, data)`  | `(base, data)` | **非阻塞式发送**。仅当TXFIFO有空间时写入。        |
  | `int16_t SPI_readDataNonBlocking(base)` | `(base)`       | **非阻塞式接收**。仅当RXFIFO有数据时读取，否则返回-1。 |

- **操作示例**：
  
  ```c
  // 1. 基本的全双工通信（发送一字节的同时接收一字节）
  GPIO_writePin(19, 0); // 拉低片选，选中从设备
  SPI_transmitData(SPIA_BASE, 0x55);       // 发送0x55
  uint16_t receivedData = SPI_receiveData(SPIA_BASE); // 读取接收到的数据
  GPIO_writePin(19, 1); // 拉高片选，通信结束
  
  // 2. 连续传输（利用FIFO）
  uint16_t i;
  uint16_t txBuffer[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
  uint16_t rxBuffer[5] = {0};
  
  GPIO_writePin(19, 0);
  for (i = 0; i < 5; i++) {
      SPI_transmitData(SPIA_BASE, txBuffer[i]); // 连续发送5个数据
  }
  for (i = 0; i < 5; i++) {
      rxBuffer[i] = SPI_receiveData(SPIA_BASE); // 连续读取5个数据
  }
  GPIO_writePin(19, 1);
  ```

### 4.4 高级功能与特性

#### 4.4.1 中断模式

使用中断可以高效处理连续的数据流。

```c
// 配置SPI在发送FIFO为空或接收FIFO有数据时产生中断
void initSPIInterrupt(void) {
    // 1. 使能特定的SPI中断源
    SPI_enableInterrupt(SPIA_BASE, SPI_INT_RX_OVRFLOW | SPI_INT_RX_READY | SPI_INT_TX_READY);

    // 2. 设置FIFO中断触发级别
    SPI_setFIFOInterruptLevel(SPIA_BASE, SPI_FIFO_TXEMPTY, SPI_FIFO_RX4); // TX空，RX有4个数据

    // 3. 注册PIE中断服务函数 (SPI-A中断通常属于INT2.10)
    Interrupt_register(INT_SPIA, &spiAISR);
    Interrupt_enable(INT_SPIA);

    // 4. 全局使能中断
    Interrupt_globalEnable(TRUE);
}

// SPI中断服务函数
__interrupt void spiAISR(void) {
    uint16_t intStatus = SPI_getInterruptStatus(SPIA_BASE);

    if (intStatus & SPI_INT_TX_READY) {
        // TX FIFO有空位，可以填充下一个数据
        // SPI_writeDataNonBlocking(SPIA_BASE, nextTxData);
    }
    if (intStatus & SPI_INT_RX_READY) {
        // RX FIFO有数据，可以读取
        // newRxData = SPI_readDataNonBlocking(SPIA_BASE);
    }
    if (intStatus & SPI_INT_RX_OVRFLOW) {
        // RX FIFO溢出，处理错误
        SPI_clearRxFIFO(SPIA_BASE); // 清除溢出
    }
    // 清除PIE中断标志
    Interrupt_clear(INT_SPIA);
    // SPI模块的中断标志通常通过读写相应寄存器自动清除
}
```

#### 4.4.2 从机模式

配置SPI为从设备，由外部主设备提供时钟。

```c
// 初始化SPI为从模式
SPI_disableModule(SPIA_BASE);
SPI_setConfig(SPIA_BASE, LSPCLK_FREQ_HZ, SPI_PROT_POL0PHA0, SPI_MODE_SLAVE, 0, SPI_DATA_LEN_8);
// 从模式下的bitrate参数被忽略，因为时钟由主设备提供
SPI_enableModule(SPIA_BASE);

// 配置从机选择的引脚（SPISTE），通常硬件自动控制
// 如果需要，也可以配置为软件控制

// 在中断中响应主机的数据请求
__interrupt void spiSlaveISR(void) {
    if (SPI_getInterruptStatus(SPIA_BASE) & SPI_INT_RX_READY) {
        uint16_t cmd = SPI_readDataNonBlocking(SPIA_BASE); // 读取主设备发来的命令
        // 根据命令准备要返回的数据
        SPI_writeDataNonBlocking(SPIA_BASE, dataToSend);
    }
}
```

#### 4.4.3 DMA支持

对于需要极高吞吐量的应用（如音频流、图像传输），DMA是必不可少的。

```c
// 配置DMA用于SPI发送（简化流程）
// 1. 设置DMA源地址（内存中的数组），目的地址（SPI数据寄存器）
// 2. 设置DMA传输大小和触发源（SPI TX中断）
// 3. 启动SPI传输（通常需要先手动写入第一个数据以启动时钟）
// 4. 后续的数据传输将由DMA自动完成，并在完成后产生DMA中断

// 具体DMA配置代码较为复杂，需参考芯片的DMA章节和示例
// 思路是：DMA与SPI的TX/RX FIFO联动，实现“数据自动搬运”
```

### 4.5 使用示例（读写SPI Flash）

#### 示例：向W25Q32 Flash芯片发送读ID命令并读取ID

```c
#include "driverlib.h"

#define SPI_FLASH_CS_GPIO  19
#define W25Q_CMD_RDID      0x9F // Read Identification命令

uint32_t SPI_ReadFlashID(void) {
    uint8_t txBuffer[4] = {0};
    uint8_t rxBuffer[4] = {0};
    uint32_t flashID = 0;

    // 1. 构建发送缓冲区：命令 + 3个哑元字节(Dummy)用于读取响应
    txBuffer[0] = W25Q_CMD_RDID;
    txBuffer[1] = 0x00; // Dummy
    txBuffer[2] = 0x00; // Dummy
    txBuffer[3] = 0x00; // Dummy

    // 2. 拉低片选，开始通信
    GPIO_writePin(SPI_FLASH_CS_GPIO, 0);

    // 3. 全双工传输：发送4字节，同时接收4字节
    for (uint16_t i = 0; i < 4; i++) {
        SPI_transmitData(SPIA_BASE, txBuffer[i]);
        rxBuffer[i] = (uint8_t)SPI_receiveData(SPIA_BASE);
    }

    // 4. 拉高片选，结束通信
    GPIO_writePin(SPI_FLASH_CS_GPIO, 1);

    // 5. 解析响应：ID通常在返回的第2、3、4字节
    flashID = (rxBuffer[1] << 16) | (rxBuffer[2] << 8) | rxBuffer[3];
    return flashID;
}

void main(void) {
    // ... 初始化SPI和GPIO ...

    uint32_t manufacturerID = SPI_ReadFlashID();
    // manufacturerID 应包含制造商和设备信息 (如0xEF4015 for Winbond W25Q32)
}
```

## 5. 关键注意事项

1. **时钟格式匹配**：**必须确保主从设备的CPOL和CPHA设置完全一致**。这是SPI通信成功的最关键因素。始终从设备的数据手册中查找其所需的模式。

2. **片选信号管理**：
   
   - 可以使用硬件自动控制的`SPISTE`引脚，但更常见的是使用**普通GPIO手动控制**，因为它提供了更大的灵活性（如支持多从设备）。
   
   - 片选信号应在**整个数据传输序列期间保持有效（低电平）**，并在序列结束后拉高。

3. **波特率设置**：主模式下，SPICLK频率由`LSPCLK`和分频器决定。确保计算出的波特率在从设备支持的范围内。从模式下，该参数被忽略。

4. **全双工特性**：记住SPI是全双工的。每次调用`SPI_transmitData()`都会启动一个时钟周期，同时也会接收一个数据。**必须读取`SPI_receiveData()`来获取对方发回的数据**，即使你并不关心它，否则RXFIFO会溢出。

5. **FIFO的使用**：**强烈建议始终使能FIFO**。它允许你在一次中断中处理多个数据，大幅提高通信效率，并降低因中断延迟导致数据丢失的风险。

6. **调试技巧**：使用逻辑分析仪或示波器检查SPI线上的波形（CLK, MOSI, MISO, CS）是调试SPI问题的最有效方法。可以直观地验证时钟极性、相位、数据内容和时序是否正确。

---


