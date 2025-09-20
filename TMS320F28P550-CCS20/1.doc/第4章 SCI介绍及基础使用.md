# 第四章 SCI介绍及基础使用

## 1. F28P550的串口介绍

TMS320F28P550集成了三个高性能串行通信接口（SCI），分别是SCI-A、SCI-B、SCI-C，它们支持全双工异步通信，每个接口独立工作。在开发板上的支持串口的引脚见下方的开发板引脚示意图：（橙色的SCI）

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214542.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214600.png)

串口​​通信参数

- ​波特率范围​​：最高支持`​​12.5Mbps​`​（具体取决于系统时钟分频配置）。

​​数据格式​​：

- `数据位`：5 - 9 位可编程
- `停止位`：1 / 1.5 / 2 位可选
- `校验位`：无校验 / 奇校验 / 偶校验
- `​硬件流控制`​​：支持RTS（请求发送）和CTS（清除发送）信号，防止数据溢出。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214206.png)

## 2. SCI使用示例

### 2.1 CCS&syscfg配置

打开工程下的 .syscfg 文件。找到 SCI 选项开始配置：

- 配置串口参数为波特率 115200、8 位数据长度、1 位停止位、无校验位。

- 开启串口发送与接收功能，并开启串口接收中断。

- 使用 SCIA-RX = GPIO28 和 SCIA-TX = GPIO29。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250516_134343.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250516_134432.png)

### 2.2 用户代码

```c
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "string.h" //使用 memset(), strlen() 需导入该文件

#define uart_rx_max 50              //定义最大接收长度
uint16_t rDataA[ uart_rx_max ]={0}; //定义串口接收数据的缓冲区
uint16_t rx_len = 0;                //串口接收数据长度
uint16_t rx_flag = 0;               //串口是否有接收到数据的标志位 =0没有 =1有

//任意毫秒的延时
void delay_ms(int x)
{
    while(x--)
    {
        DEVICE_DELAY_US(1000);
    }
}

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

    while(1)
    {
        //如果串口接收标志位为1
        if( rx_flag== 1 )
        {
            rx_flag=0;
            GPIO_togglePin(User_LED);
            //发送数据
            SCI_writeCharArray(SCIA_BASE, rDataA, strlen(rDataA));
            //清除数据
            memset(rDataA, 0, sizeof(rDataA));
            rx_len=  0;
        }
        delay_ms(1000);
    }
}

__interrupt void INT_User_UART_RX_ISR(void)
{
    //没有开启FIFO的情况下，等待接收区有数据了就返回数据
    rDataA[rx_len] = SCI_readCharBlockingNonFIFO(SCIA_BASE);
    //回环长度限制
    rx_len = ( rx_len + 1 ) % uart_rx_max;
    //设置接收标志位为1
    rx_flag = 1;

    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}
```

## 3. C2000 SCI (UART) 相关函数总结 (基于DriverLib)

> **注**：本总结以TMS320F2837x/Dx系列为例。SCI是一个非常标准的外设，其概念在所有C2000型号中通用。主要区别在于不同芯片的SCI模块数量和支持的高级功能（如FIFO深度、LIN支持）可能略有差异。

### 3.1 SCI概述与核心概念

SCI模块提供了标准的UART功能，支持全双工、异步通信，通常由波特率发生器、发送器、接收器和控制逻辑组成。

- **核心通信流程**：  
  **数据寄存器 (SCITXBUF)** -> **发送移位寄存器** -> **TX引脚**  
  **RX引脚** -> **接收移位寄存器** -> **数据寄存器 (SCIRXBUF)**

- **主要组件说明**：
  
  | **组件**     | **功能**               | **C2000特点**                                   |
  | ---------- | -------------------- | --------------------------------------------- |
  | **波特率发生器** | 产生发送和接收所需的波特率时钟。     | 基于低速外设时钟LSPCLK，通过16位分频器生成。                    |
  | **数据帧格式**  | 定义串行数据的组成。           | 可配置：数据位（8位）、停止位（1-2位）、校验位（无、奇、偶）。             |
  | **FIFOs**  | 先入先出缓冲区。             | 大多数C2000的SCI模块包含深度的TX和RX FIFO，可减少中断开销，提高通信效率。 |
  | **中断源**    | 标志通信事件（发送空、接收满、错误等）。 | 通过PIE管理，可灵活使能/禁止不同中断源。                        |

### 3.2 SCI初始化与配置

- **核心配置流程**（五步关键操作）：
  
  1. **使能SCI外设时钟**（配置LSPCLK，通常已在时钟初始化中完成）
  
  2. **配置GPIO复用为SCI功能**（将特定引脚设置为SCIRXD/SCITXD）
  
  3. **初始化SCI模块参数**（波特率、数据格式等）
  
  4. **（可选）配置FIFO**（设置触发深度、使能）
  
  5. **（可选）配置中断**（使能所需的中断源并注册ISR）

