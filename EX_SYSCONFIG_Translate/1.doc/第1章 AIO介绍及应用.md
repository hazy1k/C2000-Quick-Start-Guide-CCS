# 第一章 AIO介绍及应用

## 1. AIO 外设基本介绍

AIO（模拟输入/输出，Analog Input/Output）是 TMS320F28P550 芯片提供的重要通用模拟信号接口。AIO 可配置为输入或输出，支持浮空输入和推挽输出，广泛用于与外部传感器、通信接口等模块的数据交互。  
在实际应用中，AIO 由固件灵活配置，并可结合中断和信号同步，适应各种模拟信号处理场景。

**1. AIO 引脚数量：**

- 在128引脚的 PDT 封装中，TMS320F28P550 提供了 17 个 AIO 引脚。
- 在100引脚的 PZ 封装中，有 16 个 AIO 引脚。
- 在 80 引脚的 PNA 封装中，有 12 个 AIO 引脚。
- 在 64 引脚的 PM 封装中，有 12 个 AIO 引脚。

**2. 功能特性：**

- **模拟输入：** AIO 引脚可以用作模拟信号输入，通常用于连接外部传感器或其他模拟设备，例如电压、电流或温度传感器。
- **数字输入：** 除了模拟功能，这些引脚也可以配置为数字输入，用于接收高/低电平信号，例如按键输入或外部逻辑信号。
- **灵活性：** 这种双重功能使得开发人员可以根据具体应用需求灵活配置引脚，从而优化硬件设计并减少所需的外部元件。

**3. 与 AGPIO 的区别：**

- 除了 AIO 引脚，TMS320F28P55x 系列还提供 AGPIO (Analog with Digital Inputs and Outputs) 引脚。
- AGPIO 引脚在 AIO 的基础上增加了数字输出功能，使得这些引脚可以驱动外部设备。
- 例如，在128引脚的 PDT 封装中，TMS320F28P550 提供了 22 个 AGPIO 引脚。

## 2. SysConfig 配置选项详解

### 2.1 名称（Name）

- **参数说明**：AIO通道实例的名称，自定义便于代码调用。
- **举例**：User_AIO0

---

### 2.2 AIO方向（AIO Direction）

- **参数说明**：指定AIO通道作为输入还是输出。
- **可选项**：
  - IN（输入）
  - OUT（输出）

---

### 2.3 引脚类型（Pin Type）

- **参数说明**：决定AIO引脚的工作电气属性，包括推挽/开漏、上下拉与极性等。
- **全部可选项与含义**：
  - **Push-pull output/floating input**  
    推挽输出/浮空输入。
  - **Push-pull output/pull-up enabled on input**  
    推挽输出，输入模式带上拉。
  - **Push-pull output/pull-up enabled input with INVERTED polarity**  
    推挽输出，输入上拉且极性反转。
  - **Open-drain output/floating input**  
    开漏输出/浮空输入。
  - **Open-drain output with pull-up enabled output and input**  
    开漏输出，输出和输入均带上拉。
  - **Open-drain output/floating inverted input**  
    开漏输出/浮空反相输入。
  - **Open-drain output with pull-up enabled output and INVERTED input**  
    开漏输出，带上拉反相输入。

> **实际选型建议**：一般输入选浮空或上拉，根据外接电路定；输出一般用推挽，需和电路匹配。

---

### 2.4 检波/同步模式（Qualification Mode）

- **参数说明**：用于数字输入信号的同步检测，减少异步带来的毛刺和干扰。
- **可选项**：
  - Synchronization to SYSCLK（与系统时钟同步）
  - None（无同步，直接采样）
  - Sample 3 Times（三次采样一致）
  - Sample 6 Times（六次采样一致）

---

### 2.5 外部中断相关配置（External Interrupts）

#### 2.5.1 Connect to an XINT for interrupts

- **说明**：勾选后AIO信号可连接到外部中断（XINT）。

#### 2.5.2 Use Interrupts

- **说明**：是否启用中断逻辑。勾选后启用中断服务及相关配置面板。

#### 2.5.3 XINT相关详细配置

- **XINT Instance**：选择具体的XINT通道，如 XINT1、XINT2 等。
- **Interrupt Edge**：选择触发方式
  - Rising edge（上升沿）
  - Falling edge（下降沿）
  - Both edges（双边沿）
- **Enable Interrupt**：是否使能外部中断功能。
- **Register Interrupt Handler**：是否自动注册中断回调函数。

##### 2.5.4 XINT PIE中断器配置（XINT PIE Interrupt Configuration）

- **Name**：自动生成的中断名称，便于程序调用。
- **Interrupt Name**：自动生成的中断标识符，如 INT_User_AIO0_XINT
- **Interrupt Handler**：自动生成的中断服务函数名，如 INT_User_AIO0_XINT_ISR
- **Enable Interrupt in PIE**：是否使能PIE中断（Peripheral Interrupt Expansion，中断分配器）。

---

### 2.6 InputXbar 选择

- **说明**：在多路输入分配场合，AIO引脚可与INPUTXBAR外设互联，实现信号灵活分发。
- **举例**：myINPUTXBARINPUTx

---

### 2.7 控制核选择（Core Select）

- **参数说明**：指定该AIO通道由哪个内核控制。
- **常见选项**：
  - CPU1 selected as controller core（一般选CPU1）

---

### 2.8 PinMux外设与引脚配置（Peripheral and Pin Configuration）

#### 2.8.1 AIO Peripheral

- **参数说明**：物理AIO通道编号。
- **举例**：Any(AIO209)、AIO210等

#### 2.8.2 AIO#

- **参数说明**：物理管脚选项。

- **常见举例**：Any(A26, D6, E6/24)
  
  > 选择和PCB硬件版图一致的引脚。

---

### 2.9 其他常见高级配置项（选芯片/版本支持）

- **Drive Strength**（驱动能力）：High/Low
- **Slew Rate**（转换速度）：Fast/Slow
- **Pull Enable**（内部上/下拉）：Enable/Disable

## 3. AIO 外设简单使用示例

### 3.1 AIO配置

![屏幕截图 2025-08-04 163243.png](https://raw.githubusercontent.com/hazy1k/My-drawing-bed/main/2025/08/04-16-33-23-屏幕截图%202025-08-04%20163243.png)

### 3.2 LED配置

![屏幕截图 2025-08-04 163253.png](https://raw.githubusercontent.com/hazy1k/My-drawing-bed/main/2025/08/04-16-33-53-屏幕截图%202025-08-04%20163253.png)

### 3.3 主函数

```c
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "stdio.h"

/* 读取AIO输入电平示例 */
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
        uint32_t AIO_Flag;
        AIO_Flag = GPIO_readPin(myAIO0);
        // printf("AIO_Flag = %d\r\n", AIO_Flag);
        if(AIO_Flag) // 读取高电平，点亮LED
        {
            GPIO_writePin(LED, 0);
        }
        if(AIO_Flag == 0) // 读取低电平，熄灭LED
        {
            GPIO_writePin(LED, 1);
        }
        DEVICE_DELAY_US(10000);
    }
}

```

---


