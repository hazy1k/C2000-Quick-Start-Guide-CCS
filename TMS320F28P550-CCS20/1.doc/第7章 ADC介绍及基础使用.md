# 第七章 ADC介绍及基础使用

## 1. F28P550的ADC

5组ADC，分别是ADC-A\B\C\D\E。具有多达32个通道的输入，虽然有32个通道输入，但是只有 16 个转换通道(SOC) 和 16 个可单独寻址的结果寄存器。

采样触发源

- 软件立即启动
- 所有 ePWM ：ADCSOC A 或 B
- GPIO XINT2
- CPU 计时器 0/1/2
- ADCINT1/2
- 捕获模式（CEVT1、CEVT2、CEVT3 和 CEVT4）和 APWM 模式（周期匹配、比较匹配或两者）下的 ECAP 事件。
- 对多个 ADC 采用全局软件触发器

ADC基本参数

**分辨率：** 表示ADC转换器的输出精度，通常以位数（bit）表示，比如8位、10位、12位等，位数越高，精度越高。TMS320F28P550 支持12位的分辨率。

**采样率：** 表示ADC对模拟输入信号进行采样的速率，通常以每秒采样次数（samples per second，SPS）表示，也称为转换速率，表示ADC能够进行多少次模拟到数字的转换。TMS320F28P550 为 3.9M SPS。

**电压基准：** ADC的电压基准是用于与模拟输入信号进行比较，从而实现模拟信号到数字信号的转换的一个参考电压。这个基准电压的准确性和稳定性对ADC的转换精度有着决定性的影响。而 TMS320F28P550 可以支持软件选择三种基准：（1）1.65V 和 2.5V 的可配置内部专用 ADC 基准电压 (VREF)（2）MCU 模拟电源电压 (VDDA) （3）通过 VREF+和 VREF- 引脚为 ADC 提供外部基准(External)。如未配置电压基准则默认使用MCU电源电压作为ADC电压基准。

**采样范围：** 指ADC可以采集到的模拟输入信号的电压范围，范围见下：

VREF- ≤ ADC ≤ VREF+

其中VREF- 为设置的电压基准负，通常为0V。VREF+ 为电压基准正，根据软件的配置确定范围。

## 2. ADC基础使用示例

### 2.1 CCS&syscfg配置

ADC时钟最高75MHz，因此需要对输入时钟（SYSCLK）进行2分频得到ADC时钟；

使用 SOC-0 作为转换通道，转换通道中使用 A6采集引脚 作为模拟输入；

触发源使用软件触发；

配置引脚使用 A6引脚；

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/adc/adc_20250526_104828.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/adc/adc_20250526_105442.png)

### 2.2 用户代码

```c
#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "stdio.h"

void main(void)
{
    uint16_t myADC0Result0 = 0;
    float val = 0.0f;

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
        // 通过软件触发 SOC0 转换
        ADC_forceMultipleSOC(myADC0_BASE, ADC_FORCE_SOC0);
        // 等待ADC总线处理完成
        while(ADC_isBusy(myADC0_BASE) == true)
        {
            DEVICE_DELAY_US(1);
        }
        // 获取 SOC0 通道的转换结果
        myADC0Result0 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
        // 将ADC值换算为实际电压
        val = (myADC0Result0 / 4095.0) * 3.3;
        // 数值放大100倍取出小数
        val = val*100.0;
        // CCS的CIO输出数据
        printf("myADC0Result0=%d val=%d.%d%d\r\n",myADC0Result0, (int)val/100, (int)val/10%10, (int)val%10);

        //延时 100 ms
        DEVICE_DELAY_US(100000);
    }
}
```

## 3. C2000 ADC 模块相关函数总结 (基于DriverLib)

> **注**：本总结以TMS320F2837x/Dx系列的12位ADC为例。其核心概念适用于大多数C2000型号，但 newer devices (如F28004x) 可能采用16位ADC，并拥有更高级的特性，如可编程增益放大器(PGA)。请务必查阅您所用芯片的具体数据手册。

### 3.1 ADC模块概述与核心概念

C2000的ADC模块是一个逐次逼近型(SAR) ADC，其设计重点是**与ePWM模块紧密协同**，实现精确的采样时刻控制，从而满足实时控制系统的时序要求。

- **核心转换流程**：  
  **模拟输入** -> **采样保持电路(S/H)** -> **SAR转换逻辑** -> **结果寄存器(ADCRESULT)** -> **CPU/DMA**

