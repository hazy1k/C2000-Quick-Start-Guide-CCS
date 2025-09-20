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

## 3. C2000 ePWM 模块相关函数总结 (基于DriverLib)

> **注**：ePWM是一个高度模块化的外设。一个ePWM实例（如ePWM1）包含多个子模块：时基(TB)、计数比较(CC)、动作限定器(AQ)、死区生成(DB)、PWM斩波(PC)、事件触发(ET)、故障保护(TZ)。以下内容基于TMS320F2837x/Dx系列。

### 3.1 ePWM 模块概述与核心概念

ePWM模块的功能远不止生成简单的PWM波。它是一个完整的脉冲生成与管理生态系统。

- **核心信号流程**：  
  **时基(TB)** -> **计数比较(CC)** -> **动作限定器(AQ)** -> **死区(DB)** -> **最终输出EPWMxA/B**  
  **故障信号(TZ)** ------------------------------------> **强制关断输出**

- **子模块功能说明**：
  
  | **子模块**   | **缩写** | **核心功能**                               |
  | --------- | ------ | -------------------------------------- |
  | **时基**    | TB     | 产生核心计数器（TBCTR），定义PWM周期和相位。             |
  | **计数比较**  | CC     | 提供比较寄存器（CMPA, CMPB），用于控制占空比。           |
  | **动作限定器** | AQ     | 根据TBCTR与TBPRD/CMPx的比较结果，产生原始的开关动作事件。   |
  | **死区生成**  | DB     | 对AQ产生的原始信号插入可编程的死区时间，生成互补的PWM对，防止桥臂直通。 |
  | **PWM斩波** | PC     | 生成高频载波信号，用于驱动门极变压器（不常用）。               |
  | **事件触发**  | ET     | 基于时间事件（如CTR=PRD, CTR=CMP）产生中断或启动ADC转换。 |
  | **故障保护**  | TZ     | 响应外部故障引脚或内部错误，实现硬件级快速关断，保护功率电路。        |

### 3.2 关键子模块配置详解

#### 3.2.1 计数比较子模块 (CC)

CC模块包含CMPA和CMPB寄存器，它们的值与TBCTR实时比较，是控制占空比的核心。

- **核心函数**：
  
  | **函数**                                                 | **说明**                       |
  | ------------------------------------------------------ | ---------------------------- |
  | `EPWM_setCounterCompareValue(base, compModule, value)` | 设置CMPA或CMPB的值。               |
  | `EPWM_setCounterCompareShadowLoadMode(...)`            | **（重要）**设置影子寄存器加载模式，实现无毛刺更新。 |

- **配置示例**：
  
  ```c
  // 设置CMPA的值，直接控制占空比
  EPWM_setCounterCompareValue(EPWM1_BASE, EPWM_COUNTER_COMPARE_A, 500);
  
  // 配置CMPA使用影子寄存器，在CTR=0时自动加载（安全更新）
  EPWM_setCounterCompareShadowLoadMode(EPWM1_BASE,
                                      EPWM_COUNTER_COMPARE_A,
                                      EPWM_COMP_LOAD_ON_CNTR_ZERO); // 周期点加载
  // 此时，写入CMPA的值会先存入影子寄存器，在下一个周期生效
  ```

#### 3.2.2 动作限定器子模块 (AQ)

AQ是ePWM的逻辑核心。它根据特定事件（如`CTR=CMPA`, `CTR=PRD`）来设置、清除或翻转输出。

- **核心动作事件**：
  
  | **事件**    | **宏**                               | **描述**         |
  | --------- | ----------------------------------- | -------------- |
  | 递增时等于CMPA | `EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA`   | TBCTR递增且等于CMPA |
  | 递减时等于CMPA | `EPWM_AQ_OUTPUT_ON_TIMER_DOWN_CMPA` | TBCTR递减且等于CMPA |
  | 等于周期值     | `EPWM_AQ_OUTPUT_ON_TIMER_PERIOD`    | TBCTR等于TBPRD   |
  | 计数器为零     | `EPWM_AQ_OUTPUT_ON_TIMER_ZERO`      | TBCTR等于0       |

