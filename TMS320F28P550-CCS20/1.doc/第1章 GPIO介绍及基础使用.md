# 第一章 GPIO介绍及基础使用

## 1. GPIO简介

GPIO，全称为 「General-Purpose Input/Output」，即`通用输入/输出端口`。

它是微控制器、微处理器和某些其他集成电路上的`一个引脚`，可以由软件控制其输入或输出状态。

GPIO 为开发者提供了一种简单、灵活的方式来控制硬件设备或与外部设备进行通信。

## 2. GPIO使用示例

### 2.1 硬件说明

#### 2.1.1 LED

GPIO 章节我们将使用 LED 和 按键 作为案例，而作为案例那我们就需要了解其具体的应用电路，了解应如何通过 GPIO 操控 LED 和按键

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250310_220618.png)

- 主要器件为 LED3，这是一个将三个颜色的灯融合到一起的彩灯器件，三个颜色分别为 R（红色red）、G（绿色green）、B（蓝色blue）。

- R127、R128、R129 为LED灯的限流电阻，通常跟LED搭配，防止LED灯电路上的电流过大，导致LED灯烧毁。放置的阻值通常在 220Ω ~ 10KΩ。在原理图中，三个不同颜色的LED灯限流电阻分别用了不同的阻值，是因为 LED 灯颜色材料的不同其消化的电流也不同，为了让三个颜色的LED灯亮度一致，经过实测发现使用 3KΩ、4.3KΩ、9.1KΩ 时，通过肉眼观察，发现亮度基本一致。

- R130、R131 是上拉电阻。主要的功能就是让LED灯在上电未驱动的情况下，默认是灭的。因为 LED 的正极是高电平，负级也因为接入了上拉电阻导致其也是高电平，高电平和高电平无法形成压差产生电流，而没有电流，灯就不会亮。只有当 RGB-B 或 RGB-G 为低电平时，LED 灯才会亮。

#### 2.1.2 按键

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250310_220726.png)

在开发板的原理图中，一共有三个按键，分别是RST、BOOT、KEY按键。

其中 RST 是芯片复位功能按键，不能通过软件的方式去控制该按键，当按键按下时，芯片的 XRSN 引脚将会接入 GND，导致芯片将会自动复位重新工作。其中 R58 和 C76 组成了上电复位电路。R58是一个上拉电阻，让 XRSN 引脚上电时为高电平。C76是一个 100nF 的小电容，根据电容的原理可知道，电容其实就是一个电池，电容在上电时会保持一个很短暂的导通现象，所以上电的瞬间，XRSN 通过电容变成了低电平进行复位；当电容充满电后，电容隔开了直流，XRSN 不再是低电平。这样就实现了上电复位功能。

BOOT 是特殊按键，它接到了 GPIO24 引脚，该引脚和 GPIO32 引脚是用于设置启动模式的特殊引脚。通过这两个引脚，可以设置四种模式，具体见下图：

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250311_114443.png)

开发板硬件上，默认将 GPIO24 和 GPIO32 引脚通过上拉电阻 R55 和 R56 接到了高电平，这样启动模式就是从 Flash 中启动，我们的代码就是烧录到 Flash 中。如果你要从 SCI 串口启动，则按住 BOOT 键，再上电即可。

KEY 按键就是用户的可以操控的按键，将 GPIO27 引脚接入了这个按键，通过按键接到 GND。采用的检测方式是通过给按键的一端接入低电平，一端接入 GPIO。在没有按下按键时，因为上拉电阻 R137 的存在，导致一直处在高电平；当按键按下时，上拉电阻失效，GPIO27 引脚将被变为低电平的 GND。这样高低电平的变化，就可以让开发板检测到按键的状态。

### 2.2 CCS工程

#### 2.2.1 CCS&syscfg配置

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250513_145651.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250513_151629.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250513_152150.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tms320f28p550/beginner/gpio/gpio_20250513_153054.png)