- **主要组件与特性说明**：
  
  | **组件/特性**         | **功能**         | **C2000特点**                                 |
  | ----------------- | -------------- | ------------------------------------------- |
  | **转换精度与速度**       | 决定ADC的性能。      | 常见为12位分辨率。转换时间（采样+转换）可配置，通常可达**~300ns**。    |
  | **采样通道 (SOC)**    | 一次转换的配置单元。     | 可配置每个通道的触发源、采样窗口、通道选择。一个ADC模块支持多个SOC（如16个）。 |
  | **触发源 (Trigger)** | 启动一次ADC转换的信号。  | **关键特性！** 可由ePWM、软件、CPU定时器等触发，实现与PWM波的精确同步。 |
  | **中断与DMA**        | 转换完成后的数据处理方式。  | 可在序列转换结束后产生中断，或通过DMA自动传输结果，极大减轻CPU负担。       |
  | **硬件平均**          | 对同一通道多次采样并取平均。 | 可有效提高有效分辨率，抑制噪声，无需软件干预。                     |

### 3.2 ADC初始化与配置

- **核心配置流程**（四步关键操作）：
  
  1. **配置ADC时钟和功耗模式**（设置模块时钟，退出低功耗模式）
  
  2. **配置采样通道SOC**（指定每个通道由谁触发、采样哪个引脚、采样窗口多长）
  
  3. **配置中断或DMA**（决定转换完成后如何通知系统）
  
  4. **校准（可选但推荐）**（提高转换精度）

- **基础配置示例**（配置ADCINA0由软件触发采样）：
  
  ```c
  #include "driverlib.h"
  
  void main(void) {
      uint16_t adc_result0;
  
      // 1. 初始化器件
      Device_init();
      Device_initADC(); // 初始化ADC校准模式等
  
      // 2. 配置ADC时钟 (通常基于SYSCLK分频)
      // 假设SYSCLK=200MHz, 目标ADCCLK=25MHz (200 / 8 = 25)
      ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_8); // 设置ADC模块时钟分频
      ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED); // 12位，单端模式
  
      // 3. 配置一个采样通道 (SOC0)
      ADC_setupSOC(ADCA_BASE, 
                  ADC_SOC_NUMBER0,        // 使用SOC0配置
                  ADC_TRIGGER_SW_ONLY,    // 触发源：软件触发
                  ADC_CH_ADCIN0,          // 通道：ADCINA0
                  15);                    // 采样窗口持续时间 (ADCCLK周期数)
  
      // 4. 使能SOC0的触发
      ADC_enableSOC(ADCA_BASE, ADC_SOC_NUMBER0);
  
      // 5. 启动一次软件转换
      ADC_forceSOC(ADCA_BASE, ADC_SOC_NUMBER0);
  
      // 6. 等待转换完成
      while(ADC_getInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1) == false) {
          // 等待ADCINT1标志置位
      }
      ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1); // 清除标志
  
      // 7. 读取转换结果
      adc_result0 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
  
      // 将结果转换为电压: voltage = (adc_result0 / 4095.0) * 3.3;
  }
  ```

- **关键配置函数详解**：
  
  | **函数 (DriverLib)**                                       | **说明**                   | **参数示例与解释**                                                                                                                      |
  | -------------------------------------------------------- | ------------------------ | -------------------------------------------------------------------------------------------------------------------------------- |
  | `ADC_setPrescaler(base, div)`                            | **设置ADC模块时钟(ADCCLK)分频**。 | `div`: `ADC_CLK_DIV_1` ~ `ADC_CLK_DIV_16`<br>**ADCCLK = SYSCLK / div**                                                           |
  | `ADC_setMode(base, res, mode)`                           | **设置ADC分辨率和信号模式**。       | `res`: `ADC_RESOLUTION_12BIT`<br>`mode`: `ADC_MODE_SINGLE_ENDED` (单端) 或 `ADC_MODE_DIFFERENTIAL` (差分)                             |
  | `ADC_setupSOC(base, socNumber, trigger, channel, acqps)` | **配置一个采样通道(SOC)**。       | `socNumber`: `ADC_SOC_NUMBER0`~`15`<br>`trigger`: 触发源 (见下表)<br>`channel`: `ADC_CH_ADCIN0`~`15`<br>`acqps`: 采样窗口大小 (ACQPS + 1个周期) |
  | `ADC_enableSOC(base, socNumber)`                         | **使能一个SOC配置**。           | 配置SOC后必须使能。                                                                                                                      |
  | `ADC_forceSOC(base, socNumber)`                          | **通过软件强制触发一次SOC转换**。     | 用于软件启动的转换。                                                                                                                       |

