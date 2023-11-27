#include "mpu6050.h"

mpu_msg_t mpu_raw_msg;   // 原始数据
pose_msg_t mpu_pose_msg; // 姿态数据
accel_msg_t mpu_accel_msg;

/* ------------------------------------------------ 初始化mpu6050 ------------------------------------------------------------ */
/**
 * @brief  MPU_Read_Device_Addr()  		初始化mpu6050.
 * @note   														初始化mpu6050
 * @retval HAL status									mpu_ok/mpu_err
 */
mpu_state_t w_mpu_init(void)
{
    static uint8_t device_id = 0x00u;

    // MPU原始数据清零
    for (int i = 0; i < sizeof(mpu_raw_msg); i++)
        ((uint8_t *)&mpu_raw_msg)[i] = 0;

    // 这里可以不用初始配置gpio端口, CUBEMX生成开头会有初始化I2C的gpio端口
    //	MPU_I2C_GPIO_INIT();

    // 检查ID
    device_id = w_mpu_read_device_address();
    if (device_id == mpu_id_err)
        return mpu_id_err; // ID错误

    /* 注意：MPU6000 & MPU6050设备在'上电时'会进入'睡眠模式' */

    // 复位MPU6050,	0x08
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_reset);
    HAL_Delay(50);

    // 唤醒MPU6050,	0x00
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_rouse);
    //	HAL_Delay(50);

    // 陀螺量程范围，±2000dps.		0x03
    w_mpu_set_gyro_fsr(mpu_gyro_fsr_2000s);

    // 加速度计程范围，±8g.			0x00
    w_mpu_set_acce_fsr(mpu_acce_fsr_2g);

    // 配置采样频率分频寄存器, 1k/4, 取样率为25Hz。
    w_mpu_set_sample_rate(0x07);

    // 配置低通滤波，截止频率100Hz左右。
    w_mpu_set_dlpf(2);

    // 中断信号为高电平, 直到有读取操作才消失, 直通辅助I2C。
    w_mpu_write_byte(_MPU_INT_BP_CFG_REG, 0x80);

    // 使能 "数据就绪" 中断 (Data Ready interrupt) ,所有的传感器寄存器写操作完成时都会产生
    w_mpu_write_byte(_MPU_INT_ENABLE_REG, 0x00);

    // 不使用辅助I2C.
    w_mpu_write_byte(_MPU_USER_CTRL, 0x00);

    // 设备的系统时钟 与 X轴陀螺仪参考.
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_clksel_x);

    // 加速度与陀螺仪都工作
    w_mpu_write_byte(_MPU_PWR_MGMT_2, 0x00);

    return mpu_ok;
}

/*  读取MPU的设备ID  */
uint8_t w_mpu_read_device_address(void)
{
    static uint8_t id = 0x00u; // 存放读取到的数据（设备ID）

    // 0x75
    id = w_mpu_read_byte(_MPU_DEVICE_ID);
    if (id == mpu_err) {
        userPrint("ID_ERROR:0x%x\r\n", id);
        return mpu_id_err; // 设备id错误
    }
    return id; // 返回设备ID
}

/*  读取MPU温度  */
float w_mpu_read_temperature(void)
{
    static uint8_t msg_temp[2];        // 缓冲区
    static short mpu_temperature_biff; // 温度缓冲区

    // 读取内置温度传感器数据
    if (w_mpu_read_byte_len(_MPU_TEMP_OUT_H_REG, msg_temp, sizeof(msg_temp)) == mpu_err)
        return mpu_err;

    mpu_temperature_biff = ((uint16_t)msg_temp[0] << 8) | msg_temp[1]; // MPU温度值

    return (36.53f + ((double)mpu_temperature_biff) / 340.0f); // 计算后的真实温度值
}

