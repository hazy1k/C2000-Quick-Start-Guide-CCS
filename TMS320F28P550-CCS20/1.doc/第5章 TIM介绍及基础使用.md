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

## 3. C2000 定时器 (TIMER) 相关函数总结 (基于DriverLib)

> **注**：C2000的定时器系统非常丰富且强大，远超基本的通用定时器概念。它主要包括：
> 
> 1. **CPU定时器 (CPU Timers)**：与C28x内核紧密关联，常用于操作系统节拍或高优先级任务调度。
> 
> 2. **ePWM模块中的定时器**：这是C2000的精髓，每个ePWM模块都包含一个专用的时基模块（TB），用于产生PWM波形的核心时序。**（我们将专注于此）**
> 
> 3. **eCAP模块**：可配置为捕获模式（测量频率/占空比）或APWM模式（作为一路PWM输出）。  
>    本总结将重点介绍**ePWM的时基模块(TB)**，因为它是最常用、最强大的定时器应用形式。

### 3.1 ePWM时基模块(TB)概述与核心概念

ePWM的时基模块并非一个独立的定时器，而是一个专门为生成同步PWM波形而优化的定时器系统。它可以独立运行，也可以与其他ePWM模块同步，形成复杂的多通道、高精度控制系统。

- **核心时钟路径**：  
  **HSPCLK** -> **TBCLK** (时基时钟) -> **TBCTR** (时基计数器) -> **比较匹配** -> **生成事件**

- **主要组件说明**：
  
  | **组件**                    | **功能**                   | **C2000特点**                 |
  | ------------------------- | ------------------------ | --------------------------- |
  | **时基计数器 (TBCTR)**         | 核心计数器，在特定模式下循环计数。        | 可配置为递增、递减、先增后减模式。决定了PWM的周期。 |
  | **周期寄存器 (TBPRD)**         | 设置TBCTR的计数周期值。           | 有**影子寄存器**，可实现无毛刺的周期更新。     |
  | **时基时钟预分频器**              | 对HSPCLK进行分频，产生时基时钟TBCLK。 | 可大幅降低计数频率，适应低速控制需求。         |
  | **同步输入/输出 (SYNCI/SYNCO)** | 实现多个ePWM模块之间的精确同步。       | 可构建主从模式，确保所有PWM通道的起始点对齐。    |
  | **比较寄存器 (CMPA/CMPB)**     | 与TBCTR的值进行比较，用于控制PWM占空比。 | 属于动作限定模块(AQ)，但与时基紧密相关。      |

### 3.2 ePWM时基模块初始化与配置

- **核心配置流程**（五步关键操作）：
  
  1. **使能ePWM外设时钟**（配置HSPCLK，通常已在时钟初始化中完成）
  
  2. **配置GPIO复用为ePWM功能**（将特定引脚设置为EPWMxA/EPWMxB）
  
  3. **初始化时基模块参数**（时钟分频、计数模式、周期值）
  
  4. **配置比较模块和动作限定器**（设置占空比和输出动作）
  
  5. **（可选）配置同步和中断**

- **基础配置示例**（配置ePWM1，产生一个100kHz，50%占空比的PWM）：
  
  ```c
  #include "driverlib.h"
  
  // 假设HSPCLK = 100MHz (SYSCLK=200MHz / HSPCLKDIV=2)
  #define HSPCLK_FREQ_HZ      100000000
  #define PWM_FREQ_HZ         100000    // 100 kHz
  #define PWM_DUTY_CYCLE      0.5       // 50%
  
  void main(void) {
      uint16_t periodValue, compareValue;
  
      // 1. 初始化器件、GPIO
      Device_init();
      Device_initGPIO();
      // ... 确保HSPCLK已正确分频 ...
  
      // 2. 配置GPIO引脚为ePWM功能 (以ePWM1A为例, GPIO0)
      GPIO_setPinConfig(GPIO_0_EPWM1A);
  
      // 3. 计算周期值和比较值
      periodValue = (HSPCLK_FREQ_HZ / PWM_FREQ_HZ) - 1; // TBCLK = HSPCLK
      compareValue = (uint16_t)((float)periodValue * PWM_DUTY_CYCLE);
  
      // 4. 配置ePWM1的时基模块
      EPWM_setTimeBasePeriod(EPWM1_BASE, periodValue);   // 设置周期值
      EPWM_setPhaseShift(EPWM1_BASE, 0);                 // 相位偏移为0
      EPWM_setTimeBaseCounter(EPWM1_BASE, 0);            // 计数器从0开始
      EPWM_setTimeBaseCounterMode(EPWM1_BASE, EPWM_COUNTER_MODE_UP); // 递增计数模式
      EPWM_disablePhaseShiftLoad(EPWM1_BASE);            // 禁用相位重载
      EPWM_setClockPrescaler(EPWM1_BASE, 
                            EPWM_CLOCK_DIVIDER_1,       // TBCLK = HSPCLK / 1
                            EPWM_HSCLOCK_DIVIDER_1);    //
  
      // 5. 配置比较模块和动作限定器 (AQ)
      EPWM_setCounterCompareValue(EPWM1_BASE, 
                                 EPWM_COUNTER_COMPARE_A, // 使用CMPA
                                 compareValue);
      // 设置动作：计数器等于CMPA时，EPWM1A输出拉高；计数器等于周期值时，拉低
      EPWM_setActionQualifierAction(EPWM1_BASE,
                                   EPWM_AQ_OUTPUT_A,
                                   EPWM_AQ_OUTPUT_HIGH,  // 动作：置高
                                   EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA); // 条件：递增且等于CMPA
      EPWM_setActionQualifierAction(EPWM1_BASE,
                                   EPWM_AQ_OUTPUT_A,
                                   EPWM_AQ_OUTPUT_LOW,   // 动作：置低
                                   EPWM_AQ_OUTPUT_ON_TIMER_UP_PERIOD); // 条件：递增且等于PRD
  
      // 6. 使能ePWM模块输出
      EPWM_enableOutput(EPWM1_BASE);
  
      while(1) {
          // 主循环中可以动态修改CMPA来改变占空比
          // EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, newCompareValue);
          // 注意：直接写入CMPA可能有毛刺，高级应用应使用影子寄存器
      }
  }
  ```

