#include "bsp_lsm6ds3.h"
#include "bsp_i2c.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

Angle angle;
Quaternion quaternion;

//积分时间20ms
//多少秒采集一次陀螺仪时间就填入多少秒
const static float dt = 0.02f;

//陀螺仪yaw角软件校准值
static float gyro_zero_z = 0.0f;

//实现自己的毫秒延时
static void delay_syms(long X)
{
    while(X--)
    {
        DEVICE_DELAY_US(1000);
    }
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_ReadOneByte
 * 描述  ：从LSM6DS3TRC指定地址处开始读取一个字节数据
 * 输入  ：reg_addr地址
 * 输出  ：读取的数据dat
 *******************************************************************************/
uint8_t lsm6ds3_read_one_byte(uint8_t reg_addr)
{
	uint8_t dat = 0;

    i2c_start();//发送起始信号

    i2c_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//从设备地址
    delay_syms(1);

  	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		i2c_stop();//产生一个停止条件
	}

    i2c_send_byte(reg_addr);//寄存器地址
    delay_syms(1);

  	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		 i2c_stop();//产生一个停止条件
	}

    i2c_start();//发送重复起始信号，准备读取数据

	i2c_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x01);//从设备地址（读取模式）
    delay_syms(1);

  	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		i2c_stop();//产生一个停止条件
	}

    dat = i2c_read_byte_ack(0);

	i2c_stop();//发送停止信号

    return dat;
}

/*******************************************************************************
 * 函数名：lsm6ds3_ReadCommand
 * 描述  ：对LSM6DS3TRC读取数据
 * 输入  ：uint8_t reg_addr, uint8_t *rev_data, uint8_t length
 * 输出  ：void
 *******************************************************************************/
void lsm6ds3_read_command(uint8_t reg_addr, uint8_t *rev_data, uint8_t length)
{
	while(length)
	{
		*rev_data++ = lsm6ds3_read_one_byte(reg_addr++);
		length--;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_WriteCommand
 * 描述  ：往LSM6DS3TRC写入命令
 * 输入  ：uint8_t reg_addr, uint8_t *send_data, uint16_t length
 * 输出  ：void
 *******************************************************************************/
void lsm6ds3_write_command(uint8_t reg_addr, uint8_t *send_data, uint16_t length)
{
	i2c_start();

	delay_syms(10);

	i2c_send_byte((LSM6DS3TRC_I2CADDR<<1) | 0x00);//发送设备地址

	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		i2c_stop();//产生一个停止条件
	}

	delay_syms(10);

	i2c_send_byte(reg_addr);//发送寄存器地址

	delay_syms(10);

	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		i2c_stop();//产生一个停止条件
	}

	delay_syms(10);

	i2c_send_byte(*send_data);//发送数据

	delay_syms(10);

	if(i2c_wait_ack())	// 检测设备的ACK应答
	{
		i2c_stop();//产生一个停止条件
	}

	delay_syms(10);

	i2c_stop();//产生一个停止条件
}

/*******************************************************************************
 * 函数名：IIC_CheckDevice
 * 描述  ：检测I2C总线设备，CPU向发送设备地址，然后读取设备应答来判断该设备是否存在
 * 输入  ：_Address：设备的I2C总线地址
 *******************************************************************************/
uint8_t i2c_check_device(uint8_t _Address)
{
	uint8_t ucAck;

    i2c_start();		// 发送启动信号

    i2c_send_byte(_Address );
    ucAck = i2c_wait_ack();	// 检测设备的ACK应答

    i2c_stop();			// 发送停止信号

    return ucAck;

}

/*******************************************************************************
 * 函数名：lsm6ds3_CheckOk
 * 描述  ：判断LSM6DS3TRC是否正常
 * 输出  ： 1 表示正常， 0 表示不正常
 *******************************************************************************/
