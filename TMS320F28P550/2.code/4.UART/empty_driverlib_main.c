#include "driverlib.h"
#include "device.h"
#include "board.h"
#include "c2000ware_libraries.h"
#include "string.h" //使用 memset(), strlen() 需导入该文件

#define uart_rx_max 50              //定义最大接收长度
uint16_t rDataA[ uart_rx_max ]={0}; //定义串口接收数据的缓冲区
uint16_t rx_len = 0;                //串口接收数据长度
uint16_t rx_flag = 0;               //串口是否有接收到数据的标志位 =0没有 =1有

//任意毫秒的延时
void delay_ms(int x)
{
    while(x--)
    {
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

    while(1)
    {
        //如果串口接收标志位为1
        if( rx_flag== 1 )
        {
            rx_flag=0;
            GPIO_togglePin(User_LED);
            //发送数据
            SCI_writeCharArray(SCIA_BASE, rDataA, strlen(rDataA));
            //清除数据
            memset(rDataA, 0, sizeof(rDataA));
            rx_len=  0;
        }
        delay_ms(1000);
    }
}

__interrupt void INT_User_UART_RX_ISR(void)
{
    //没有开启FIFO的情况下，等待接收区有数据了就返回数据
    rDataA[rx_len] = SCI_readCharBlockingNonFIFO(SCIA_BASE);
    //回环长度限制
    rx_len = ( rx_len + 1 ) % uart_rx_max;
    //设置接收标志位为1
    rx_flag = 1;

    SCI_clearInterruptStatus(SCIA_BASE, SCI_INT_RXFF);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP9);
}