- **基础配置示例**（配置SCI-A，115200, 8-N-1，轮询模式）：
  
  ```c
  #include "driverlib.h"
  
  // 假设LSPCLK = 50MHz (SYSCLK=200MHz / LSPCLKDIV=4)
  #define LSPCLK_FREQ_HZ  50000000
  #define SCI_BAUDRATE    115200
  
  void main(void) {
      // 1. 初始化器件、GPIO、PIE
      Device_init();
      Device_initGPIO();
      // ... 确保LSPCLK已正确分频 ...
  
      // 2. 配置GPIO引脚为SCI功能 (以SCIA为例，GPIO28-RX, GPIO29-TX)
      GPIO_setPinConfig(GPIO_28_SCIRXDA);  // GPIO28 复用为 SCIA的RX
      GPIO_setPinConfig(GPIO_29_SCITXDA);  // GPIO29 复用为 SCIA的TX
      // GPIO方向会自动设置，但也可以显式配置
      GPIO_setDirectionMode(28, GPIO_DIR_MODE_IN);
      GPIO_setDirectionMode(29, GPIO_DIR_MODE_OUT);
  
      // 3. 初始化SCI模块参数
      SCI_setConfig(SCIA_BASE, LSPCLK_FREQ_HZ, SCI_BAUDRATE,
                   (SCI_CONFIG_WLEN_8 | SCI_CONFIG_STOP_ONE | SCI_CONFIG_PAR_NONE));
      SCI_resetModule(SCIA_BASE); // 复位模块，确保配置生效
      SCI_enableModule(SCIA_BASE); // 使能SCI模块
  
      // 4. 使能FIFO（推荐）并设置触发级别
      SCI_enableFIFO(SCIA_BASE); // 使能FIFO功能
      SCI_resetTxFIFO(SCIA_BASE);
      SCI_resetRxFIFO(SCIA_BASE);
      SCI_setFIFOInterruptLevel(SCIA_BASE, SCI_FIFO_TX0, SCI_FIFO_RX4); // TX空，RX有4字节产生中断
  
      // 5. （轮询示例）发送一个字符串
      const char message[] = "Hello, C2000 SCI!\r\n";
      uint16_t i;
      for (i = 0; i < sizeof(message) - 1; i++) {
          SCI_writeCharBlockingFIFO(SCIA_BASE, message[i]); // 阻塞式发送字符
      }
  
      while(1) {
          // 6. （轮询示例）检查并接收数据
          if (SCI_getRxStatus(SCIA_BASE) & SCI_RXSTATUS_READY) {
              uint16_t receivedChar = SCI_readCharNonBlocking(SCIA_BASE);
              // 处理 receivedChar
              SCI_writeCharBlockingFIFO(SCIA_BASE, receivedChar); // 回显
          }
      }
  }
  ```

- **关键配置函数详解**：
  
  | **函数 (DriverLib)**                                                                       | **说明**                         | **参数示例与解释**                                                                                                          |
  | ---------------------------------------------------------------------------------------- | ------------------------------ | -------------------------------------------------------------------------------------------------------------------- |
  | `SCI_setConfig(uint32_t base, uint32_t lspclkHz, uint32_t baudrate, uint32_t config)`    | **SCI核心配置函数**。一次性设置波特率和数据格式。   | `base`: `SCIA_BASE`, `SCIB_BASE`等<br>`lspclkHz`: LSPCLK频率 (Hz)<br>`baudrate`: 目标波特率 (bps)<br>`config`: 数据格式位掩码 (见下表) |
  | `SCI_enableModule(uint32_t base)` / `disableModule`                                      | **使能/禁用SCI模块**。                | `base`: SCI模块基地址                                                                                                     |
  | `SCI_resetModule(uint32_t base)`                                                         | **复位SCI模块**。通常在配置前或错误恢复后调用。    | `base`: SCI模块基地址                                                                                                     |
  | `SCI_enableFIFO(uint32_t base)` / `disableFIFO`                                          | **使能/禁用FIFO功能**。**强烈推荐使能**。    | `base`: SCI模块基地址                                                                                                     |
  | `SCI_resetTxFIFO(uint32_t base)` / `resetRxFIFO`                                         | **复位TX/RX FIFO指针**。使能FIFO后应调用。 | `base`: SCI模块基地址                                                                                                     |
  | `SCI_setFIFOInterruptLevel(uint32_t base, SCI_FIFOLevel txLevel, SCI_FIFOLevel rxLevel)` | **设置FIFO中断触发深度**。              | `txLevel`: `SCI_FIFO_TX0` (TX空), `SCI_FIFO_TX1`...<br>`rxLevel`: `SCI_FIFO_RX0`~`SCI_FIFO_RX15` (RX达到n字节)            |

