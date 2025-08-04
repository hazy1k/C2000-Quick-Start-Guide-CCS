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

## 3. 相关函数介绍

```c
//*****************************************************************************
//
//! 通过软件强制设置模数转换器中多个SOC（Start-Of-Conversion）标志位为1。
//!
//! \param base 是ADC模块的基地址。
//! \param socMask 指定要通过软件强制的SOC掩码
//!
//! 本函数会强制设置与\e socMask参数指定的SOC相关联的SOCFRC1标志位。
//! 当指定SOC获得优先级后，这将启动一次转换。无论该SOC是否已配置为接受其他特定触发，
//! 都可以使用此软件触发方式。
//! socMask参数的有效值可以是单个ADC_FORCE_SOCx值，也可以是它们的或(OR)组合，
//! 以触发多个SOC。
//!
//! \note 若要触发SOC0、SOC1和SOC2，应传入(ADC_FORCE_SOC0 | ADC_FORCE_SOC1 | ADC_FORCE_SOC2)
//! 作为socMask参数值。
//!
//! \return 无返回值。
//
//*****************************************************************************
static inline void ADC_forceMultipleSOC(uint32_t base, uint16_t socMask);

//*****************************************************************************
//
//! 检测ADC是否处于忙碌状态。
//!
//! \param base 是ADC模块的基地址。
//!
//! 本函数允许调用者判断ADC是否正在忙碌，以及是否可以采样另一个通道。
//!
//! \return 如果ADC正在采样则返回\b true，如果所有采样都已完成则返回\b false。
//
//*****************************************************************************
static inline bool ADC_isBusy(uint32_t base);

//*****************************************************************************
//
//! 读取转换结果。
//!
//! \param resultBase 是ADC结果寄存器的基地址。
//! \param socNumber 是启动转换(SOC)的编号。
//!
//! 本函数返回与传入的\e resultBase基地址和\e socNumber参数对应的SOC转换结果。
//!
//! socNumber参数取值应为\b ADC_SOC_NUMBERX，其中X是0到15的数字，
//! 用于指定要读取哪个SOC的结果。
//!
//! \note 注意确保使用的是结果寄存器的基地址(ADCxRESULT_BASE)，
//! 而不是控制寄存器的基地址。
//!
//! \return 返回转换结果值。
//
//*****************************************************************************
static inline uint16_t ADC_readResult(uint32_t resultBase, ADC_SOCNumber socNumber);
```

---


