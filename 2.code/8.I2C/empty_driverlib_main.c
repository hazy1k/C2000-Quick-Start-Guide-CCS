#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "bsp_uart.h"
#include "bsp_lsm6ds3.h"
#include "stdio.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

//浮点转字符串
//只转换小数点后两位
void float_to_str(float num, char* output)
{
    int num_int = 0;
    int point1;
    int point2;

    num_int = num * 100;
    point1 = num_int/10%10;
    point2 = num_int%10;
    num_int = num_int/100;

    if( point2 < 0 ) point2=-point2;
    if( point1 < 0 ) point1=-point1;
    sprintf(output, "%d.%d%d",num_int, point1, point2);
}

void main(void)
{
    char x[20]={0},y[20]={0},z[20]={0},bufs[50]={0};

    Device_init();
    Device_initGPIO();
    Interrupt_initModule();
    Interrupt_initVectorTable();
    Board_init();
    C2000Ware_libraries_init();
    EINT;
    ERTM;
    // 陀螺仪初始化
    lsm6ds3_init();
    while(1)
    {
        //获取欧拉角
        lsm6ds3_get_angle(&angle);
        //将欧拉角的浮点型数据转换字符串
        float_to_str(angle.x,x);
        float_to_str(angle.y,y);
        float_to_str(angle.z,z);
        //格式化字符串
        sprintf(bufs,"xyz: %s,%s,%s\r\n",x,y,z);
        //串口发送字符串
        uart0_sendString(bufs);

        DEVICE_DELAY_US(15000);
    }
}
