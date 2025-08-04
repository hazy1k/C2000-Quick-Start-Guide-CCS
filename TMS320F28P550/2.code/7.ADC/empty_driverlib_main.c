#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "stdio.h"
#include <stdint.h>

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
