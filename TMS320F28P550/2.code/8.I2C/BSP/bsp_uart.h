#ifndef BSP_UART_H
#define BSP_UART_H

#include "driverlib.h"
#include "device.h"
#include "board.h"

void uart0_sendChar(char ch);
void uart0_sendString(char* str);

#endif
