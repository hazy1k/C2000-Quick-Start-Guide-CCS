#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "stdio.h"

/* 读取AIO输入电平示例 */
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

    while(1)
    {
        uint32_t AIO_Flag;
        AIO_Flag = GPIO_readPin(myAIO0);
        // printf("AIO_Flag = %d\r\n", AIO_Flag);
        if(AIO_Flag) // 读取高电平，点亮LED
        {
            GPIO_writePin(LED, 0);
        }
        if(AIO_Flag == 0) // 读取低电平，熄灭LED
        {
            GPIO_writePin(LED, 1);
        }
        DEVICE_DELAY_US(10000);
    }
}