- **数据格式配置宏 (`config` 参数)**：
  
  | **配置项**   | **宏**                                                               | **说明**     |
  | --------- | ------------------------------------------------------------------- | ---------- |
  | **数据位长度** | `SCI_CONFIG_WLEN_8`                                                 | 8位数据       |
  |           | `SCI_CONFIG_WLEN_7`                                                 | 7位数据 (不常用) |
  | **停止位**   | `SCI_CONFIG_STOP_ONE`                                               | 1个停止位      |
  |           | `SCI_CONFIG_STOP_TWO`                                               | 2个停止位      |
  | **校验位**   | `SCI_CONFIG_PAR_NONE`                                               | 无校验        |
  |           | `SCI_CONFIG_PAR_EVEN`                                               | 偶校验        |
  |           | `SCI_CONFIG_PAR_ODD`                                                | 奇校验        |
  | **组合示例**  | `(SCI_CONFIG_WLEN_8 \| SCI_CONFIG_STOP_ONE \| SCI_CONFIG_PAR_NONE)` | 经典的8-N-1格式 |

### 3.3 SCI数据收发函数

- **阻塞式函数 (Polling)**：适合简单应用或调试，会等待直到操作完成。
  
  | **函数**                           | **原型**                  | **说明**                         |
  | -------------------------------- | ----------------------- | ------------------------------ |
  | `SCI_writeCharBlockingFIFO`      | `(base, charData)`      | **阻塞式发送一个字符**。等待TX FIFO有空位后写入。 |
  | `SCI_readCharBlockingFIFO`       | `(base)`                | **阻塞式接收一个字符**。等待RX FIFO有数据后读取。 |
  | `SCI_writeCharArrayBlockingFIFO` | `(base, array, length)` | **阻塞式发送一个字节数组**。               |
  | `SCI_readCharArrayBlockingFIFO`  | `(base, array, length)` | **阻塞式接收指定长度的字节数组**。            |

- **非阻塞式函数 (Interrupt/DMA)**：适合高效应用，立即返回状态。
  
  | **函数**                         | **原型**             | **说明**                                           |
  | ------------------------------ | ------------------ | ------------------------------------------------ |
  | `SCI_writeCharNonBlockingFIFO` | `(base, charData)` | **非阻塞式发送**。仅当TX FIFO有空间时写入，否则返回错误。               |
  | `SCI_readCharNonBlocking`      | `(base)`           | **非阻塞式接收**。仅当RX FIFO有数据时读取，否则返回错误。               |
  | `SCI_getRxStatus`              | `(base)`           | **获取接收状态**。检查是否有数据可用 (`SCI_RXSTATUS_READY`)。     |
  | `SCI_getTxStatus`              | `(base)`           | **获取发送状态**。检查TX FIFO是否为空 (`SCI_TXSTATUS_EMPTY`)。 |

- **操作示例**：
  
  ```c
  // 1. 发送字符串 (阻塞式)
  SCI_writeCharArrayBlockingFIFO(SCIA_BASE, (uint16_t*)"Hello\r\n", 6);
  
  // 2. 接收并回显 (非阻塞式查询)
  int16_t rxData;
  rxData = SCI_readCharNonBlocking(SCIA_BASE);
  if (rxData != -1) { // -1 表示没有数据
      SCI_writeCharNonBlockingFIFO(SCIA_BASE, rxData); // 回显
  }
  
  // 3. 检查发送是否完成（非阻塞式）
  if (SCI_getTxStatus(SCIA_BASE) & SCI_TXSTATUS_EMPTY) {
      // TX FIFO 和 发送移位寄存器 都已空，可以安全进入低功耗模式
  }
  ```

### 3.4 SCI中断模式

使用中断可以极大地提高CPU效率，避免轮询等待。

- **中断配置示例**（使用RX中断接收数据）：
  
  ```c
  
  ```

- **常用中断源宏**：
  
  | **中断源**     | **宏**           | **说明**             |
  | ----------- | --------------- | ------------------ |
  | **RX FIFO** | `SCI_INT_RXFF`  | RX FIFO达到设定的触发级别   |
  | **TX FIFO** | `SCI_INT_TXFF`  | TX FIFO为空（或低于触发级别） |
  | **接收错误**    | `SCI_INT_RXERR` | 帧错误、溢出错误、校验错误等     |

## 4. 关键注意事项

1. **波特率计算**：确保`LSPCLK_FREQ_HZ`参数正确。错误的LSPCLK频率是导致波特率不准的最常见原因。公式：`BRR = LSPCLK / (BAUD * 8) - 1`，DriverLib的`SCI_setConfig`内部会自动计算。

2. **GPIO复用**：必须正确配置GPIO的复用功能，将引脚映射到SCI，而不是普通的GPIO或其他外设。这是无法通信的首要排查点。

3. **FIFO的使用**：**强烈建议始终使能FIFO**。即使使用轮询，FIFO也能缓冲数据，提高通信可靠性并减少错误。

4. **中断处理**：在ISR中，**读取接收数据或检查发送状态会自动清除SCI模块内部的中断标志**。但你**必须**使用`Interrupt_clear()`来清除PIE组的中断标志。

5. **电平转换**：C2000的SCI引脚是**TTL电平（3.3V）**，如果需要连接RS232设备（如老式PC串口），必须使用**电平转换芯片（如MAX3232）**；如果需要连接RS485设备，则需使用**RS485收发器**。

6. **软件流控**：C2000的SCI硬件不支持CTS/RTS硬件流控，如果需要，必须在软件中实现XON/XOFF协议。

---
