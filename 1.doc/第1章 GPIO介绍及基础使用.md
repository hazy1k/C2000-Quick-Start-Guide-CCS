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

## 3. GPIO相关函数介绍

| 序号  | 函数名称                                                                               | 功能                      |
| --- | ---------------------------------------------------------------------------------- | ----------------------- |
| 1   | void GPIO_setInterruptType(GPIO_ExternalIntNum extIntNum, GPIO_IntType intType)    | 设置中断类型                  |
| 2   | GPIO_IntType GPIO_getInterruptType(GPIO_ExternalIntNum extIntNum)                  | 获取中断类型                  |
| 3   | void GPIO_enableInterrupt(GPIO_ExternalIntNum extIntNum)                           | 使能外部中断                  |
| 4   | void GPIO_disableInterrupt(GPIO_ExternalIntNum extIntNum)                          | 失能外部中断                  |
| 5   | uint16_t GPIO_getInterruptCounter(GPIO_ExternalIntNum extIntNum)                   | 获取外部中断计数器的值             |
| 6   | `uint32_t GPIO_readPin(uint32_t pin)`                                              | `读取指定引脚上的值`             |
| 7   | uint32_t GPIO_readPinDataRegister(uint32_t pin)                                    | 读取指定引脚的数据寄存器值           |
| 8   | `void GPIO_writePin(uint32_t pin, uint32_t outVal)`                                | `将一个值写入指定引脚`            |
| 9   | `void GPIO_togglePin(uint32_t pin)`                                                | `指定引脚的值进行翻转`            |
| 10  | uint32_t GPIO_readPortData(GPIO_Port port)                                         | 读取指定端口上的数据              |
| 11  | uint32_t GPIO_readPortDataRegister(GPIO_Port port)                                 | 读取存储在GPIO数据寄存器中的值       |
| 12  | void GPIO_writePortData(GPIO_Port port, uint32_t outVal)                           | 将一个值写入指定端口              |
| 13  | void GPIO_setPortPins(GPIO_Port port, uint32_t pinMask)                            | 将指定端口上的所有指定引脚配置为特定状态    |
| 14  | void GPIO_clearPortPins(GPIO_Port port, uint32_t pinMask)                          | 清除指定端口上的所有指定引脚          |
| 15  | void GPIO_togglePortPins(GPIO_Port port, uint32_t pinMask)                         | 翻转指定端口上的所有指定引脚          |
| 16  | void GPIO_lockPortConfig(GPIO_Port port, uint32_t pinMask)                         | 锁定指定端口上指定引脚的配置          |
| 17  | void GPIO_unlockPortConfig(GPIO_Port port, uint32_t pinMask)                       | 解锁指定端口上指定引脚的配置          |
| 18  | void GPIO_commitPortConfig(GPIO_Port port, uint32_t pinMask)                       | 对指定端口上的指定引脚的锁存配置进行设置    |
| 19  | `void GPIO_setDirectionMode(uint32_t pin, GPIO_Direction pinIO)`                   | `设置指定引脚的方向和模式`          |
| 20  | `GPIO_Direction GPIO_getDirectionMode(uint32_t pin)`                               | `获取指定引脚上的方向和模式`         |
| 21  | void GPIO_setInterruptPin(uint32_t pin, GPIO_ExternalIntNum extIntNum)             | 为指定的外部中断配置引脚            |
| 22  | void GPIO_setPadConfig(uint32_t pin, uint32_t pinType)                             | 设置指定引脚的引脚类型             |
| 23  | uint32_t GPIO_getPadConfig(uint32_t pin)                                           | 获取指定引脚的引脚类型             |
| 24  | void GPIO_setQualificationMode(uint32_t pin, GPIO_QualificationMode qualification) | 设置指定引脚的限定模式             |
| 25  | GPIO_QualificationMode GPIO_getQualificationMode(uint32_t pin)                     | 获取指定引脚的限定模式             |
| 26  | void GPIO_setQualificationPeriod(uint32_t pin, uint32_t divider)                   | 配置指定GPIO输入信号滤波采样周期      |
| 27  | void GPIO_setControllerCore(uint32_t pin, GPIO_CoreSelect core)                    | 设置指定引脚在指定核心上运行          |
| 28  | void GPIO_setAnalogMode(uint32_t pin, GPIO_AnalogMode mode)                        | 设置指定引脚的模拟模式             |
| 29  | void GPIO_setPinConfig(uint32_t pinConfig)                                         | 设置指定引脚的复用功能通常是复用于GPIO外设 |