- **SOC触发源 (`trigger` 参数)**：
  
  | **触发源**        | **宏**                    | **应用场景**                        |
  | -------------- | ------------------------ | ------------------------------- |
  | **软件触发**       | `ADC_TRIGGER_SW_ONLY`    | 手动启动转换，用于非实时性采样。                |
  | **ePWMx SOCA** | `ADC_TRIGGER_EPWM1_SOCA` | **最常用**。由ePWM1在特定时刻（如CMPA匹配）触发。 |
  | **ePWMx SOCB** | `ADC_TRIGGER_EPWM1_SOCB` | 由ePWM1的第二个触发事件触发。               |
  | **ePWMx INT**  | `ADC_TRIGGER_EPWM1_INT`  | 由ePWM1的中断事件触发。                  |
  | **CPU Timer**  | `ADC_TRIGGER_CPU1_TINT0` | 由CPU定时器中断触发。                    |
  | **外部引脚**       | `ADC_TRIGGER_GPIO`       | 由外部GPIO引脚跳变触发。                  |

### 3.3 高级功能与配置

#### 3.3.1 与ePWM同步触发 (核心功能)

这是实现数字电源和电机驱动等闭环控制的关键。ADC采样时刻必须与PWM波形精确同步。

```c
// 假设已配置好ePWM1
// 配置ADC的SOC0由ePWM1的SOCA信号触发，用于在PWM周期中点采样电流
ADC_setupSOC(ADCA_BASE,
            ADC_SOC_NUMBER0,
            ADC_TRIGGER_EPWM1_SOCA, // 触发源：ePWM1 SOCA
            ADC_CH_ADCIN0,          // 采样电流传感器通道
            15);                    // 采样窗口

// 在ePWM1配置中，需要设置在其周期中点产生SOCA触发信号
// 例如，在Up-Down计数模式下，在CTR=0或CTR=TBPRD时触发
EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A); // 使能ePWM1的SOCA输出
EPWM_setADCTriggerSource(EPWM1_BASE,
                        EPWM_SOC_A,
                        EPWM_SOC_TBCTR_ZERO); // 设置在计数器为0时产生SOCA
// 或者更常见的是在CMPA匹配时触发，以在PWM中点采样
// EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA);
```

#### 3.3.2 中断与DMA处理

对于多个通道的连续采样，使用中断或DMA来读取结果至关重要。

- **中断模式**：
  
  ```c
  // 1. 配置ADC在一个序列（多个SOC）转换完成后产生中断
  ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER0); // 将SOC0与ADCINT1关联
  ADC_setInterruptPulseMode(ADCA_BASE, ADC_INT_NUMBER1, ADC_PULSE_END_OF_CONV); // 转换完成后产生脉冲
  ADC_enableInterrupt(ADCA_BASE, ADC_INT_NUMBER1); // 使能ADCINT1
  
  // 2. 配置PIE中断
  Interrupt_register(INT_ADCA1, &adcA1ISR); // 注册中断服务函数 (ADCA1属于PIE组1)
  Interrupt_enable(INT_ADCA1);
  
  // 3. 在ISR中读取结果
  __interrupt void adcA1ISR(void) {
      adcResult0 = ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0);
      ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1); // 清除ADC标志
      Interrupt_clear(INT_ADCA1); // 清除PIE标志
  }
  ```

- **DMA模式 (高效，推荐)**：  
  C2000的ADC模块可以与DMA控制器联动，自动将转换结果传输到指定的内存数组，完全无需CPU干预。
  
  ```c
  #define ADC_RESULT_BUFFER_SIZE 16
  uint16_t adcResults[ADC_RESULT_BUFFER_SIZE] __attribute__((aligned(32))); // DMA需要对齐
  
  // 1. 配置DMA (简化流程，DMA配置本身是一个复杂主题)
  // 假设DMA通道1用于ADCA
  DMA_configAddress(ADCA_DMA_CHANNEL, &DMA_destAddress, (void*)adcResults, ...);
  DMA_configBurst(ADCA_DMA_CHANNEL, ADC_RESULT_BUFFER_SIZE, ...); // 设置传输大小
  DMA_configMode(ADCA_DMA_CHANNEL, DMA_TRIGGER_ADCA1, ...); // 触发源为ADCAEVT1
  
  // 2. 配置ADC在序列完成后产生DMA触发脉冲
  ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER15); // 将最后一个SOC与中断关联
  ADC_setInterruptPulseMode(ADCA_BASE, ADC_INT_NUMBER1, ADC_PULSE_END_OF_CONV);
  ADC_enableDMA(ADCA_BASE, ADC_DMA_ENABLE); // 使能ADC的DMA功能
  
  // 3. 启动后，ADC结果会自动填充到adcResults数组，完成后产生DMA中断
  ```

