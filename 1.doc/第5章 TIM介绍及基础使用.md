# 第五章 TIM介绍及基础使用

## 1. F28P550的CPU定时器

F28P550的CPU定时器有3个，Timer0、Timer1和Timer2。 它们是完全一样的 `32 位计时器`，具有`可预设定周期` 和 `16 位时钟预分频`。此计时器具有 32 位递减计数寄存器，该寄存器在计数器达到 0 时生成一中断。

计数器以 CPU 时钟速度 除以 预分频值设置 形式递减。当计数器达到 0 时，则自动重新加载 32 位周期值。

CPU 计时器 0 用于普通用途并连接至 PIE 块。

CPU 计时器 1 也用于普通用途，并连接至 CPU 的 INT13。

CPU 计时器 2 为 TI-RTOS 保留。该计时器连接至 CPU 的 INT14。如果未使用 TI-RTOS ，CPU 计时器 2 也可用于普通用途。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/cputimer/cputimer_20250522_122013.png)

CPU 计时器 0 和 CPU 计时器 1 使用的是 SYSCLK 时钟，默认为150MHz。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/cputimer/cputimer_20250522_141218.png)

CPU 计时器 2 的时钟可由下列任一时钟来源：

- SYSCLK（默认）
- 内部振荡器 1（INTOSC1）
- 内部振荡器 2（INTOSC2）
- X1 (XTAL)

定时器时间换算

假设使用CPU 计时器 0，其使用的时钟源是 SYSCLK，它默认的频率 = 150MHz

period 为设置的定时器周期

Prescaler 为时钟预分频

则定时时间为：

**定时时间 = (Period + 1) * (Prescaler + 1) / 定时器频率**

## 2. TIM使用示例

### 2.1 CCS&syscfg配置

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/cputimer/cputimer_20250522_144053.png)

图中，我们配置了 预分频值Prescaler = 0，周期Period = 149,999,999，则定时时间为：

**定时时间 = (Period + 1) * (Prescaler + 1) / 定时器频率 = (149,999,999 + 1) * 1 / 150,000,000 = 1 秒**

因为计数器是一个向下计数的模式，则计数器从 149,999,999 减到 0 时，一共会花费 1 秒的时间。

每记一个数所花费的时间 = 1 / 预分频后的定时器频率 = 1 / 150,000,000 = 6.6667 纳秒。

因为案例中用到了蓝色LED灯用来展示状态，所以还需要配置LED灯的引脚为输出模式。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/cputimer/cputimer_20250516_141500.png)

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
        
    }
}

__interrupt void INT_User_TIM0_ISR(void)
{
    GPIO_togglePin(User_LED);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

//
// End of File
//

```

## 3. 相关函数介绍

```c
//*****************************************************************************
//
//! 停止CPU定时器。
//!
//! \param base 是定时器模块的基地址。
//!
//! 该函数用于停止CPU定时器。
//!
//! \return 无。
//
//*****************************************************************************
static inline void CPUTimer_stopTimer(uint32_t base);

//*****************************************************************************
//
//! 启动（或重新启动）CPU定时器。
//!
//! \param base 定时器模块的基地址。
//!
//! 该函数用于启动（或重新启动）CPU定时器。
//!
//! 注意：此函数会重载定时器计数器。
//!
//! \return 无。
//
//*****************************************************************************
static inline void CPUTimer_startTimer(uint32_t base);

/*****************************************************************************
//
//! 启动（或恢复）CPU定时器。
//!
//! \param base 定时器模块的基地址。
//!
//! 该函数用于启动或恢复CPU定时器的运行。
//!
//! 注意：此函数不会重置定时器计数器。
//!
//! \return 无。
//
//*****************************************************************************/
static inline void CPUTimer_resumeTimer(uint32_t base);

//*****************************************************************************
//
//! 重载定时器计数器。
//!
//! \param base 定时器模块的基地址。
//!
//! 该函数将CPU定时器周期寄存器中的值重新加载到定时器计数器。
//!
//! \return 无。
//
//*****************************************************************************/
static inline void CPUTimer_reloadTimerCounter(uint32_t base);

//*****************************************************************************
//
//! 获取当前定时器计数值。
//!
//! \param base 定时器模块的基地址。
//!
//! 该函数返回当前CPU定时器计数器的数值。
//!
//! \return 返回当前定时器计数值。
//
//*****************************************************************************/
static inline uint32_t CPUTimer_getTimerCount(uint32_t base);

//*****************************************************************************
//
//! 获取定时器溢出状态。
//!
//! \param base 定时器模块的基地址。
//!
//! 该函数返回CPU定时器的溢出状态。
//!
//! \return 若定时器已溢出返回true，否则返回false。
//
//*****************************************************************************/
static inline bool CPUTimer_getTimerOverflowStatus(uint32_t base);
```

---