- **关键时基配置函数详解**：
  
  | **函数 (DriverLib)**                                                                              | **说明**               | **参数示例与解释**                                                                                                                                                       |
  | ----------------------------------------------------------------------------------------------- | -------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
  | `EPWM_setTimeBasePeriod(uint32_t base, uint16_t period)`                                        | **设置时基周期值 (TBPRD)**。 | `base`: `EPWM1_BASE`, `EPWM2_BASE`等<br>`period`: 周期计数值，决定PWM频率。                                                                                                   |
  | `EPWM_setTimeBaseCounterMode(uint32_t base, EPWM_CounterMode mode)`                             | **设置计数模式**。          | `mode`:<br>`EPWM_COUNTER_MODE_UP` (递增)<br>`EPWM_COUNTER_MODE_DOWN` (递减)<br>`EPWM_COUNTER_MODE_UP_DOWN` (先增后减，中心对称PWM)                                             |
  | `EPWM_setClockPrescaler(uint32_t base, EPWM_ClockDivider clkDiv, EPWM_HSCLOCKDivider hsClkDiv)` | **设置时基时钟预分频**。       | `clkDiv`: TBCLK分频 (`EPWM_CLOCK_DIVIDER_1`~`32`)<br>`hsClkDiv`: High-speed pre-divider (`EPWM_HSCLOCK_DIVIDER_1`~`16`)<br>**TBCLK = HSPCLK / (hsClkDiv * clkDiv)** |
  | `EPWM_setTimeBaseCounter(uint32_t base, uint16_t count)`                                        | **设置时基计数器的初始值**。     | `count`: 初始计数值，通常设为0。                                                                                                                                             |
  | `EPWM_setPhaseShift(uint32_t base, uint16_t phase)`                                             | **设置相位偏移值**。用于模块间同步。 | `phase`: 相位偏移计数值。                                                                                                                                                 |

### 3.3 计数模式与PWM生成

ePWM的时基计数器模式直接决定了PWM的类型。

- **递增计数模式 (Up-Count)**：
  
  - **波形**：不对称PWM。
  
  - **特点**：计数器从0到TBPRD。常用于电源控制（如Buck、Boost转换器），因为开关事件可以集中在周期的一端。
  
  - **占空比计算**：`占空比 = CMPA / TBPRD`

- **递减计数模式 (Down-Count)**：
  
  - **波形**：不对称PWM。
  
  - **特点**：计数器从TBPRD到0。不常用，但有时用于特定拓扑。
  
  - **占空比计算**：`占空比 = 1 - (CMPA / TBPRD)`

- **先增后减计数模式 (Up-Down-Count)**：
  
  - **波形**：对称（中心对齐）PWM。
  
  - **特点**：计数器从0到TBPRD再回到0。**常用于电机驱动和逆变器**，因为谐波特性更好。每个周期有两次比较匹配。
  
  - **占空比计算**：`占空比 = (2 * CMPA) / (2 * TBPRD) = CMPA / TBPRD` (但有效频率为 `Fpwm = TBCLK / (2 * TBPRD)`)

### 3.4 高级功能与特性

- **影子寄存器与无毛刺更新**：  
  为了在PWM运行时安全地更新周期或比较值，避免产生毛刺脉冲，必须使用影子寄存器。
  
  ```c
  // 配置CMPA使用影子寄存器，在CTR=0（周期点）时自动加载
  EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE,
                                      EPWM_COUNTER_COMPARE_A,
                                      EPWM_COMP_LOAD_ON_CNTR_ZERO); // 在计数器为0时加载
  
  // 安全地更新占空比（新值会先写入影子寄存器，在下一个周期生效）
  EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, newCompareValue);
  ```

- **同步 (Synchronization)**：  
  多个ePWM模块可以同步，确保所有PWM波形具有一致的起始点。
  
  ```c
  // 配置ePWM1为主模块，输出同步脉冲
  EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);
  
  // 配置ePWM2为从模块，使用ePWM1的同步脉冲作为其同步输入
  EPWM_setSyncInputPulseSource(EPWM2_BASE, EPWM_SYNC_IN_PULSE_SRC_EPWM1SYNCOUT);
  EPWM_allowSyncOutPulse(EPWM2_BASE); // 允许同步事件复位它的计数器
  ```