uint8_t lsm6ds3_check_ok(void)
{
	if(i2c_check_device( LSM6DS3TRC_I2CADDR ) == 1)
	{
	    //printf("Device exist\r\n");
		return 1;
	}
	else
	{
		// 失败后，切记发送I2C总线停止信号
	    //printf("Device not exist\r\n");
		i2c_stop();
		return 0;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_GetChipID
 * 描述  ：读取LSM6DS3TRC器件ID
 * 输出  ：返回值true表示0x6a，返回false表示不是0x6a
 *******************************************************************************/
uint8_t lsm6ds3_get_Chip_id(void)
{
	uint8_t buf = 0;

	lsm6ds3_read_command(LSM6DS3TRC_WHO_AM_I, &buf, 1);//Who I am ID
	//printf("buf 0x%02X\r\n",buf);
	if (buf == 0x6a)
	{
	    //printf("ID ok\r\n");
		return 1;
	}
	else
	{
	    //printf("ID error\r\n");
		return 0;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_Reset
 * 描述  ：LSM6DS3TRC重启和重置寄存器
 *******************************************************************************/
void lsm6ds3_reset(void)
{
	uint8_t buf[1] = {0};
	//reboot modules
	buf[0] = 0x80;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);//BOOT->1
    delay_syms(15);
	//reset register
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);//读取SW_RESET状态
	buf[0] |= 0x01;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);//将CTRL3_C寄存器的SW_RESET位设为1
	while (buf[0] & 0x01)
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);//等到CTRL3_C寄存器的SW_RESET位返回0
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_BDU
 * 描述  ：LSM6DS3TRC设置块数据更新
 * 输入  ：uint8_t flag=1启动 0禁用
 *******************************************************************************/
void lsm6ds3_set_BDU(uint8_t flag)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);

	if (flag == 1)
	{
		buf[0] |= 0x40;//启用BDU
		lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);
	}
	else
	{
		buf[0] &= 0xbf;//禁用BDU
		lsm6ds3_write_command(LSM6DS3TRC_CTRL3_C, buf, 1);
	}

	lsm6ds3_read_command(LSM6DS3TRC_CTRL3_C, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Accelerometer_Rate
 * 描述  ：LSM6DS3TRC设置加速度计的数据采样率
 * 输入  ：uint8_t rate
 *******************************************************************************/
void lsm6ds3_set_accelerometer_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= rate;//设置加速度计的数据采样率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Gyroscope_Rate
 * 描述  ：LSM6DS3TRC设置陀螺仪数据速率
 * 输入  ：uint8_t rate
 *******************************************************************************/
void lsm6ds3_set_gyroscope_rate(uint8_t rate)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= rate;//设置陀螺仪数据速率
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Accelerometer_Fullscale
 * 描述  ：LSM6DS3TRC加速度计满量程选择
 * 输入  ：uint8_t value
 *******************************************************************************/
void lsm6ds3_set_accelerometer_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= value;//设置加速度计的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：lsm6ds3_Set_Gyroscope_Fullscale
 * 描述  ：LSM6DS3TRC陀螺仪满量程选择
 * 输入  ：uint8_t value
 *******************************************************************************/
void lsm6ds3_set_gyroscope_fullscale(uint8_t value)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL2_G, buf, 1);
	buf[0] |= value;//设置陀螺仪的满量程
	lsm6ds3_write_command(LSM6DS3TRC_CTRL2_G, buf, 1);
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Set_Accelerometer_Bandwidth
 * 描述  ：LSM6DS3TRC设置加速度计模拟链带宽
 * 输入  ：uint8_t BW0XL, uint8_t ODR
 * 备注  ：BW0XL模拟链带宽, ODR输出数据率
 *******************************************************************************/
void lsm6ds3_set_accelerometer_bandwidth(uint8_t BW0XL, uint8_t ODR)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_CTRL1_XL, buf, 1);
	buf[0] |= BW0XL;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL1_XL, buf, 1);

	lsm6ds3_read_command(LSM6DS3TRC_CTRL8_XL, buf, 1);
	buf[0] |= ODR;
	lsm6ds3_write_command(LSM6DS3TRC_CTRL8_XL, buf, 1);
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Status
 * 描述  ：从LSM6DS3TRC状态寄存器获取数据状态
 *******************************************************************************/