![屏幕截图 2025-07-16 190923.png](https://raw.githubusercontent.com/hazy1k/My-drawing-bed/main/2025/07/16-19-12-36-屏幕截图%202025-07-16%20190923.png)

![屏幕截图 2025-07-16 190929.png](https://raw.githubusercontent.com/hazy1k/My-drawing-bed/main/2025/07/16-19-12-42-屏幕截图%202025-07-16%20190929.png)

#### 2.2.2 用户代码

```c
    while(1)
    {
        if( GPIO_readPin(User_KEY) == 0 )
        {
            GPIO_writePin(User_LED, 0);
        }
        else
        {
            GPIO_writePin(User_LED, 1);
        }
    }
```

## 3. C2000 GPIO相关函数总结 (基于DriverLib)

> **注**：本总结以常用的TMS320F2837x (Delfino) 系列为例，其概念和函数与其他C2000型号（如F2833x, F28004x）相通，但具体寄存器名称和函数参数可能略有差异，请以您所用芯片的**DriverLib用户指南**为准。

### 3.1 初始化与配置

- **核心配置流程**（四步关键操作）：
  
  1. **使能外设时钟**（通常默认开启，但最好显式配置）
  
  2. **设置引脚功能为GPIO**（C2000引脚多为多功能复用，需明确指定为GPIO）
  
  3. **配置GPIO方向和模式**（输入/输出，上拉/下拉等）
  
  4. **配置GPIO量化器/滤波器**（若用于数字输入，如eCAP，eQEP等）

- **基础配置示例**（配置GPIO31为输出，控制LED）：
  
  ```c
  #include "driverlib.h"
  
  // 1. 主初始化（包含系统时钟、PLL等配置，通常由示例工程提供）
  Device_init(); // 初始化器件时钟，PLL等
  Device_initGPIO(); // 初始化GPIO模块，此函数在F2837x中很重要
  
  // 2. 显式配置GPIO31为输出
  GPIO_setPinConfig(GPIO_31_GPIO31); // 设置引脚功能为普通GPIO，而非外设功能
  GPIO_setDirectionMode(31, GPIO_DIR_MODE_OUT); // 设置GPIO31为输出模式
  GPIO_setPadConfig(31, GPIO_PIN_TYPE_STD); // 设置推挽输出（标准模式，无上拉下拉）
  GPIO_writePin(31, 0); // 初始输出低电平
  
  // 也可以使用便捷的 setupPin API (F28004x等更新型号常用)
  GPIO_setupPinMux(31, GPIO_MUX_CPU1, 0); // 选择CPU1控制，功能选择0（GPIO）
  GPIO_setupPinOptions(31, GPIO_OUTPUT, GPIO_PUSHPULL); // 设置为推挽输出
  GPIO_writePin(31, 0);
  ```

- **GPIO配置函数详解**：
  
  | **函数 (DriverLib)**                                                     | **说明**                                  | **参数示例**                                                                                                                                           |
  | ---------------------------------------------------------------------- | --------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------- |
  | `GPIO_setPinConfig(uint32_t pinConfig)`                                | **设置引脚复用功能**。参数是预定义的宏，指定引脚和功能。          | `GPIO_31_GPIO31` (GPIO31用作GPIO)<br>`GPIO_28_EPWM1A` (GPIO28用作EPWM1A)                                                                               |
  | `GPIO_setDirectionMode(uint32_t pin, GPIO_Direction mode)`             | **设置输入/输出方向**。                          | `pin`: 引脚编号 (0-31, 或更多取决于芯片)<br>`mode`: `GPIO_DIR_MODE_IN` 或 `GPIO_DIR_MODE_OUT`                                                                   |
  | `GPIO_setPadConfig(uint32_t pin, uint32_t pinType)`                    | **设置引脚的电气特性（上下拉）**。                     | `pinType`:<br>`GPIO_PIN_TYPE_STD` (标准推挽，无上下拉)<br>`GPIO_PIN_TYPE_PULLUP` (使能内部上拉)<br>`GPIO_PIN_TYPE_PULLDOWN` (使能内部下拉)<br>`GPIO_PIN_TYPE_OD` (开漏输出) |
  | `GPIO_setQualificationMode(uint32_t pin, GPIO_QualificationMode mode)` | **设置输入量化/滤波**。用于消除毛刺，对eQEP、eCAP等外设至关重要。 | `mode`:<br>`GPIO_QUAL_SYNC` (仅同步到系统时钟)<br>`GPIO_QUAL_3SAMPLE` (3个采样周期滤波)<br>`GPIO_QUAL_6SAMPLE` (6个采样周期滤波)<br>`GPIO_QUAL_ASYNC` (异步路径，无滤波)         |

### 3.2 GPIO操作核心函数

- **基本读写操作**：
  
  | **函数**                                         | **原型**                   | **说明**                          |
  | ---------------------------------------------- | ------------------------ | ------------------------------- |
  | `GPIO_writePin(uint32_t pin, uint32_t outVal)` | `(pin, 0 or 1)`          | **写单个引脚**。设置输出引脚的电平。            |
  | `uint32_t GPIO_readPin(uint32_t pin)`          | `(pin)`                  | **读单个引脚**。返回输入或输出引脚的电平状态。       |
  | `GPIO_togglePin(uint32_t pin)`                 | `(pin)`                  | **翻转单个引脚**。如果输出高则变低，反之亦然。       |
  | `GPIO_setPins(uint32_t port, uint32_t pins)`   | `(GPIO_PORT_X, bitmask)` | **同时设置一组引脚为高**。操作同一GPIO端口的多个引脚。 |
  | `GPIO_clearPins(uint32_t port, uint32_t pins)` | `(GPIO_PORT_X, bitmask)` | **同时清除一组引脚为低**。操作同一GPIO端口的多个引脚。 |
  | `uint32_t GPIO_readPort(uint32_t port)`        | `(GPIO_PORT_X)`          | **读取整个GPIO端口的值**。               |

- **操作示例**：
  
  ```c
  // 1. 点灯操作（输出）
  GPIO_writePin(31, 1); // LED On
  GPIO_writePin(31, 0); // LED Off
  GPIO_togglePin(31);   // LED Toggle
  
  // 2. 按键读取（输入）
  // 假设GPIO16连接按键，配置为输入带上拉
  GPIO_setPinConfig(GPIO_16_GPIO16);
  GPIO_setDirectionMode(16, GPIO_DIR_MODE_IN);
  GPIO_setPadConfig(16, GPIO_PIN_TYPE_PULLUP); // 按键按下时拉到地，故常态靠上拉为高
  
  if (GPIO_readPin(16) == 0) {
      // 检测到按键被按下（引脚被拉低）
      do_something();
  }
  
  // 3. 同时操作多个引脚（例如：数据总线）
  // 假设GPIO0-GPIO7连接数据总线，配置为输出
  #define DATA_BUS_MASK 0x00FF // 位掩码，对应Pin 0-7
  GPIO_setPortPins(GPIO_PORT_A, DATA_BUS_MASK); // 初始化，具体函数名可能因系列而异
  
  // 一次性写入一个字节的数据
  GPIO_writePort(GPIO_PORT_A, (data_byte & DATA_BUS_MASK)); // 写入端口
  // 或使用Set/Clear
  GPIO_clearPins(GPIO_PORT_A, DATA_BUS_MASK); // 先清空
  GPIO_setPins(GPIO_PORT_A, (data_byte & DATA_BUS_MASK)); // 再置位
  ```

### 3.3 高级功能与特性

- **GPIO中断（外部中断XINT1-XINT5）**：  
  C2000的GPIO中断不与引脚直接绑定，而是通过外部中断源（XINT1-XINT5）来连接特定的GPIO引脚。
  
  ```c
  // 配置GPIO12作为XINT1的中断源，下降沿触发
  GPIO_setPinConfig(GPIO_12_GPIO12);
  GPIO_setDirectionMode(12, GPIO_DIR_MODE_IN);
  GPIO_setPadConfig(12, GPIO_PIN_TYPE_PULLUP);
  
  // 1. 设置中断回调函数（使用DriverLib中断管理器）
  Interrupt_register(INT_XINT1, &xint1Isr); // 注册中断服务函数
  
  // 2. 配置XINT1外部中断
  XINT_Config xint1Config;
  xint1Config.trigger = XINT_TRIG_FALLING_EDGE; // 下降沿触发
  xint1Config.pin = 12; // 指定GPIO12作为中断源
  xint1Config.enableInt = true; // 使能中断
  XINT_setConfig(XINT_BASE_1, &xint1Config); // 应用配置到XINT1
  
  // 3. 使能XINT1中断
  Interrupt_enable(INT_XINT1);
  Interrupt_clear(INT_XINT1); // 清除可能存在的 pending 中断
  
  // 4. 中断服务函数
  __interrupt void xint1Isr(void) {
      // 处理中断
      GPIO_togglePin(31); // 例如，翻转LED
  
      XINT_clearEventFlag(XINT_BASE_1); // 必须清除XINT1的事件标志！
      Interrupt_clear(INT_XINT1); // 清除PIE组内的中断标志
  }
  ```

- **输入量化（Filtering）**：  
  这是C2000 GPIO的一个特色功能，用于对输入信号进行数字滤波，防止噪声误触发。
  
  ```c
  // 为eQEP的A通道信号（GPIO20）配置6周期采样滤波
  GPIO_setPinConfig(GPIO_20_EQEP1A);
  GPIO_setDirectionMode(20, GPIO_DIR_MODE_IN);
  GPIO_setQualificationMode(20, GPIO_QUAL_6SAMPLE); // 关键配置！6周期滤波
  ```

### 3.4 使用示例（完整流程）

#### 3.4.1 示例：配置LED和按键，并实现中断控制

```c
#include "driverlib.h"

// 定义引脚
#define LED_PIN        31
#define BUTTON_PIN     16
#define BUTTON_XINT    XINT_BASE_1 // 使用XINT1

// 中断服务函数声明
__interrupt void xint1Isr(void);

void main(void) {
    // 1. 初始化器件
    Device_init();
    Device_initGPIO();
    Interrupt_initModule(); // 初始化中断模块(PIE)
    Interrupt_initVectorTable(); // 初始化中断向量表

    // 2. 配置LED引脚（输出）
    GPIO_setPinConfig(GPIO_31_GPIO31);
    GPIO_setDirectionMode(LED_PIN, GPIO_DIR_MODE_OUT);
    GPIO_setPadConfig(LED_PIN, GPIO_PIN_TYPE_STD);
    GPIO_writePin(LED_PIN, 0);

    // 3. 配置按键引脚（输入，并连接到XINT1）
    GPIO_setPinConfig(GPIO_16_GPIO16);
    GPIO_setDirectionMode(BUTTON_PIN, GPIO_DIR_MODE_IN);
    GPIO_setPadConfig(BUTTON_PIN, GPIO_PIN_TYPE_PULLUP); // 按键按下为低，故上拉

    // 4. 配置XINT1中断
    Interrupt_register(INT_XINT1, &xint1Isr); // 注册ISR

    XINT_Config xint1Config;
    xint1Config.trigger = XINT_TRIG_FALLING_EDGE; // 按键按下是下降沿
    xint1Config.pin = BUTTON_PIN; // 指定GPIO16触发XINT1
    xint1Config.enableInt = true;
    XINT_setConfig(BUTTON_XINT, &xint1Config);

    Interrupt_enable(INT_XINT1); // 使能PIE中的XINT1中断
    XINT_clearEventFlag(BUTTON_XINT); // 清除初始标志位

    // 5. 全局使能中断
    Interrupt_globalEnable(); // 全局中断使能(设置INTM位)

    while(1) {
        // 主循环可执行其他任务
        // 中断会处理按键
    }
}

// XINT1 中断服务函数
__interrupt void xint1Isr(void) {
    // 翻转LED状态
    GPIO_togglePin(LED_PIN);

    // 必须清除中断标志！
    XINT_clearEventFlag(BUTTON_XINT); // 清除XINT1事件标志
    Interrupt_clear(INT_XINT1); // 清除PIE组标志
}
```

## 4. 关键注意事项

1. **`Device_initGPIO()`的重要性**：在F2837x等系列中，必须在配置GPIO前调用此函数来释放GPIO到软件控制，否则引脚可能被锁存为默认的上电状态或被仿真器占用。

2. **引脚复用功能选择**：`GPIO_setPinConfig()` 是配置引脚功能的核心。必须根据数据手册中的“Pin Function”表格选择正确的宏（`GPIO_XX_YYYY`）。

3. **输入量化（Qualification）**：
   
   - 用于数字输入外设（ePWM, eCAP, eQEP, ADC SOC）的GPIO信号**通常需要配置量化**（如`GPIO_QUAL_SYNC`或`GPIO_QUAL_6SAMPLE`）以消除噪声。
   
   - 用于普通数字读写的GPIO或外部中断，通常不需要量化或只需简单同步（`GPIO_QUAL_SYNC`）。

4. **中断处理**：
   
   - C2000的中断系统较复杂，涉及PIE（外设中断扩展）。
   
   - 在中断服务程序（ISR）中**必须清除正确的中断标志**：一是外设本身的事件标志（如`XINT_clearEventFlag`），二是PIE组内的应答标志（`Interrupt_clear`）。
   
   - 使用DriverLib的 `Interrupt_register()` 和 `__interrupt` 关键字可以简化中断向量管理。

5. **高低字节控制**：一些C2000芯片的GPIO端口分为高16位和低16位（如GPIOA和GPIOB），操作`GPIO_setPins`等函数时需要注意端口选择和对齐。

---
