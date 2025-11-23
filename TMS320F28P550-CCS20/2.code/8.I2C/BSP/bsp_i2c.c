#include "bsp_i2c.h"

#define delay_us(X)		DEVICE_DELAY_US(X)  //I2C时序的微秒延时
#define I2C_DELAY_TIME  3                   //I2C时序的延时时间

//发送开始信号
void i2c_start(void)
{
    SDA_OUT();
    SCL(0);
    SDA(1);
    SCL(1);
    delay_us(I2C_DELAY_TIME);
    SDA(0);
    delay_us(I2C_DELAY_TIME);
    SCL(0);
    delay_us(I2C_DELAY_TIME);
}

//发送停止信号
void i2c_stop(void)
{
    SDA_OUT();
    SCL(0);
    SDA(0);
    SCL(1);
    delay_us(I2C_DELAY_TIME);
    SDA(1);
    delay_us(I2C_DELAY_TIME);

}

/******************************************************************
 * 函 数 说 明：主机发送应答或者非应答信号
 * 函 数 形 参：0发送应答  1发送非应答
******************************************************************/
void i2c_send_ack(unsigned char ack)
{
    SDA_OUT();
    SCL(0);
    SDA(0);
    delay_us(I2C_DELAY_TIME);
    if(!ack) SDA(0);
    else     SDA(1);
    SCL(1);
    delay_us(I2C_DELAY_TIME);
    SCL(0);
    SDA(1);
}
/******************************************************************
 * 函 数 说 明：等待从机应答
 * 函 数 返 回：0有应答  1超时无应答
******************************************************************/
unsigned char i2c_wait_ack(void)
{
    char ack = 0;
    char ack_flag = 50;
    SDA_IN();
    SDA(1);
    while( (SDA_GET()==1) && ( ack_flag ) )
    {
        ack_flag--;
        delay_us(I2C_DELAY_TIME);
    }

    if( ack_flag == 0 )
    {
        i2c_stop();
        return 1;
    }
    else
    {
        SCL(1);
        delay_us(I2C_DELAY_TIME);
        SCL(0);
        SDA_OUT();
    }
    return ack;
}

/******************************************************************
 * 函 数 说 明：写入一个字节
 * 函 数 形 参：dat要写入的数据
******************************************************************/
void i2c_send_byte(uint8_t dat)
{
    int i = 0;
    SDA_OUT();
    SCL(0);//拉低时钟开始数据传输
    for( i = 0; i < 8; i++ )
    {
        SDA( (dat & 0x80) >> 7 );
        delay_us(2);
        SCL(1);
        delay_us(I2C_DELAY_TIME);
        SCL(0);
        delay_us(I2C_DELAY_TIME);
        dat<<=1;
    }
}

/******************************************************************
*	函 数 名: i2c_read_byte_ack
*	功能说明: CPU从I2C总线设备读取一个字节数据
*	形    参：ack ： 1发送ACK应答，0发送nACK非应答
*	返 回 值: 读到的数据
******************************************************************/
unsigned char i2c_read_byte_ack(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
    {
        SCL(0);
        delay_us(2);
        SCL(1);
        receive<<=1;
        if(SDA_GET())
            receive++;
        delay_us(1);
    }
    if (!ack)
        i2c_send_ack(1);//发送nACK
    else
        i2c_send_ack(0); //发送ACK
    return receive;
}