uint8_t lsm6ds3_get_status(void)
{
	uint8_t buf[1] = {0};
	lsm6ds3_read_command(LSM6DS3TRC_STATUS_REG, buf, 1);
	return buf[0];
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Acceleration
 * 描述  ：从LSM6DS3TRC读取加速度计数据
 * 输入  ：uint8_t fsxl, float *acc_float
 * 备注  ：转换为浮点数的加速度值
 *******************************************************************************/
void lsm6ds3_get_acceleration(uint8_t fsxl, float *acc_float)
{
	uint8_t buf[6];
	int16_t acc[3];
	lsm6ds3_read_command(LSM6DS3TRC_OUTX_L_XL, buf, 6);//获取加速度计原始数据
	acc[0] = buf[1] << 8 | buf[0];
	acc[1] = buf[3] << 8 | buf[2];
	acc[2] = buf[5] << 8 | buf[4];

	switch (fsxl)//根据不同量程来选择输出的数据的转换系数
	{
	case LSM6DS3TRC_ACC_FSXL_2G:
		acc_float[0] = ((float)acc[0] * 0.061f);
		acc_float[1] = ((float)acc[1] * 0.061f);
		acc_float[2] = ((float)acc[2] * 0.061f);
		break;

	case LSM6DS3TRC_ACC_FSXL_16G:
		acc_float[0] = ((float)acc[0] * 0.488f);
		acc_float[1] = ((float)acc[1] * 0.488f);
		acc_float[2] = ((float)acc[2] * 0.488f);
		break;

	case LSM6DS3TRC_ACC_FSXL_4G:
		acc_float[0] = ((float)acc[0] * 0.122f);
		acc_float[1] = ((float)acc[1] * 0.122f);
		acc_float[2] = ((float)acc[2] * 0.122f);
		break;

	case LSM6DS3TRC_ACC_FSXL_8G:
		acc_float[0] = ((float)acc[0] * 0.244f);
		acc_float[1] = ((float)acc[1] * 0.244f);
		acc_float[2] = ((float)acc[2] * 0.244f);
		break;
	}
}

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Gyroscope
 * 描述  ：从LSM6DS3TRC读取陀螺仪数据
 * 输入  ：uint8_t fsg, float *gry_float
 * 备注  ：转换为浮点数的角速度值
 *******************************************************************************/
void lsm6ds3_get_gyroscope(uint8_t fsg, float *gry_float)
{
	uint8_t buf[6];
	int16_t gry[3];

	lsm6ds3_read_command(LSM6DS3TRC_OUTX_L_G, buf, 6);//获取陀螺仪原始数据

	gry[0] = buf[1] << 8 | buf[0];
	gry[1] = buf[3] << 8 | buf[2];
	gry[2] = buf[5] << 8 | buf[4];
	switch (fsg)//根据不同量程来选择输出的数据的转换系数
	{
	case LSM6DS3TRC_GYR_FSG_245:
		gry_float[0] = ((float)gry[0] * 8.750f);
		gry_float[1] = ((float)gry[1] * 8.750f);
		gry_float[2] = ((float)gry[2] * 8.750f);
		break;
	case LSM6DS3TRC_GYR_FSG_500:
		gry_float[0] = ((float)gry[0] * 17.50f);
		gry_float[1] = ((float)gry[1] * 17.50f);
		gry_float[2] = ((float)gry[2] * 17.50f);
		break;
	case LSM6DS3TRC_GYR_FSG_1000:
		gry_float[0] = ((float)gry[0] * 35.00f);
		gry_float[1] = ((float)gry[1] * 35.00f);
		gry_float[2] = ((float)gry[2] * 35.00f);
		break;
	case LSM6DS3TRC_GYR_FSG_2000:
		gry_float[0] = ((float)gry[0] * 70.00f);
		gry_float[1] = ((float)gry[1] * 70.00f);
		gry_float[2] = ((float)gry[2] * 70.00f);
		break;
	}
}

/*******************************************************************************
 * 函数名：lsm6ds3_soft_calibrate_z0
 * 描述  ：传感器校准
 * 备注  ：上电前用，减小Z零点漂移
 *******************************************************************************/
static void lsm6ds3_soft_calibrate_z0(void)
{
    uint16_t calibration_samples = 500;//校准采样数
    float gz_sum = 0.0f;
    int16_t GyroZ;
    uint8_t buf[2]={0};
    uint16_t i;
    uint8_t status = 0;

    for (i = 0; i < calibration_samples; i++)
    {
        ///////////根据Z轴的变换规律进行修正/////////
        status = lsm6ds3_get_status();
        if( status & LSM6DS3TRC_STATUS_GYROSCOPE )
        {
            // 读取Z轴数据
            lsm6ds3_read_command(LSM6DS3TRC_OUTZ_L_G, buf, 2);
            GyroZ = buf[1] << 8 | buf[0];
            gz_sum += (float)GyroZ;
        }
        delay_syms(20);//要和dt同步
    }
    gyro_zero_z = gz_sum / calibration_samples;
}


// 卡尔曼滤波器结构体
typedef struct {
    float q; // 过程噪声协方差
    float r; // 测量噪声协方差
    float x; // 状态估计值
    float p; // 估计误差协方差
    float k; // 卡尔曼增益
} MPU6050_KalmanFilter;

//定义三个欧拉角的滤波体
MPU6050_KalmanFilter kf_roll, kf_pitch, kf_yaw;

/*******************************************************************************
 * 函数名：LSM6DS3TRC_Get_Gyroscope
 * 描述  ：卡尔曼滤波更新函数
 * 输入  ：kf 开发板滤波的结构体变量地址 measurement滤波前的参数
 * 返回  ：滤波后的参数
 *******************************************************************************/
static float KalmanFilter_Update(MPU6050_KalmanFilter *kf, float measurement)
{
    // 预测步骤
    kf->p = kf->p + kf->q;
    // 计算卡尔曼增益
    kf->k = kf->p / (kf->p + kf->r);
    // 更新估计值
    kf->x = kf->x + kf->k * (measurement - kf->x);
    // 更新估计误差协方差
    kf->p = (1 - kf->k) * kf->p;
    return kf->x;
}
//卡尔曼滤波参数初始化
static void KalmanFilter_init(void)
{
	//roll的初始化
	kf_roll.q = 0.02f;
	kf_roll.r = 0.1f;
	kf_roll.x = 0;
	kf_roll.p = 1;
	//Pitch的初始化
	kf_pitch.q = 0.02f;
	kf_pitch.r = 0.1f;
	kf_pitch.x = 0;
	kf_pitch.p = 1;
	//yaw的初始化
	kf_yaw.q = 0.02f;
	kf_yaw.r = 0.1f;
	kf_yaw.x = 0;
	kf_yaw.p = 1;
}

float acc[3] = {0,0,0};
float gyr[3] = {0,0,0};

float Kp=130.0f;
float Ki=0.005f;

float halfT=0.001f;
float q0=1, q1 = 0, q2 = 0, q3 = 0;
float exInt = 0, eyInt = 0, ezInt = 0;

//如yaw值偏移逐渐加大，则加大该值
//如yaw值偏移逐渐减小，则减小该值
float z_offset = 0.1034f;	//yaw轴手动偏移

//获取原始数据并转换为欧拉角
//参数angle 为传入的角度结构体
void lsm6ds3_get_angle(Angle* angle)
{
    int i = 0;
    uint8_t status = 0;
    float norm;
    float vx,vy,vz;
    float ex,ey,ez;

	//获取传感器状态
    status = lsm6ds3_get_status();

    if( (status&LSM6DS3TRC_STATUS_ACCELEROMETER) && (status&LSM6DS3TRC_STATUS_GYROSCOPE) )
    {
		//获取加速度原始数据
        lsm6ds3_get_acceleration(LSM6DS3TRC_ACC_FSXL_2G, acc);
		//单位换算
        for( i = 0; i < 3; i++ )
        {
			acc[i] = acc[i] / 1000.0f;
        }
		//获取陀螺仪原始数据
        lsm6ds3_get_gyroscope(LSM6DS3TRC_GYR_FSG_2000, gyr);
		//单位换算
        for( i = 0; i < 3; i++ )
        {
			//Z轴应用软件校准值
            if( i == 2 )
            {
                gyr[i] = gyr[i] - (int16_t)gyro_zero_z;
            }
            gyr[i] = gyr[i] /1000.0f;
        }

        //测量正常化，三维向量变为单位向量
        norm = sqrt(acc[0]*acc[0] + acc[1]*acc[1] + acc[2]*acc[2]);
        acc[0] = acc[0] / norm;//单位化
        acc[1] = acc[1] / norm;
        acc[2] = acc[2] / norm;
        //估计方向的重力
        vx = 2* (q1*q3 - q0*q2);
        vy = 2* (q0*q1 + q2*q3);
        vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

        ex = (acc[1]*vz - acc[2]*vy);
        ey = (acc[2]*vx - acc[0]*vz);
        ez = (acc[0]*vy - acc[1]*vx);

        //积分误差比例积分增益
        exInt = exInt + ex*Ki;
        eyInt = eyInt + ey*Ki;
        ezInt = ezInt + ez*Ki;
        //调整后的陀螺仪测量
        gyr[0] = gyr[0] + Kp*ex + exInt;
        gyr[1] = gyr[1] + Kp*ey + eyInt;
        gyr[2] = gyr[2] + Kp*ez + ezInt;
        //整合四元数率和正常化
        q0 = q0 + (-q1*gyr[0] - q2*gyr[1]- q3*gyr[2])*halfT;
        q1 = q1 + (q0*gyr[0] + q2*gyr[2] - q3*gyr[1])*halfT;
        q2 = q2 + (q0*gyr[1] - q1*gyr[2] + q3*gyr[0])*halfT;
        q3 = q3 + (q0*gyr[2] + q1*gyr[1] - q2*gyr[0])*halfT;
        //正常化四元数
        norm= sqrt(q0*q0+ q1*q1+ q2*q2+ q3*q3);
        //四元素
		q0 = q0 / norm;//w
        q1 = q1 / norm;//x
        q2 = q2 / norm;//y
        q3 = q3 / norm;//z

        angle->x = KalmanFilter_Update(&kf_roll, asin(2 * q2 * q3 + 2 * q0 * q1 ) * 57.3);
        angle->y =KalmanFilter_Update(&kf_pitch, atan2(-2 * q1 * q3 + 2 * q0 * q2, q0*q0-q1*q1-q2*q2+q3*q3)*57.3);
        gyr[2] = KalmanFilter_Update(&kf_yaw, gyr[2] * dt);
		//如参数小于预期请加大3.0f
		//如参数大于预期请减小3.0f
        angle->z +=  gyr[2] * 3.0f + z_offset;
    }
}

/*******************************************************************************
 * 函数名：lsm6ds3_angle_return_zero
 * 描述  ：角度归零初始化
 *******************************************************************************/
void lsm6ds3_angle_return_zero(void)
{
    angle.x = 0;
    angle.y = 0;
    angle.z = 0;
    lsm6ds3_reset();
}
/*******************************************************************************
 * 函数名：传感器初始化
 * 描述  ：lsm6ds3_init
 *******************************************************************************/
uint8_t lsm6ds3_init(void)
{
    //检测设备是否存在
    if( lsm6ds3_check_ok() == 0 ) return 1;

    //设备重启
    lsm6ds3_reset();

    //在读取MSB和LSB之前不更新输出寄存器
    lsm6ds3_set_BDU(1);

    //设置加速度计的数据采样率 1/52=19.2ms
    lsm6ds3_set_accelerometer_rate(LSM6DS3TRC_ACC_RATE_52HZ);

    //设置陀螺仪的数据采样率 1/52=19.2ms
    lsm6ds3_set_gyroscope_rate(LSM6DS3TRC_GYR_RATE_52HZ);

    //设置加速度计满量程选择
    lsm6ds3_set_accelerometer_fullscale(LSM6DS3TRC_ACC_FSXL_2G);

    //设置陀螺仪全量程选择
    lsm6ds3_set_gyroscope_fullscale(LSM6DS3TRC_GYR_FSG_2000);

    //设置加速度计模拟链带宽
    lsm6ds3_set_accelerometer_bandwidth(LSM6DS3TRC_ACC_BW0XL_400HZ, LSM6DS3TRC_ACC_LOW_PASS_ODR_100);

    delay_syms(100);

    //软件校准,减少yaw的零点漂移
    lsm6ds3_soft_calibrate_z0();

	//卡尔曼滤波初始化
	KalmanFilter_init();

    return 0;
}
