#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "stdio.h" //导入用以支持 printf

void delay_ms(int x)
{
    while(x--)
    {
        //调用 TI 自带的微秒延时
        DEVICE_DELAY_US(1000);
    }
}

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
    // 重新配置系统时钟
    SysCtl_setClock(DEVICE_SETCLOCK_CFG);
    while(1)
    {
        // 获取当前主频并输出
        printf("clk = %ld\r\n", SysCtl_getClock(DEVICE_OSCSRC_FREQ));
        GPIO_togglePin(User_LED);
        delay_ms(1000);
    }
}
