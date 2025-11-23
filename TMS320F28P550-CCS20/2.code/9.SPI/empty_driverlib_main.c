#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "bsp_w25qxx.h"
#include <stdio.h>
#include <string.h>

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

    int flash_id=0;
    char read_write_buff[10] = {0};
    char read_write_buff2[10] = {0};

    //读取器件ID
    flash_id = w25qxx_read_id();
    DEVICE_DELAY_US(500000);
    printf("flash_id=%X\r\n",flash_id);

    //往0地址写入5个字节数据,分别是"12345"
    w25qxx_write("12345", 0x00, 5);
    DEVICE_DELAY_US(500000);

    //读取0地址的5个字节数据到buff
    w25qxx_read(read_write_buff, 0x00, 5);
    //通过CIO输出
    printf("%s\r\n",read_write_buff);
    DEVICE_DELAY_US(500000);

    /* 字符串测试 */
    w25qxx_write("hello", 0x01, sizeof(read_write_buff2));
    DEVICE_DELAY_US(500000);
    w25qxx_read(read_write_buff2, 0x01, sizeof(read_write_buff2));
    printf("%s\r\n",read_write_buff2);
    DEVICE_DELAY_US(500000);

    while(1);
}
