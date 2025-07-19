# 第四章 SCI介绍及基础使用

## 1. F28P550的串口介绍

TMS320F28P550集成了三个高性能串行通信接口（SCI），分别是SCI-A、SCI-B、SCI-C，它们支持全双工异步通信，每个接口独立工作。在开发板上的支持串口的引脚见下方的开发板引脚示意图：（橙色的SCI）

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214542.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214600.png)

串口​​通信参数

- ​波特率范围​​：最高支持`​​12.5Mbps​`​（具体取决于系统时钟分频配置）。

​​数据格式​​：

- `数据位`：5 - 9 位可编程
- `停止位`：1 / 1.5 / 2 位可选
- `校验位`：无校验 / 奇校验 / 偶校验
- `​硬件流控制`​​：支持RTS（请求发送）和CTS（清除发送）信号，防止数据溢出。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250521_214206.png)

## 2. SCI使用示例

### 2.1 CCS&syscfg配置

打开工程下的 .syscfg 文件。找到 SCI 选项开始配置：

- 配置串口参数为波特率 115200、8 位数据长度、1 位停止位、无校验位。

- 开启串口发送与接收功能，并开启串口接收中断。

- 使用 SCIA-RX = GPIO28 和 SCIA-TX = GPIO29。

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250516_134343.png)

![](https://wiki.lckfb.com/storage/images/zh-hans/tjx-tms320f28p550/beginner/uart/uart_20250516_134432.png)

### 2.2 用户代码

```c
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

```

## 3. 相关函数介绍

```c
//*****************************************************************************
//
//! 等待并通过指定串口发送一个字符数组。
//!
//! \param base 是SCI端口的基地址。
//! \param array 是要发送的字符数组地址。
//!   这是一个指向待发送字符数组的指针。
//! \param length 是数组的长度，即需要发送的字符数量。
//!
//! 从指定端口的发送缓冲区（如果启用了发送FIFO则为FIFO）中发送从\e array地址开始的\e length个字符。
//! 如果发送缓冲区或FIFO没有可用空间，该函数将阻塞等待直到有足够空间，并在所有\e length个字符成功发送后返回。
//! 注意：\e array虽然是指向uint16_t的指针，但只有最低有效8位会被写入SCI端口，因为SCI仅支持8位字符传输。
//!
//! \return 无返回值。
//
//*****************************************************************************
void SCI_writeCharArray(uint32_t base, const uint16_t * const array, uint16_t length);

//*****************************************************************************
//
//! 在FIFO增强功能未启用时，阻塞式等待并读取指定端口的单个字符。
//!
//! \param base 是SCI端口的基地址。
//!
//! 从指定端口的接收缓冲区（非FIFO模式）获取字符。若缓冲区无可用字符，
//! 此函数将阻塞等待直到接收到数据后才返回。
//!
//! \return 返回从指定端口读取的字符，以\e uint16_t形式表示。
//
//*****************************************************************************/
static inline uint16_t SCI_readCharBlockingNonFIFO(uint32_t base)

/*****************************************************************************
//
//! 逐字节填充指定内存区域，常用于内存初始化与清零操作
//!
//! \param s 目标内存块的首地址指针，支持任意类型内存（如数组、结构体等）
//! \param c 填充值（将被转换为无符号字符，仅低8位有效）
//! \param n 需填充的字节数（单位：字节）
//!
//! 功能说明：
//! 1. 将内存块的前  n 个字节设置为  c 的ASCII码值[1,5,7](@ref)
//! 2. 典型用途包括：
//!    - 清零操作：将  c 设为0以初始化内存或清除敏感数据[3,6](@ref)
//!    - 字符填充：如将字符串缓冲区填充为特定字符（需保留终止符'\0'）[2,5](@ref)
//!    - 结构体初始化：快速清空复杂数据结构[3,8](@ref)
//!
//! 注意事项：
//! - 对非字符类型（如int数组）赋值非0/-1值时，可能导致非预期结果[2,7](@ref)
//!    （因逐字节填充可能破坏多字节数据类型的存储结构）
//! - 避免溢出：确保  n 不超过目标内存的物理大小[2,6](@ref)
//! - 字符串操作时需手动处理终止符，避免覆盖导致未定义行为[5,7](@ref)
//!
//! \return 指向目标内存块的指针（即  s 的原始地址）
//
//*****************************************************************************/
void *memset(void *s, int c, size_t n);

/*****************************************************************************
//
//! 计算以空字符('\0')结尾的字符串的字节长度
//!
//! \param str 指向待计算字符串的指针，必须为以'\0'结尾的有效字符串
//!
//! 功能说明：
//! 1. 从起始地址遍历内存，直到遇到第一个'\0'字符为止，统计遍历的字节数
//! 2. 不包含终止符'\0'本身的长度
//! 3. 适用于ASCII、ISO-8859-1等单字节编码字符串
//!
//! 注意事项：
//! - 安全性：若传入未初始化的指针或非字符串内存区域，可能导致未定义行为[2,10](@ref)
//! - 多字节字符：对UTF-8等编码的字符串，返回值是字节数而非字符数（如"中文"返回6字节）[11,12](@ref)
//! - 性能影响：时间复杂度为O(n)，高频调用时需考虑缓存计算结果[5,12](@ref)
//! - 与sizeof区别：sizeof计算数组总大小（含未使用空间），strlen统计实际字符数[6,10](@ref)
//!
//! \return 返回无符号整数类型 size_t，表示字符串的字节长度
//
//*****************************************************************************/
size_t strlen(const char *str);
```

---