- **核心函数**：
  
  | **函数**                                                       | **说明**                           |
  | ------------------------------------------------------------ | -------------------------------- |
  | `EPWM_setActionQualifierAction(base, output, action, event)` | **定义AQ动作**。指定在某个事件发生时，对输出执行什么操作。 |

- **配置示例（生成不对称PWM）**：
  
  ```c
  // 配置EPWM1A输出：
  // - 当计数器等于CMPA且递增时，拉高输出。
  // - 当计数器等于周期值时，拉低输出。
  EPWM_setActionQualifierAction(EPWM1_BASE,
                               EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_HIGH,
                               EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA);
  EPWM_setActionQualifierAction(EPWM1_BASE,
                               EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_LOW,
                               EPWM_AQ_OUTPUT_ON_TIMER_PERIOD);
  ```

- **配置示例（生成对称PWM）**：
  
  ```c
  // 在Up-Down模式下，需要两个事件来控制一个输出
  // - 递增时等于CMPA时拉高
  // - 递减时等于CMPA时拉低
  EPWM_setActionQualifierAction(EPWM1_BASE,
                               EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_HIGH,
                               EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA);
  EPWM_setActionQualifierAction(EPWM1_BASE,
                               EPWM_AQ_OUTPUT_A,
                               EPWM_AQ_OUTPUT_LOW,
                               EPWM_AQ_OUTPUT_ON_TIMER_DOWN_CMPA);
  ```

#### 3.2.3 死区生成子模块 (DB)

DB模块接收AQ产生的原始信号（A和B），并生成最终带死区的互补输出（A和B）。

- **核心函数**：
  
  | **函数**                                                   | **说明**                    |
  | -------------------------------------------------------- | ------------------------- |
  | `EPWM_deadBandEnable(base)`                              | 使能死区模块。                   |
  | `EPWM_setDeadBandDelayMode(base, delayMode, pwmsignals)` | 设置死区延迟模式（上升沿延迟、下降沿延迟或两者）。 |
  | `EPWM_setDeadBandDelayPolarity(base, pol, pwmsignals)`   | 设置输出极性（高有效/低有效）。          |
  | `EPWM_setDeadBandDelay(base, riseDelay, fallDelay)`      | 设置上升沿和下降沿的死区时间（时钟周期数）。    |

- **配置示例（生成带死区的互补PWM）**：
  
  ```c
  // 1. 使能死区模块
  EPWM_deadBandEnable(EPWM1_BASE);
  
  // 2. 配置死区：对上升沿和下降沿都插入延迟
  EPWM_setDeadBandDelayMode(EPWM1_BASE,
                           EPWM_DB_RED_FED, // Rise Edge Delay & Fall Edge Delay
                           EPWM_DB_COUNTER_CLOCK_FULL_CYCLE); // 时钟分频（通常用全周期）
  
  // 3. 设置输出极性：OUTA高有效，OUTB低有效（互补）
  EPWM_setDeadBandDelayPolarity(EPWM1_BASE,
                               EPWM_DB_POLARITY_ACTIVE_HIGH_CompL, // A高有效，B低有效互补
                               EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);
  
  // 4. 设置死区时间（假设TBCLK=100MHz，欲设置1us死区时间）
  // 死区时间 = (值) * (TBCLK周期)
  uint16_t deadTimeValue = 100; // 100 * 10ns = 1us
  EPWM_setDeadBandDelay(EPWM1_BASE, deadTimeValue, deadTimeValue);
  ```

#### 3.2.4 故障保护子模块 (TZ)

TZ是安全性的基石。它可以通过外部引脚或内部错误源，强制将ePWM输出驱动到安全状态（高、低或高阻）。