/*  读取MPU所有原始数据  */
mpu_state_t w_mpu_read_all_raw_data(mpu_msg_t *mpu_raw_msg_buff)
{

    // 0x3B, （加速度计 --> 温度 --> 陀螺仪）数据地址先后顺序

    uint8_t msg_buff[14];              // 存入缓冲区
    static short mpu_temperature_biff; // 温度缓冲区

    if (w_mpu_read_byte_len(_MPU_RAW_DATA_REG, msg_buff, sizeof(msg_buff)) == mpu_err)
        //		return mpu_busy;								//iic忙碌（0x02/0x2）
        return mpu_err;

    // 整合MPU原始数据
    mpu_raw_msg_buff->mpu_acce[0] = ((uint16_t)msg_buff[0] << 8) | msg_buff[1]; // X轴加速度值
    mpu_raw_msg_buff->mpu_acce[1] = ((uint16_t)msg_buff[2] << 8) | msg_buff[3]; // Y轴加速度值
    mpu_raw_msg_buff->mpu_acce[2] = ((uint16_t)msg_buff[4] << 8) | msg_buff[5]; // Z轴加速度值

    mpu_raw_msg_buff->mpu_gyro[0] = ((uint16_t)msg_buff[8] << 8) | msg_buff[9];   // X轴陀螺仪值
    mpu_raw_msg_buff->mpu_gyro[1] = ((uint16_t)msg_buff[10] << 8) | msg_buff[11]; // Y轴陀螺仪值
    mpu_raw_msg_buff->mpu_gyro[2] = ((uint16_t)msg_buff[12] << 8) | msg_buff[13]; // Z轴陀螺仪值

    mpu_temperature_biff = ((uint16_t)msg_buff[6] << 8) | msg_buff[7]; // MPU温度值

    mpu_raw_msg_buff->mpu_temperature = 36.53f + ((double)mpu_temperature_biff) / 340.0f; // 计算后的真实温度值

    return mpu_ok;
}

/*  配置采样频率分频寄存器  */
mpu_state_t w_mpu_set_sample_rate(uint16_t rate)
{
    // 0x19, 设置数字低通滤波器
    if (w_mpu_write_byte(_MPU_SAMPLE_RATE_DIVIDER_REG, rate) == mpu_ok)
        return mpu_ok; // 配置成功
    else
        return mpu_err; // 配置失败
}

/*  配置数字低通滤波器  */
mpu_state_t w_mpu_set_dlpf(uint16_t dlpf)
{
    // 0x1A,  截止频率100Hz左右。
    if (w_mpu_write_byte(_MPU_CONFIG_REG, dlpf) == mpu_ok)
        return mpu_ok; // 配置成功
    else
        return mpu_err; // 配置失败
}

/*  配置陀螺仪满量程范围  */
mpu_state_t w_mpu_set_gyro_fsr(mpu_fsr_t gyro_fsr)
{
    // 0x1B
    if (w_mpu_write_byte(_MPU_GYRO_CONFIG_REG, gyro_fsr << 3) == mpu_ok)
        return mpu_ok; // 配置成功
    else
        return mpu_err; // 配置失败
}

/*  配置加速度计满量程范围  */
mpu_state_t w_mpu_set_acce_fsr(mpu_fsr_t acce_fsr)
{
    // 0x1C
    if (w_mpu_write_byte(_MPU_ACCE_CONFIG_REG, acce_fsr << 3) == mpu_ok)
        return mpu_ok; // 配置成功
    else
        return mpu_err; // 配置失败
}

/*  mpu写入一个Byte  */
mpu_state_t w_mpu_write_byte(uint8_t reg_addr, uint8_t w_data)
{
    uint8_t w_data_byte = w_data; // 放入要写入的数据

    if (HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, _MPU_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, &w_data_byte, 1, 0xfff) == HAL_OK)
        return mpu_ok; // 写入成功
    else
        return mpu_err; // 写入失败}
}
/*  mpu读取一个Byte  */
uint8_t w_mpu_read_byte(uint8_t reg_addr)
{
    static uint8_t r_data_byte = 0x00u; // 存放读取一个Byte的数据
    if (HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, _MPU_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, &r_data_byte, 1, 0xfff) == HAL_OK)
        //	HAL_Delay(50);
        return r_data_byte; // 返回读取的数据
    else
        return mpu_err;
}