- **时基中断**：  
  时基模块可以在特定时间点产生中断，例如在周期开始或结束时。
  
  ```c
  // 使能时基周期中断（在每个PWM周期开始时产生中断）
  EPWM_enableInterrupt(EPWM1_BASE);
  EPWM_setInterruptSource(EPWM1_BASE, EPWM_INT_TBCTR_ZERO); // 中断源：计数器等于0
  EPWM_setInterruptEventCount(EPWM1_BASE, 1); // 每1个事件产生一次中断
  
  // 注册PIE中断服务函数 (ePWM1中断属于INT3.1)
  Interrupt_register(INT_EPWM1, &epwm1ISR);
  Interrupt_enable(INT_EPWM1);
  ```

### 3.5 使用示例（电机驱动三相PWM）

#### **示例：配置ePWM1/2/3，产生三相对称PWM，相位互差120度**

```c
#include "driverlib.h"

#define PWM_FREQ_HZ        20000    // 20kHz开关频率
#define HSPCLK_FREQ_HZ     100000000 // 100MHz
#define TBPRD_VALUE        (HSPCLK_FREQ_HZ / (2 * PWM_FREQ_HZ) - 1) // 用于Up-Down模式
#define PHASE_SHIFT_120    ( (2 * TBPRD_VALUE + 1) / 3 ) // 计算120度相位差对应的计数值

void main(void) {
    Device_init();
    Device_initGPIO();
    Device_initPWM();

    // 配置GPIO为ePWM功能 (GPIO0/1->ePWM1A/B, GPIO2/3->ePWM2A/B, GPIO4/5->ePWM3A/B)
    GPIO_setPinConfig(GPIO_0_EPWM1A);
    GPIO_setPinConfig(GPIO_2_EPWM2A);
    GPIO_setPinConfig(GPIO_4_EPWM3A);

    // 配置ePWM1为主模块
    _initEPWMModule_(EPWM1_BASE, 0, TBPRD_VALUE); // 相位偏移为0
    EPWM_setSyncOutPulseMode(EPWM1_BASE, EPWM_SYNC_OUT_PULSE_ON_CNTR_ZERO);

    // 配置ePWM2为从模块，相位偏移120度
    _initEPWMModule_(EPWM2_BASE, PHASE_SHIFT_120, TBPRD_VALUE);
    EPWM_setSyncInputPulseSource(EPWM2_BASE, EPWM_SYNC_IN_PULSE_SRC_EPWM1SYNCOUT);
    EPWM_allowSyncOutPulse(EPWM2_BASE);

    // 配置ePWM3为从模块，相位偏移240度
    _initEPWMModule_(EPWM3_BASE, (2 * PHASE_SHIFT_120), TBPRD_VALUE);
    EPWM_setSyncInputPulseSource(EPWM3_BASE, EPWM_SYNC_IN_PULSE_SRC_EPWM1SYNCOUT);
    EPWM_allowSyncOutPulse(EPWM3_BASE);

    while(1) {
        // 主循环中可以通过CLB、SPI或软件更新CMPA/CMPB值，实现SVPWM或正弦PWM调制
    }
}

// ePWM模块初始化函数（公用）
void _initEPWMModule_(uint32_t base, uint16_t phase, uint16_t period) {
    EPWM_setTimeBasePeriod(base, period);
    EPWM_setPhaseShift(base, phase);
    EPWM_setTimeBaseCounter(base, 0);
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP_DOWN); // 中心对称模式
    EPWM_setClockPrescaler(base, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);
    // ... 还需要配置动作限定器(AQ)来生成PWM ...
}
```

## 4. 关键注意事项

1. **时钟源HSPCLK**：ePWM的时基时钟TBCLK来源于HSPCLK。确保HSPCLK的频率正确配置和分频是计算PWM频率的基础。

2. **影子寄存器**：在**电机驱动、数字电源**等对波形完整性要求极高的应用中，**必须使用影子寄存器**来更新CMPA、CMPB和TBPRD，以避免灾难性的毛刺。

3. **死区时间**：上述示例未配置死区时间。在实际的半桥或全桥电路中，必须使用ePWM的**死区子模块(DB)** 来生成互补带死区的PWM对，防止上下管直通。

4. **计数模式选择**：
   
   - **UP/DOWN模式**：用于电机驱动，谐波更小。
   
   - **UP模式**：用于开关电源，控制更简单。

5. **精度与频率**：PWM频率和占空比的精度由TBCLK的频率和TBPRD的值决定。`TBCLK`越高，`TBPRD`越大，精度越高，但会限制最高PWM频率。需要在分辨率和频率之间权衡。

6. **模拟比较器与跳变区域**：ePWM可以与C2000内部的模拟比较器联动，实现过流保护等功能的**硬件级快速关断（Trip Zone）**，响应速度远快于软件中断。这是C2000用于实时控制的关键安全特性。

---