- **核心函数**：
  
  | **函数**                                           | **说明**                   |
  | ------------------------------------------------ | ------------------------ |
  | `EPWM_enableTZModule(base, subModule, enable)`   | 使能特定故障源（如TZ1引脚）。         |
  | `EPWM_setTripZoneAction(base, output, action)`   | 设置发生故障时输出的动作。            |
  | `EPWM_setTripZoneDigitalCompareEventAction(...)` | 设置数字比较（DC）事件触发的动作（高级功能）。 |
  | `EPWM_forceTripZoneEvent(base, tzFlags)`         | **软件强制触发故障事件**，用于测试。     |

- **配置示例（过流保护）**：
  
  ```c
  // 1. 使能TZ1引脚作为故障源（通常连接电流采样比较器输出）
  EPWM_enableTZModule(EPWM1_BASE, EPWM_TZ_MODULE_TZ1, true); // 使能TZ1
  
  // 2. 配置TZ1触发时，将EPWM1A和EPWM1B都强制拉高（封锁驱动，桥臂断开）
  //    根据你的硬件设计，也可能是拉低。
  EPWM_setTripZoneAction(EPWM1_BASE,
                        EPWM_TZ_ACTION_EVENT_TZA, // 对EPWM1A的操作
                        EPWM_TZ_ACTION_HIGH);     // 故障时强制输出高
  EPWM_setTripZoneAction(EPWM1_BASE,
                        EPWM_TZ_ACTION_EVENT_TZB, // 对EPWM1B的操作
                        EPWM_TZ_ACTION_HIGH);     // 故障时强制输出高
  
  // 3. （可选）配置为单次故障锁定（CBC-Latch模式）
  //    故障发生后，输出将一直保持安全状态，直到软件清除标志
  EPWM_setTripZoneLatchMode(EPWM1_BASE, EPWM_TZ_ACTION_LATCH_ON_TRIP); // 锁存
  // 在中断中需要清除标志才能恢复输出
  // EPWM_clearTripZoneLatchFlag(EPWM1_BASE, EPWM_TZ_FLAG_TZ1);
  ```

#### 3.2.5 事件触发子模块 (ET)

ET模块允许在特定的时间点（如CTR=PRD, CTR=CMP）触发中断(INT)或启动ADC转换(SOC)。

- **核心函数**：
  
  | **函数**                                               | **说明**             |
  | ---------------------------------------------------- | ------------------ |
  | `EPWM_enableInterrupt(base)`                         | 使能ePWM模块产生中断。      |
  | `EPWM_setInterruptSource(base, event)`               | 设置中断源（如周期中断、比较中断）。 |
  | `EPWM_setInterruptEventCount(base, count)`           | 设置每N次事件产生一次中断。     |
  | `EPWM_enableADCTrigger(base, trig)`                  | 使能ADC启动转换(SOC)触发。  |
  | `EPWM_setADCTriggerSource(base, trig, event)`        | 设置ADC触发的源。         |
  | `EPWM_setADCTriggerEventPrescale(base, trig, count)` | 设置每N次事件产生一次ADC触发。  |

- **配置示例（周期中断和ADC触发）**：
  
  ```c
  // 1. 配置在每个PWM周期开始时（CTR=0）产生中断
  EPWM_enableInterrupt(EPWM1_BASE);
  EPWM_setInterruptSource(EPWM1_BASE, EPWM_INT_TBCTR_ZERO); // 中断源：计数器为零
  EPWM_setInterruptEventCount(EPWM1_BASE, 1); // 每个事件都中断
  
  // 2. 配置在CTR=CMPA时启动ADC转换（用于采样相电流）
  EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A); // 使能SOCA
  EPWM_setADCTriggerSource(EPWM1_BASE,
                          EPWM_SOC_A,
                          EPWM_SOC_TBCTR_U_CMPA); // 触发条件：递增且等于CMPA
  EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1); // 每个事件都触发
  
  // 3. 注册中断并使能PIE（略，见前面章节）
  ```

### 3.3 完整ePWM配置示例（全桥驱动）