/*  mpu连续写入多个Byte  */
mpu_state_t w_mpu_write_byte_len(uint8_t reg_addr, uint8_t *w_data_buf, uint8_t len)
{

    if (HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, _MPU_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, w_data_buf, len, 50) == HAL_OK)
        return mpu_ok; // 写入成功
    else
        return mpu_err; // 写入失败}
}
/*  mpu连续读取多个Byte  */
mpu_state_t w_mpu_read_byte_len(uint8_t reg_addr, uint8_t *r_data_buf, uint8_t len)
{

    if (HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, _MPU_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, r_data_buf, len, 50) == HAL_OK)
        //		HAL_Delay(50);
        return mpu_ok; // 读取完成
    else 
        return mpu_err;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* DMP (inv_mpu.c inv_mpu_dmp_motion_driver.c) 需要用到 CODE BEGIN */

void my_hal_delay(unsigned long ms)
{
    HAL_Delay(ms);
}

/**
 * @brief  get_tick_ms_cnt()  			ms级别时间戳, 为DMP库get_ms
 */
void f1_get_tick_ms_cnt(unsigned long *cnt)
{
    *cnt = HAL_GetTick();
}

/**
 * @brief  get_tick_ms_cnt1()  			ms级别时间戳,可以为空, 使用中不起作用, 为DMP库get_ms不报错
 */
void f1_get_tick_ms_cnt1(unsigned long *cnt)
{
    ;
}

uint8_t dmp_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char len, unsigned char *w_data)
{
    // 超时时间看实际更改, 问题不大
    return HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, w_data, len, 5);
}

uint8_t dmp_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char len, unsigned char *r_data)
{
    // 超时时间看实际更改, 问题不大
    return HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, r_data, len, 5);
}

/* DMP (inv_mpu.c inv_mpu_dmp_motion_driver.c) 需要用到 CODE END  */

/* 内置DMP的应用 CODE BEGIN */

#define DEFAULT_MPU_HZ (100) // 输出的速度, 100Hz, 10ms
// #define DEFAULT_MPU_HZ  (200)			//输出的速度, 200Hz, 20ms

// q30格式,long转float时的除数.
#define q30 1073741824.0f

uint8_t buffer[14];
int16_t MPU6050_FIFO[6][11];

int16_t Gx_offset = 0, Gy_offset = 0, Gz_offset = 0;

// 陀螺仪方向设置, 绝对位置
static signed char gyro_orientation[9] = {1, 0, 0,
                                          0, 1, 0,
                                          0, 0, 1};

// 方向转换
static unsigned short inv_row_2_scale(const signed char *row)
{
    unsigned short b;

    if (row[0] > 0)
        b = 0;
    else if (row[0] < 0)
        b = 4;
    else if (row[1] > 0)
        b = 1;
    else if (row[1] < 0)
        b = 5;
    else if (row[2] > 0)
        b = 2;
    else if (row[2] < 0)
        b = 6;
    else
        b = 7; // error
    return b;
}

// 陀螺仪方向控制
static unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;
    /*
            //恒等矩阵
            {1, 0, 0,
             0, 1, 0,
             0, 0, 1};

     */
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;

    return scalar;
}