比较常用的功能函数已经加粗表示。具体的代码，请见库中的 gpio.h。在原版的文件中每一个功能函数都有进行英文注释，如果需要中文版注释，请看下方的机翻代码：

```c
//###########################################################################
//
// 文件：  gpio.h
//
// 标题：  C28x GPIO驱动
//
//###########################################################################
//
// C2000Ware版本 v5.04.00.00
//
// 版权所有 (C) 2024 德州仪器公司 - http://www.ti.com
//
// 允许以源代码和二进制形式重新分发和使用（无论是否修改），需满足以下条件：
//
//   源代码的再分发必须保留上述版权声明、本条件列表及以下免责声明。
//
//   二进制形式的再分发必须在文档和/或其他提供的材料中复制上述版权声明、
//   本条件列表及以下免责声明。
//
//   未经事先书面许可，不得使用德州仪器公司或其贡献者名称来推广衍生自本软件的产品。
//
// 本软件按"原样"提供，版权持有人和贡献者不承担任何明示或默示的担保，
// 包括但不限于对适销性和特定用途适用性的默示担保。在任何情况下，版权持有人或
// 贡献者均不对任何直接、间接、偶然、特殊、惩戒性或后果性损害（包括但不限于
// 替代商品或服务采购、使用损失、数据或利润损失或业务中断）承担责任。
// $
//###########################################################################

#ifndef GPIO_H
#define GPIO_H

//*****************************************************************************
//
// 若使用C++编译器，确保本头文件中的所有定义具有C语言绑定
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! \addtogroup gpio_api GPIO
//! @{
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_xint.h"
#include "cpu.h"
#include "xbar.h"
#include "debug.h"

//*****************************************************************************
//
// 驱动函数内部使用的寄存器访问宏定义（不适用于应用代码）
// 除以2是针对C28x的字访问特性
//
//*****************************************************************************
#define GPIO_CTRL_REGS_STEP     ((GPIO_O_GPBCTRL - GPIO_O_GPACTRL) / 2U)
#define GPIO_DATA_REGS_STEP     ((GPIO_O_GPBDAT - GPIO_O_GPADAT) / 2U)
#define GPIO_DATA_READ_REGS_STEP ((GPIO_O_GPBDAT_R - GPIO_O_GPADAT_R) / 2U)

#define GPIO_GPxCTRL_INDEX      (GPIO_O_GPACTRL / 2U)
#define GPIO_GPxQSEL_INDEX      (GPIO_O_GPAQSEL1 / 2U)
#define GPIO_GPxMUX_INDEX       (GPIO_O_GPAMUX1 / 2U)
#define GPIO_GPxDIR_INDEX       (GPIO_O_GPADIR / 2U)
#define GPIO_GPxAMSEL_INDEX     (0x00000014U / 2U) // GPAAMSEL保留地址
#define GPIO_GPxPUD_INDEX       (GPIO_O_GPAPUD / 2U)
#define GPIO_GPxINV_INDEX       (GPIO_O_GPAINV / 2U)
#define GPIO_GPxODR_INDEX       (GPIO_O_GPAODR / 2U)
#define GPIO_GPxGMUX_INDEX      (GPIO_O_GPAGMUX1 / 2U)
#define GPIO_GPxCSEL_INDEX      (GPIO_O_GPACSEL1 / 2U)
#define GPIO_GPxLOCK_INDEX      (GPIO_O_GPALOCK / 2U)
#define GPIO_GPxCR_INDEX        (GPIO_O_GPACR / 2U)

#define GPIO_GPxDAT_INDEX       (GPIO_O_GPADAT / 2U)
#define GPIO_GPxSET_INDEX       (GPIO_O_GPASET / 2U)
#define GPIO_GPxCLEAR_INDEX     (GPIO_O_GPACLEAR / 2U)
#define GPIO_GPxTOGGLE_INDEX    (GPIO_O_GPATOGGLE / 2U)

#define GPIO_GPxDAT_R_INDEX     (GPIO_O_GPADAT_R / 2U)

#define GPIO_MUX_TO_GMUX        (GPIO_O_GPAGMUX1 - GPIO_O_GPAMUX1)

#ifndef DOXYGEN_PDF_IGNORE
//*****************************************************************************
//
// 可传递给GPIO_setPadConfig()的引脚类型参数及GPIO_getPadConfig()返回值
//
//*****************************************************************************
#define GPIO_PIN_TYPE_STD       0x0000U //!< 推挽输出或浮空输入
#define GPIO_PIN_TYPE_PULLUP    0x0001U //!< 输入上拉使能
#define GPIO_PIN_TYPE_INVERT    0x0002U //!< 输入极性反转
#define GPIO_PIN_TYPE_OD        0x0004U //!< 开漏输出
#endif

//*****************************************************************************
//
//! 可传递给GPIO_setDirectionMode()的方向模式参数及GPIO_getDirectionMode()返回值
//
//*****************************************************************************
typedef enum
{
    GPIO_DIR_MODE_IN,                   //!< 引脚配置为GPIO输入
    GPIO_DIR_MODE_OUT                   //!< 引脚配置为GPIO输出
} GPIO_Direction;

//*****************************************************************************
//
//! 可传递给GPIO_setInterruptType()的中断类型参数及GPIO_getInterruptType()返回值
//
//*****************************************************************************
typedef enum
{
    GPIO_INT_TYPE_FALLING_EDGE = 0x00,   //!< 下降沿中断
    GPIO_INT_TYPE_RISING_EDGE  = 0x04,   //!< 上升沿中断
    GPIO_INT_TYPE_BOTH_EDGES   = 0x0C    //!< 双沿中断
} GPIO_IntType;

//*****************************************************************************
//
//! 可传递给GPIO_setQualificationMode()的滤波模式参数及返回值
//
//*****************************************************************************
typedef enum
{
    GPIO_QUAL_SYNC,                     //!< 同步SYSCLK
    GPIO_QUAL_3SAMPLE,                  //!< 3采样滤波
    GPIO_QUAL_6SAMPLE,                  //!< 6采样滤波
    GPIO_QUAL_ASYNC                     //!< 无同步
} GPIO_QualificationMode;

//*****************************************************************************
//
//! 可传递给GPIO_setAnalogMode()的模拟模式参数
//
//*****************************************************************************
typedef enum
{
    GPIO_ANALOG_DISABLED,       //!< 数字模式
    GPIO_ANALOG_ENABLED         //!< 模拟模式
} GPIO_AnalogMode;

//*****************************************************************************
//
//! 可传递给GPIO_setControllerCore()的核心选择参数
//
//*****************************************************************************
typedef enum
{
    GPIO_CORE_CPU1,             //!< 选择CPU1作为控制核心
    GPIO_CORE_CPU1_CLA1         //!< 选择CPU1的CLA1作为控制核心
} GPIO_CoreSelect;

//*****************************************************************************
//
//! 可传递给端口操作函数的端口标识
//
//*****************************************************************************
typedef enum
{
    GPIO_PORT_A = 0,                    //!< GPIO端口A
    GPIO_PORT_B = 1,                    //!< GPIO端口B
    GPIO_PORT_C = 2,                    //!< GPIO端口C
    GPIO_PORT_G = 6,                    //!< GPIO端口G
    GPIO_PORT_H = 7,                    //!< GPIO端口H
} GPIO_Port;

//*****************************************************************************
//
//! 外部中断编号（用于相关中断函数参数）
//
//*****************************************************************************
typedef enum
{
    GPIO_INT_XINT1,                     //!< 外部中断1
    GPIO_INT_XINT2,                     //!< 外部中断2
    GPIO_INT_XINT3,                     //!< 外部中断3
    GPIO_INT_XINT4,                     //!< 外部中断4
    GPIO_INT_XINT5                      //!< 外部中断5
} GPIO_ExternalIntNum;

//*****************************************************************************
//
// API函数原型
//
//*****************************************************************************
//*****************************************************************************
//
//! \internal
//! 检查引脚编号是否有效
//!
//! 注意：此函数反映设备最大封装下的最高GPIO编号。
//! 具体封装的有效引脚范围请参考数据手册。
//!
//! \return 布尔值表示引脚是否有效
//
//*****************************************************************************
#ifdef DEBUG
static inline bool
GPIO_isPinValid(uint32_t pin)
{
    return((pin <= 81U) || ((pin >= 208U) && (pin <= 215U)) ||
           ((pin >= 224U) && (pin <= 253U) && (pin != 243U) &&
            (pin != 246U) && (pin != 250U)));
}
#endif

//*****************************************************************************
//
//! 设置指定外部中断的触发类型
//!
//! \param extIntNum 外部中断编号
//! \param intType 中断触发类型
//!
//! 此函数为指定外部中断配置触发机制：
//! - GPIO_INT_TYPE_FALLING_EDGE 下降沿触发
//! - GPIO_INT_TYPE_RISING_EDGE 上升沿触发
//! - GPIO_INT_TYPE_BOTH_EDGES 双沿触发
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_setInterruptType(GPIO_ExternalIntNum extIntNum, GPIO_IntType intType)
{
    HWREGH(XINT_BASE + (uint16_t)extIntNum) =
        (HWREGH(XINT_BASE + (uint16_t)extIntNum) & ~XINT_1CR_POLARITY_M) |
        (uint16_t)intType;
}

//*****************************************************************************
//
//! 获取外部中断的触发类型
//!
//! \param extIntNum 外部中断编号
//!
//! \return 返回GPIO_setInterruptType()描述的枚举值之一
//
//*****************************************************************************
static inline GPIO_IntType
GPIO_getInterruptType(GPIO_ExternalIntNum extIntNum)
{
    return((GPIO_IntType)((uint16_t)(HWREGH(XINT_BASE + (uint16_t)extIntNum) &
                                     XINT_1CR_POLARITY_M)));
}

//*****************************************************************************
//
//! 使能指定外部中断
//!
//! \param extIntNum 外部中断编号
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_enableInterrupt(GPIO_ExternalIntNum extIntNum)
{
    HWREGH(XINT_BASE + (uint16_t)extIntNum) |= XINT_1CR_ENABLE;
}

//*****************************************************************************
//
//! 禁用指定外部中断
//!
//! \param extIntNum 外部中断编号
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_disableInterrupt(GPIO_ExternalIntNum extIntNum)
{
    HWREGH(XINT_BASE + (uint16_t)extIntNum) &= ~XINT_1CR_ENABLE;
}

//*****************************************************************************
//
//! 获取外部中断计数器值
//!
//! \param extIntNum 外部中断编号
//!
//! 注意：计数器以SYSCLKOUT速率时钟
//!
//! \return 返回外部中断计数器值
//
//*****************************************************************************
static inline uint16_t
GPIO_getInterruptCounter(GPIO_ExternalIntNum extIntNum)
{
    ASSERT(extIntNum <= GPIO_INT_XINT3);
    return((HWREGH(XINT_BASE + XINT_O_1CTR + (uint16_t)extIntNum)));
}

//*****************************************************************************
//
//! 读取指定引脚的电平值
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 返回指定引脚的数据寄存器值
//
//*****************************************************************************
static inline uint32_t
GPIO_readPin(uint32_t pin)
{
    volatile uint32_t *gpioDataReg;

    ASSERT(GPIO_isPinValid(pin));

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((pin / 32U) * GPIO_DATA_REGS_STEP);

    return((gpioDataReg[GPIO_GPxDAT_INDEX] >> (pin % 32U)) & (uint32_t)0x1U);
}

//*****************************************************************************
//
//! 读取指定引脚的数据寄存器值
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 返回指定引脚的数据寄存器值
//
//*****************************************************************************
static inline uint32_t
GPIO_readPinDataRegister(uint32_t pin)
{
    volatile uint32_t *gpioDataReg;

    ASSERT(GPIO_isPinValid(pin));

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATAREAD_BASE) +
                  ((pin / 32U) * GPIO_DATA_READ_REGS_STEP);

    return((gpioDataReg[GPIO_GPxDAT_R_INDEX] >> (pin % 32U)) & (uint32_t)0x1U);
}

//*****************************************************************************
//
//! 设置指定引脚的输出值
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param outVal 要写入的值（0或1）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_writePin(uint32_t pin, uint32_t outVal)
{
    volatile uint32_t *gpioDataReg;
    uint32_t pinMask;

    ASSERT(GPIO_isPinValid(pin));

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((pin / 32U) * GPIO_DATA_REGS_STEP);

    pinMask = (uint32_t)1U << (pin % 32U);

    if(outVal == 0U)
    {
        gpioDataReg[GPIO_GPxCLEAR_INDEX] = pinMask;
    }
    else
    {
        gpioDataReg[GPIO_GPxSET_INDEX] = pinMask;
    }
}

//*****************************************************************************
//
//! 翻转指定引脚的电平
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_togglePin(uint32_t pin)
{
    volatile uint32_t *gpioDataReg;

    ASSERT(GPIO_isPinValid(pin));

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((pin / 32U) * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxTOGGLE_INDEX] = (uint32_t)1U << (pin % 32U);
}

//*****************************************************************************
//
//! 读取指定端口的数据
//!
//! \param port 端口标识（如GPIO_PORT_A）
//!
//! \return 返回端口各引脚状态（位0对应引脚0，依此类推）
//
//*****************************************************************************
static inline uint32_t
GPIO_readPortData(GPIO_Port port)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((uint32_t)port * GPIO_DATA_REGS_STEP);

    return(gpioDataReg[GPIO_GPxDAT_INDEX]);
}

//*****************************************************************************
//
//! 读取指定端口的数据寄存器值
//!
//! \param port 端口标识（如GPIO_PORT_A）
//!
//! \return 返回端口数据寄存器值（位0对应引脚0，依此类推）
//
//*****************************************************************************
static inline uint32_t
GPIO_readPortDataRegister(GPIO_Port port)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATAREAD_BASE) +
                  ((uint32_t)port * GPIO_DATA_READ_REGS_STEP);

    return(gpioDataReg[GPIO_GPxDAT_R_INDEX]);
}

//*****************************************************************************
//
//! 设置指定端口的输出值
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param outVal 要写入的值（位0对应引脚0，依此类推）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_writePortData(GPIO_Port port, uint32_t outVal)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((uint32_t)port * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxDAT_INDEX] = outVal;
}

//*****************************************************************************
//
//! 置位指定端口的引脚
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（置位对应引脚）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_setPortPins(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((uint32_t)port * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxSET_INDEX] = pinMask;
}

//*****************************************************************************
//
//! 清除指定端口的引脚
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（清除对应引脚）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_clearPortPins(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((uint32_t)port * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxCLEAR_INDEX] = pinMask;
}

//*****************************************************************************
//
//! 翻转指定端口的引脚
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（翻转对应引脚）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_togglePortPins(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIODATA_BASE) +
                  ((uint32_t)port * GPIO_DATA_REGS_STEP);

    gpioDataReg[GPIO_GPxTOGGLE_INDEX] = pinMask;
}

//*****************************************************************************
//
//! 锁定指定端口的引脚配置
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（锁定对应引脚）
//!
//! 注意：此函数锁定引脚复用、方向等配置寄存器，不影响引脚值修改
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_lockPortConfig(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIOCTRL_BASE) +
                  ((uint32_t)port * GPIO_CTRL_REGS_STEP);

    EALLOW;
    gpioDataReg[GPIO_GPxLOCK_INDEX] |= pinMask;
    EDIS;
}

//*****************************************************************************
//
//! 解锁指定端口的引脚配置
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（解锁对应引脚）
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_unlockPortConfig(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIOCTRL_BASE) +
                  ((uint32_t)port * GPIO_CTRL_REGS_STEP);

    EALLOW;
    gpioDataReg[GPIO_GPxLOCK_INDEX] &= ~pinMask;
    EDIS;
}

//*****************************************************************************
//
//! 提交锁定配置（永久锁定）
//!
//! \param port 端口标识（如GPIO_PORT_A）
//! \param pinMask 引脚掩码（锁定对应引脚）
//!
//! 注意：调用后GPIO_lockPortConfig()和GPIO_unlockPortConfig()将不再有效
//!
//! \return 无
//
//*****************************************************************************
static inline void
GPIO_commitPortConfig(GPIO_Port port, uint32_t pinMask)
{
    volatile uint32_t *gpioDataReg;

    gpioDataReg = (uint32_t *)((uintptr_t)GPIOCTRL_BASE) +
                  ((uint32_t)port * GPIO_CTRL_REGS_STEP);

    EALLOW;
    gpioDataReg[GPIO_GPxCR_INDEX] |= pinMask;
    EDIS;
}

//*****************************************************************************
//
//! 设置引脚方向和模式
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param pinIO 方向模式（输入/输出）
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setDirectionMode(uint32_t pin, GPIO_Direction pinIO);

//*****************************************************************************
//
//! 获取引脚方向模式
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 返回GPIO_setDirectionMode()描述的枚举值之一
//
//*****************************************************************************
extern GPIO_Direction
GPIO_getDirectionMode(uint32_t pin);

//*****************************************************************************
//
//! 设置外部中断引脚
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param extIntNum 外部中断编号
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setInterruptPin(uint32_t pin, GPIO_ExternalIntNum extIntNum);

//*****************************************************************************
//
//! 设置引脚电气特性
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param pinType 引脚类型（标准/上拉/开漏/反相等）
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setPadConfig(uint32_t pin, uint32_t pinType);

//*****************************************************************************
//
//! 获取引脚电气特性配置
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 返回GPIO_setPadConfig()描述的位域组合
//
//*****************************************************************************
extern uint32_t
GPIO_getPadConfig(uint32_t pin);

//*****************************************************************************
//
//! 设置引脚输入滤波模式
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param qualification 滤波模式
//!
//! 要设置滤波采样周期，需使用GPIO_setQualificationPeriod()
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setQualificationMode(uint32_t pin, GPIO_QualificationMode qualification);

//*****************************************************************************
//
//! 获取引脚滤波模式
//!
//! \param pin 引脚编号（如GPIO34对应34）
//!
//! \return 返回滤波模式枚举值
//
//*****************************************************************************
extern GPIO_QualificationMode
GPIO_getQualificationMode(uint32_t pin);

//*****************************************************************************
//
//! 设置引脚滤波周期
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param divider 分频系数（1或2-510之间的偶数）
//!
//! 注意：此函数配置8个引脚的共享滤波周期寄存器
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setQualificationPeriod(uint32_t pin, uint32_t divider);

//*****************************************************************************
//
//! 设置引脚控制核心
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param core 控制核心选择
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setControllerCore(uint32_t pin, GPIO_CoreSelect core);

//*****************************************************************************
//
//! 设置引脚模拟模式
//!
//! \param pin 引脚编号（如GPIO34对应34）
//! \param mode 模拟模式使能/禁用
//!
//! 注意：此参数同时适用于AIO和GPIO
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setAnalogMode(uint32_t pin, GPIO_AnalogMode mode);

//*****************************************************************************
//
//! 配置GPIO引脚的复用功能
//!
//! \param pinConfig 引脚配置值（仅使用GPIO_#_????预定义值）
//!
//! 此函数配置选择外设功能的引脚复用器。每个GPIO引脚同一时间只能关联一个
//! 外设功能，且每个外设功能应只关联一个GPIO引脚（尽管许多外设功能可以
//! 映射到多个GPIO引脚）。
//!
//! 可用映射关系定义在<pin_map.h>中。
//!
//! \return 无
//
//*****************************************************************************
extern void
GPIO_setPinConfig(uint32_t pinConfig);

//*****************************************************************************
//
// 关闭Doxygen分组
//! @}
//
//*****************************************************************************

//*****************************************************************************
//
// 结束C++编译器的C语言绑定段
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // GPIO_H
```

---
