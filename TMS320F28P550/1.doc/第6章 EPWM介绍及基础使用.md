# 第六章 EPWM介绍及基础使用

## 1. F28P550上的PWM

F28P550上的PWM叫ePWM，是一种增强型脉宽调制器， 一共有12个，每一个有两个通道，A通道和B通道。

ePWM具有两种工作模式，分别是基于时基Time Base的PWM模式，和比较器Compare的PWM模式。这两种工作模式都具有向上计数模式，向下计数模式，向上向下计数模式。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_120143.png)

以向上向下计数模式为例，计数值一直向上加到最大，再从最大一直减到最小。计数变化的期间，如果计数值等于了设置的阈值（比较值），则输出的信号进行翻转。

这样向上计数的时候必定会遇到一次阈值，信号进行翻转；向下计数的时候也必定会遇到一次阈值，信号又进行一次翻转；最终实现的效果如下：

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_120512.png)

## 2. EPWM使用示例

### 2.1 CCS&syscfg配置

- 在 EPWM Time Base 中，设置时钟为默认的 1 分频，定时器周期 7500，使用向上向下计数模式；

- 在 EPWM Counter Compare 中，在 CMPA 的下拉选项下，设置比较值（阈值）Counter Compare A (CMPA) 为0，即占空比为0%；

- 在 EPWM Action Qualifier 中，在 ePWMxA Event Output Configuration 的下拉选项下，设置向上计数到COMPA值时输出为高，向下计数到COMPA值时输出为高。

- 配置PWM到LED的引脚

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_153551.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_154018.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_154640.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/pwm/pwm_20250523_161216.png)

关于输出频率和占空比的设置说明

ePWM时钟频率 EPWMCLK 来源为 SYSCLK，其默认为 150MHz。

CLKDIV 为 EPWM 定时器的时钟预分频；

HSPCLKDIV 为 EPWM 定时器的时钟二次预分频；

**实际PWM的输出频率TBCLK = EPWMCLK / (HSPCLKDIV * CLKDIV)**

**定时器的周期值 = ePWM频率 / (2 * 希望输出频率)**

**希望输出频率 = ePWM频率 / (2 * 定时器的周期值)**

**COMPx 对比值 = (期望占空比 / 100) * 定时器的周期值**

### 2.2 用户代码

```c
//#############################################################################
//
// FILE:   empty_driverlib_main.c
//
//! \addtogroup driver_example_list
//! <h1>Empty Project Example</h1> 
//!
//! This example is an empty project setup for Driverlib development.
//!
//
//#############################################################################
//
//
// 
// C2000Ware v5.05.00.00
//
// Copyright (C) 2024 Texas Instruments Incorporated - http://www.ti.com
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
// 
//   Redistributions of source code must retain the above copyright 
//   notice, this list of conditions and the following disclaimer.
// 
//   Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the 
//   documentation and/or other materials provided with the   
//   distribution.
// 
//   Neither the name of Texas Instruments Incorporated nor the names of
//   its contributors may be used to endorse or promote products derived
//   from this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// $
//#############################################################################

//
// Included Files
//
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"

/* PWM呼吸灯参数 */
#define STEP 10 // 占比比变化步长
uint16_t light_duty = 0; // PWM占空比
int8_t dir = 1; // 呼吸灯方向(1=正向, -1=反向)

void breathing_light(void)
{
    light_duty += dir * STEP; // 改变PWM占空比
    if(light_duty >= 7500)
    {
        light_duty = 7500;
        dir = -1; // 呼吸灯方向反转
    }
    else if (light_duty <= 0) 
    {
        light_duty = 0;
        dir = 1; // 呼吸灯方向恢复
    }
    // 设置EPWM比较值
    EPWM_setCounterCompareValue(User_EPWM_BASE, EPWM_COUNTER_COMPARE_A, light_duty);
}


//
// Main
//
void main(void)
{

    //
    // Initialize device clock and peripherals
    //
    Device_init();

    //
    // Disable pin locks and enable internal pull-ups.
    //
    Device_initGPIO();

    //
    // Initialize PIE and clear PIE registers. Disables CPU interrupts.
    //
    Interrupt_initModule();

    //
    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    //
    Interrupt_initVectorTable();

    //
    // PinMux and Peripheral Initialization
    //
    Board_init();

    //
    // C2000Ware Library initialization
    //
    C2000Ware_libraries_init();

    //
    // Enable Global Interrupt (INTM) and real time interrupt (DBGM)
    //
    EINT;
    ERTM;

    while(1)
    {
        breathing_light(); // 呼吸灯变化
        DEVICE_DELAY_US(1000);
    }
}

//
// End of File
//
```

## 3. 相关函数介绍

```c
//*****************************************************************************
//
//! 设置计数器比较值。
//!
//! \param base 是 EPWM 模块的基地址。
//! \param compModule 是计数器比较值模块。
//! \param compCount 是计数器比较计数值。
//!
//! 此函数用于设置计数器比较寄存器的比较值。
//! compCount 的最大值为 0xFFFF。
//! compModule 的有效值为：
//!   - EPWM_COUNTER_COMPARE_A - 计数器比较 A。
//!   - EPWM_COUNTER_COMPARE_B - 计数器比较 B。
//!   - EPWM_COUNTER_COMPARE_C - 计数器比较 C。
//!   - EPWM_COUNTER_COMPARE_D - 计数器比较 D。
//!
//! \return 无。
//
//*****************************************************************************
static inline void EPWM_setCounterCompareValue(uint32_t base, EPWM_CounterCompareModule compModule, uint16_t compCount);


//*****************************************************************************
//
//! 设置 PWM 周期计数值。
//!
//! \param base 是 EPWM 模块的基地址。
//! \param periodCount 是周期计数值。
//!
//! 此函数用于设置 PWM 的周期计数值。periodCount 的值将直接写入寄存器。
//! 用户应根据所需的波形周期或频率映射到正确的 periodCount。
//! 调用函数 EPWM_selectPeriodLoadEvent() 并传入适当的参数以设置周期计数的加载模式。
//! periodCount 的最大有效值为 0xFFFF。
//!
//! \return 无。
//
//*****************************************************************************
static inline void EPWM_setTimeBasePeriod(uint32_t base, uint16_t periodCount);
```

---