// MPU6050传感器自测
static void run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x03) {

        // 测试通过。我们可以相信陀螺仪的数据，把它放下来到DMP
        /* Test passed. We can trust the gyro data here, so let's push it down
         * to the DMP.
         */
        float sens;
        unsigned short accel_sens;
        mpu_get_gyro_sens(&sens);
        gyro[0] = (long)(gyro[0] * sens);
        gyro[1] = (long)(gyro[1] * sens);
        gyro[2] = (long)(gyro[2] * sens);
        dmp_set_gyro_bias(gyro);
        mpu_get_accel_sens(&accel_sens);
        accel[0] *= accel_sens;
        accel[1] *= accel_sens;
        accel[2] *= accel_sens;
        dmp_set_accel_bias(accel);
        userPrint(".");
    } else
        userPrint("\r\nsetting bias failed.\r\n");
}
// DMP初始化
void dmp_init(void)
{
    uint8_t temp[1] = {0};
    dmp_i2c_read(HW_S_HW_MPU_ADDR, _MPU_DEVICE_ID, I2C_MEMADD_SIZE_8BIT, temp);
    //	 temp[0] = w_mpu_read_device_address();
    // userPrint("mpu_set_sensor complete.\r\n");
    HAL_Delay(1);
    userPrint("Device_ID:0x%x\r\n", temp[0]);
    HAL_Delay(1);

    // 检查ID, 不对系统复位
    if (temp[0] != 0x68)
        NVIC_SystemReset();

    if (!mpu_init()) {
        // 重启内部寄存器到默认,
        if (!mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL))
            userPrint(".");
        else
            userPrint("\r\nmpu_set_sensor failed ......\r\n");
        // 配置FIFO
        if (!mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL))
            userPrint(".");
        else
            userPrint("\r\nmpu_configure_fifo failed ......\r\n");
        // 配置采样频率分频寄存器
        if (!mpu_set_sample_rate(DEFAULT_MPU_HZ))
            userPrint(".");
        else
            userPrint("\r\nmpu_set_sample_rate failed ......\r\n");

        // DMP加载动作驱动程序固件完成
        if (!dmp_load_motion_driver_firmware())
            userPrint(".");
        else
            userPrint("\r\ndmp_load_motion_driver_firmware ......\r\n");

        // DMP设置陀螺仪方向完成
        if (!dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation)))
            userPrint(".");
        else
            userPrint("\r\ndmp_set_orientation failed ......\r\n");

        // DMP使功能完整
        if (!dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                                DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
                                DMP_FEATURE_GYRO_CAL))
            userPrint(".");
        else
            userPrint("\r\ndmp_enable_feature failed ......\r\n");

        // DMP设定FIFO速率完成
        if (!dmp_set_fifo_rate(DEFAULT_MPU_HZ))
            userPrint(".");
        else
            userPrint("\r\ndmp_set_fifo_rate failed ......\r\n");

        // 自测自身数据,测试通过. 我们可以相信陀螺仪的数据，把它放下来到DMP
        run_self_test();

        // mpu设置DMP状态完成
        if (!mpu_set_dmp_state(1))
            userPrint(".\r\n");
        else
            userPrint("\r\nmpu_set_dmp_state failed ......\r\n");
        my_hal_delay(20);

        // 先读取姿态角一段时间, 先稳定
        for (uint16_t rd = 0; rd < 5; rd++) // 6s
        {
            // 读取MPU内置DMP的姿态
            read_dmp(&mpu_pose_msg, &mpu_accel_msg);
            userPrint("Pitch: %f Roll: %f Yaw: %f\n", mpu_pose_msg.pitch, mpu_pose_msg.roll, mpu_pose_msg.yaw);
            my_hal_delay(20);
        }
        my_hal_delay(10);
    }
}

uint8_t read_dmp(pose_msg_t *pose_msg, accel_msg_t *mpu_accel_msg)
{

    uint8_t more;
    long quat[4]; // 四元数
    int16_t gyro[3], accel[3], sensors;
    unsigned long sensor_timestamp; // 传感器时间戳

    float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

    // 读取FIFO中的数据
    if (dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more))
        return 1;

    if (sensors & INV_WXYZ_QUAT) {
        q0 = quat[0] / q30;
        q1 = quat[1] / q30;
        q2 = quat[2] / q30;
        q3 = quat[3] / q30;

        pose_msg->pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3f;                                      // 四元数解算, 俯仰角(Pitch)-->绕着X轴旋转
        pose_msg->roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3f;      // 四元数解算, 翻滚角(Roll)-->绕着Y轴旋转
        pose_msg->yaw   = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3f; // 四元数解算, 偏航角(Yaw)-->绕着Z轴旋转

        mpu_accel_msg->x = (4 * 9.8 * accel[0]) / 32768.0f;
        mpu_accel_msg->y = (4 * 9.8 * accel[1]) / 32768.0f;
        mpu_accel_msg->z = (4 * 9.8 * accel[2]) / 32768.0f;
        return 0;
    } else
        return 2;
}