#### 3.3.3 硬件过采样与平均

此功能可以**有效提高有效位数(ENOB)**，抑制噪声，而无需软件进行多次采样和平均计算。

```c
// 配置ADC硬件对每个触发进行16次采样并平均，最终输出一个12位的结果
ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 15);
ADC_enableHardwareAveraging(ADCA_BASE, ADC_AVG_MODE_16_AVG); // 16次平均
// 注意：使能硬件平均后，转换时间会变长（采样窗口 * 平均次数），但分辨率显著提高。
```

### 3.4 使用示例（电机相电流采样）

#### **示例：同步采样两相电流，并使用DMA传输**

```c
#include "driverlib.h"

#define RESULTS_BUFFER_SIZE 2
uint16_t currentResults[RESULTS_BUFFER_SIZE] __attribute__((aligned(32))); // 存放U、V相电流

void initADCForCurrentSense(void) {
    // 1. ADC基础时钟配置
    ADC_setPrescaler(ADCA_BASE, ADC_CLK_DIV_8); // ADCCLK = 200MHz / 8 = 25MHz
    ADC_setMode(ADCA_BASE, ADC_RESOLUTION_12BIT, ADC_MODE_SINGLE_ENDED);

    // 2. 配置两个SOC
    // SOC0: 采样ADCINA0 (U相电流), 由EPWM1 SOCA触发
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER0, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN0, 25);
    // SOC1: 采样ADCINA1 (V相电流), 由EPWM1 SOCA触发 (连续模式，SOC0完成后立即启动SOC1)
    ADC_setupSOC(ADCA_BASE, ADC_SOC_NUMBER1, ADC_TRIGGER_EPWM1_SOCA, ADC_CH_ADCIN1, 25);

    // 3. 配置中断/DMA触发
    // 将最后一个SOC（SOC1）与ADCINT1关联，用于在序列完成后触发DMA
    ADC_setInterruptSource(ADCA_BASE, ADC_INT_NUMBER1, ADC_SOC_NUMBER1);
    ADC_setInterruptPulseMode(ADCA_BASE, ADC_INT_NUMBER1, ADC_PULSE_END_OF_CONV);
    ADC_enableDMA(ADCA_BASE, ADC_DMA_ENABLE); // 使能DMA

    // 4. 使能SOC
    ADC_enableSOC(ADCA_BASE, ADC_SOC_NUMBER0);
    ADC_enableSOC(ADCA_BASE, ADC_SOC_NUMBER1);
}

// 在ePWM配置中，需要设置PWM周期中点产生SOCA触发信号
void initEPWMForADCTigger(void) {
    // ... ePWM时基、比较器配置 ...
    EPWM_enableADCTrigger(EPWM1_BASE, EPWM_SOC_A);
    EPWM_setADCTriggerSource(EPWM1_BASE, EPWM_SOC_A, EPWM_SOC_TBCTR_U_CMPA); // 递增且等于CMPA时触发
    EPWM_setADCTriggerEventPrescale(EPWM1_BASE, EPWM_SOC_A, 1); // 每个周期都触发
}

// DMA配置 (略，需根据具体DMA通道配置)
// 主循环中，currentResults数组会自动更新为最新的U/V相电流值
```

## 4. 关键注意事项

1. **采样窗口时间 (`acqps`)**：必须足够长，让采样保持电容充放电至稳定。取决于**源阻抗**和**ADCCLK**频率。**源阻抗越高，需要的采样窗口时间越长**。计算公式：`采样时间 = (acqps + 1) * (1 / ADCCLK)`。

2. **ePWM-ADC同步**：实现高性能闭环控制的关键是**精确控制ADC采样时刻**。务必利用ePWM的SOC触发功能，在PWM波形的特定点（如中点）进行采样，以确保采样的准确性和一致性。

3. **参考电压**：ADC的转换结果依赖于参考电压(`VREFHI/VREFLO`)的精度和稳定性。对于高精度应用，建议使用外部精密基准电压源，而不是芯片内部的参考电压。

4. **模拟电路布局**：ADC的性能严重依赖于PCB布局。需遵循模拟布局规则：使用模拟地平面、充足的去耦电容（0.1uF靠近引脚）、缩短走线、避免数字信号干扰。

5. **校准**：TI芯片通常提供出厂校准值。可以使用`Device_calibrateADC()`之类的函数（如果提供）来加载这些校准值，以抵消ADC的偏移和增益误差，提高测量精度。

6. **结果对齐**：读取的12位结果在16位结果寄存器中默认是**右对齐**的。确保在转换电压值时使用正确的位掩码（`result & 0xFFF`）。

---
