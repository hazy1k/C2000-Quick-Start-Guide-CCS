#include "bsp_uart.h"
#include <string.h>
#include <stdio.h>

//发送单个字节
void uart0_sendChar(char ch)
{
    while( SCI_isTransmitterBusy(mySCI0_BASE) != false);

    SCI_writeCharNonBlocking(mySCI0_BASE, ch);
}

//发送字符串
void uart0_sendString(char* str)
{
    while(*str!=0 && str != 0)
    {
        uart0_sendChar(*str++);
    }
}