```c
void initEPWMForFullBridge(uint32_t base, float dutyCycle) {
    uint16_t period = 999; // 对于UP模式，PWM频率 = HSPCLK / (period+1)
    uint16_t cmpValue = (uint16_t)(period * dutyCycle);

    // 1. 时基(TB)配置
    EPWM_setTimeBasePeriod(base, period);
    EPWM_setTimeBaseCounterMode(base, EPWM_COUNTER_MODE_UP);
    EPWM_setClockPrescaler(base, EPWM_CLOCK_DIVIDER_1, EPWM_HSCLOCK_DIVIDER_1);

    // 2. 计数比较(CC)配置 - 使用影子寄存器安全更新
    EPWM_setCounterCompareShadowLoadMode(base, EPWM_COUNTER_COMPARE_A, EPWM_COMP_LOAD_ON_CNTR_ZERO);
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A, cmpValue);

    // 3. 动作限定器(AQ)配置 - 生成两路原始PWM
    //    EPWMA:  CTR=CMPA时拉高，CTR=PRD时拉低
    //    EPWMB:  CTR=CMPA时拉低，CTR=PRD时拉高 (与A互补)
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_A, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMER_PERIOD);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_LOW, EPWM_AQ_OUTPUT_ON_TIMER_UP_CMPA);
    EPWM_setActionQualifierAction(base, EPWM_AQ_OUTPUT_B, EPWM_AQ_OUTPUT_HIGH, EPWM_AQ_OUTPUT_ON_TIMER_PERIOD);

    // 4. 死区(DB)配置 - 防止桥臂直通
    EPWM_deadBandEnable(base);
    EPWM_setDeadBandDelayMode(base, EPWM_DB_RED_FED, EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);
    uint16_t deadTime = 100; // 假设100个TBCLK周期
    EPWM_setDeadBandDelay(base, deadTime, deadTime);
    // 配置输出极性：A和B均为高有效，但DB模块会自动生成互补带死区的信号
    EPWM_setDeadBandDelayPolarity(base, EPWM_DB_POLARITY_ACTIVE_HIGH, EPWM_DB_COUNTER_CLOCK_FULL_CYCLE);

    // 5. 故障保护(TZ)配置
    EPWM_enableTZModule(base, EPWM_TZ_MODULE_TZ1, true);
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZA, EPWM_TZ_ACTION_HIGH); // 故障时封锁
    EPWM_setTripZoneAction(base, EPWM_TZ_ACTION_EVENT_TZB, EPWM_TZ_ACTION_HIGH);

    // 6. 事件触发(ET)配置 - 在周期点中断，用于运行控制算法
    EPWM_enableInterrupt(base);
    EPWM_setInterruptSource(base, EPWM_INT_TBCTR_ZERO);
    EPWM_setInterruptEventCount(base, 1);
}

// 主函数中调用
initEPWMForFullBridge(EPWM1_BASE, 0.5); // 初始化ePWM1，50%占空比
```

## 4. 关键注意事项

1. **影子寄存器**：在**任何实时控制应用**中，更新CMPA、CMPB、TBPRD必须使用**影子寄存器**，以避免PWM波形出现毛刺，导致功率器件损坏。

2. **时序对齐**：ADC的SOC触发时刻必须与PWM波形保持精确同步（例如在PWM中点采样电流），以确保控制环路计算的准确性。

3. **TZ响应速度**：TZ故障是**硬件自动响应**，无需CPU干预，响应速度极快（纳秒级），是保护系统的最后一道防线。**切勿用软件中断替代**。

4. **时钟一致性**：确保计算PWM频率、死区时间、触发位置时，所使用的`HSPCLK`和`TBCLK`频率值与实际系统配置完全一致。

5. **仿真挂起**：调试时，如果CPU在断点处挂起，ePWM硬件会继续运行。这可能导致功率电路失控。建议在调试实时控制系统时**禁用PWM输出**或使用**低功率测试平台**。

---
