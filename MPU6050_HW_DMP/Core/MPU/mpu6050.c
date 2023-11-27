#include "mpu6050.h"

mpu_msg_t mpu_raw_msg;   // ԭʼ����
pose_msg_t mpu_pose_msg; // ��̬����
accel_msg_t mpu_accel_msg;

/* ------------------------------------------------ ��ʼ��mpu6050 ------------------------------------------------------------ */
/**
 * @brief  MPU_Read_Device_Addr()  		��ʼ��mpu6050.
 * @note   														��ʼ��mpu6050
 * @retval HAL status									mpu_ok/mpu_err
 */
mpu_state_t w_mpu_init(void)
{
    static uint8_t device_id = 0x00u;

    // MPUԭʼ��������
    for (int i = 0; i < sizeof(mpu_raw_msg); i++)
        ((uint8_t *)&mpu_raw_msg)[i] = 0;

    // ������Բ��ó�ʼ����gpio�˿�, CUBEMX���ɿ�ͷ���г�ʼ��I2C��gpio�˿�
    //	MPU_I2C_GPIO_INIT();

    // ���ID
    device_id = w_mpu_read_device_address();
    if (device_id == mpu_id_err)
        return mpu_id_err; // ID����

    /* ע�⣺MPU6000 & MPU6050�豸��'�ϵ�ʱ'�����'˯��ģʽ' */

    // ��λMPU6050,	0x08
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_reset);
    HAL_Delay(50);

    // ����MPU6050,	0x00
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_rouse);
    //	HAL_Delay(50);

    // �������̷�Χ����2000dps.		0x03
    w_mpu_set_gyro_fsr(mpu_gyro_fsr_2000s);

    // ���ٶȼƳ̷�Χ����8g.			0x00
    w_mpu_set_acce_fsr(mpu_acce_fsr_2g);

    // ���ò���Ƶ�ʷ�Ƶ�Ĵ���, 1k/4, ȡ����Ϊ25Hz��
    w_mpu_set_sample_rate(0x07);

    // ���õ�ͨ�˲�����ֹƵ��100Hz���ҡ�
    w_mpu_set_dlpf(2);

    // �ж��ź�Ϊ�ߵ�ƽ, ֱ���ж�ȡ��������ʧ, ֱͨ����I2C��
    w_mpu_write_byte(_MPU_INT_BP_CFG_REG, 0x80);

    // ʹ�� "���ݾ���" �ж� (Data Ready interrupt) ,���еĴ������Ĵ���д�������ʱ�������
    w_mpu_write_byte(_MPU_INT_ENABLE_REG, 0x00);

    // ��ʹ�ø���I2C.
    w_mpu_write_byte(_MPU_USER_CTRL, 0x00);

    // �豸��ϵͳʱ�� �� X�������ǲο�.
    w_mpu_write_byte(_MPU_PWR_MGMT_1, mpu_pwr_mgmt_1_clksel_x);

    // ���ٶ��������Ƕ�����
    w_mpu_write_byte(_MPU_PWR_MGMT_2, 0x00);

    return mpu_ok;
}

/*  ��ȡMPU���豸ID  */
uint8_t w_mpu_read_device_address(void)
{
    static uint8_t id = 0x00u; // ��Ŷ�ȡ�������ݣ��豸ID��

    // 0x75
    id = w_mpu_read_byte(_MPU_DEVICE_ID);
    if (id == mpu_err) {
        userPrint("ID_ERROR:0x%x\r\n", id);
        return mpu_id_err; // �豸id����
    }
    return id; // �����豸ID
}

/*  ��ȡMPU�¶�  */
float w_mpu_read_temperature(void)
{
    static uint8_t msg_temp[2];        // ������
    static short mpu_temperature_biff; // �¶Ȼ�����

    // ��ȡ�����¶ȴ���������
    if (w_mpu_read_byte_len(_MPU_TEMP_OUT_H_REG, msg_temp, sizeof(msg_temp)) == mpu_err)
        return mpu_err;

    mpu_temperature_biff = ((uint16_t)msg_temp[0] << 8) | msg_temp[1]; // MPU�¶�ֵ

    return (36.53f + ((double)mpu_temperature_biff) / 340.0f); // ��������ʵ�¶�ֵ
}

/*  ��ȡMPU����ԭʼ����  */
mpu_state_t w_mpu_read_all_raw_data(mpu_msg_t *mpu_raw_msg_buff)
{

    // 0x3B, �����ٶȼ� --> �¶� --> �����ǣ����ݵ�ַ�Ⱥ�˳��

    uint8_t msg_buff[14];              // ���뻺����
    static short mpu_temperature_biff; // �¶Ȼ�����

    if (w_mpu_read_byte_len(_MPU_RAW_DATA_REG, msg_buff, sizeof(msg_buff)) == mpu_err)
        //		return mpu_busy;								//iicæµ��0x02/0x2��
        return mpu_err;

    // ����MPUԭʼ����
    mpu_raw_msg_buff->mpu_acce[0] = ((uint16_t)msg_buff[0] << 8) | msg_buff[1]; // X����ٶ�ֵ
    mpu_raw_msg_buff->mpu_acce[1] = ((uint16_t)msg_buff[2] << 8) | msg_buff[3]; // Y����ٶ�ֵ
    mpu_raw_msg_buff->mpu_acce[2] = ((uint16_t)msg_buff[4] << 8) | msg_buff[5]; // Z����ٶ�ֵ

    mpu_raw_msg_buff->mpu_gyro[0] = ((uint16_t)msg_buff[8] << 8) | msg_buff[9];   // X��������ֵ
    mpu_raw_msg_buff->mpu_gyro[1] = ((uint16_t)msg_buff[10] << 8) | msg_buff[11]; // Y��������ֵ
    mpu_raw_msg_buff->mpu_gyro[2] = ((uint16_t)msg_buff[12] << 8) | msg_buff[13]; // Z��������ֵ

    mpu_temperature_biff = ((uint16_t)msg_buff[6] << 8) | msg_buff[7]; // MPU�¶�ֵ

    mpu_raw_msg_buff->mpu_temperature = 36.53f + ((double)mpu_temperature_biff) / 340.0f; // ��������ʵ�¶�ֵ

    return mpu_ok;
}

/*  ���ò���Ƶ�ʷ�Ƶ�Ĵ���  */
mpu_state_t w_mpu_set_sample_rate(uint16_t rate)
{
    // 0x19, �������ֵ�ͨ�˲���
    if (w_mpu_write_byte(_MPU_SAMPLE_RATE_DIVIDER_REG, rate) == mpu_ok)
        return mpu_ok; // ���óɹ�
    else
        return mpu_err; // ����ʧ��
}

/*  �������ֵ�ͨ�˲���  */
mpu_state_t w_mpu_set_dlpf(uint16_t dlpf)
{
    // 0x1A,  ��ֹƵ��100Hz���ҡ�
    if (w_mpu_write_byte(_MPU_CONFIG_REG, dlpf) == mpu_ok)
        return mpu_ok; // ���óɹ�
    else
        return mpu_err; // ����ʧ��
}

/*  ���������������̷�Χ  */
mpu_state_t w_mpu_set_gyro_fsr(mpu_fsr_t gyro_fsr)
{
    // 0x1B
    if (w_mpu_write_byte(_MPU_GYRO_CONFIG_REG, gyro_fsr << 3) == mpu_ok)
        return mpu_ok; // ���óɹ�
    else
        return mpu_err; // ����ʧ��
}

/*  ���ü��ٶȼ������̷�Χ  */
mpu_state_t w_mpu_set_acce_fsr(mpu_fsr_t acce_fsr)
{
    // 0x1C
    if (w_mpu_write_byte(_MPU_ACCE_CONFIG_REG, acce_fsr << 3) == mpu_ok)
        return mpu_ok; // ���óɹ�
    else
        return mpu_err; // ����ʧ��
}

/*  mpuд��һ��Byte  */
mpu_state_t w_mpu_write_byte(uint8_t reg_addr, uint8_t w_data)
{
    uint8_t w_data_byte = w_data; // ����Ҫд�������

    if (HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, _MPU_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, &w_data_byte, 1, 0xfff) == HAL_OK)
        return mpu_ok; // д��ɹ�
    else
        return mpu_err; // д��ʧ��}
}
/*  mpu��ȡһ��Byte  */
uint8_t w_mpu_read_byte(uint8_t reg_addr)
{
    static uint8_t r_data_byte = 0x00u; // ��Ŷ�ȡһ��Byte������
    if (HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, _MPU_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, &r_data_byte, 1, 0xfff) == HAL_OK)
        //	HAL_Delay(50);
        return r_data_byte; // ���ض�ȡ������
    else
        return mpu_err;
}

/*  mpu����д����Byte  */
mpu_state_t w_mpu_write_byte_len(uint8_t reg_addr, uint8_t *w_data_buf, uint8_t len)
{

    if (HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, _MPU_WRITE, reg_addr, I2C_MEMADD_SIZE_8BIT, w_data_buf, len, 50) == HAL_OK)
        return mpu_ok; // д��ɹ�
    else
        return mpu_err; // д��ʧ��}
}
/*  mpu������ȡ���Byte  */
mpu_state_t w_mpu_read_byte_len(uint8_t reg_addr, uint8_t *r_data_buf, uint8_t len)
{

    if (HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, _MPU_READ, reg_addr, I2C_MEMADD_SIZE_8BIT, r_data_buf, len, 50) == HAL_OK)
        //		HAL_Delay(50);
        return mpu_ok; // ��ȡ���
    else 
        return mpu_err;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* DMP (inv_mpu.c inv_mpu_dmp_motion_driver.c) ��Ҫ�õ� CODE BEGIN */

void my_hal_delay(unsigned long ms)
{
    HAL_Delay(ms);
}

/**
 * @brief  get_tick_ms_cnt()  			ms����ʱ���, ΪDMP��get_ms
 */
void f1_get_tick_ms_cnt(unsigned long *cnt)
{
    *cnt = HAL_GetTick();
}

/**
 * @brief  get_tick_ms_cnt1()  			ms����ʱ���,����Ϊ��, ʹ���в�������, ΪDMP��get_ms������
 */
void f1_get_tick_ms_cnt1(unsigned long *cnt)
{
    ;
}

uint8_t dmp_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char len, unsigned char *w_data)
{
    // ��ʱʱ�俴ʵ�ʸ���, ���ⲻ��
    return HAL_I2C_Mem_Write(&MPU_I2C_HANDLE, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, w_data, len, 5);
}

uint8_t dmp_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char len, unsigned char *r_data)
{
    // ��ʱʱ�俴ʵ�ʸ���, ���ⲻ��
    return HAL_I2C_Mem_Read(&MPU_I2C_HANDLE, slave_addr, reg_addr, I2C_MEMADD_SIZE_8BIT, r_data, len, 5);
}

/* DMP (inv_mpu.c inv_mpu_dmp_motion_driver.c) ��Ҫ�õ� CODE END  */

/* ����DMP��Ӧ�� CODE BEGIN */

#define DEFAULT_MPU_HZ (100) // ������ٶ�, 100Hz, 10ms
// #define DEFAULT_MPU_HZ  (200)			//������ٶ�, 200Hz, 20ms

// q30��ʽ,longתfloatʱ�ĳ���.
#define q30 1073741824.0f

uint8_t buffer[14];
int16_t MPU6050_FIFO[6][11];

int16_t Gx_offset = 0, Gy_offset = 0, Gz_offset = 0;

// �����Ƿ�������, ����λ��
static signed char gyro_orientation[9] = {1, 0, 0,
                                          0, 1, 0,
                                          0, 0, 1};

// ����ת��
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

// �����Ƿ������
static unsigned short inv_orientation_matrix_to_scalar(
    const signed char *mtx)
{
    unsigned short scalar;
    /*
            //��Ⱦ���
            {1, 0, 0,
             0, 1, 0,
             0, 0, 1};

     */
    scalar = inv_row_2_scale(mtx);
    scalar |= inv_row_2_scale(mtx + 3) << 3;
    scalar |= inv_row_2_scale(mtx + 6) << 6;

    return scalar;
}

// MPU6050�������Բ�
static void run_self_test(void)
{
    int result;
    long gyro[3], accel[3];

    result = mpu_run_self_test(gyro, accel);
    if (result == 0x03) {

        // ����ͨ�������ǿ������������ǵ����ݣ�������������DMP
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
// DMP��ʼ��
void dmp_init(void)
{
    uint8_t temp[1] = {0};
    dmp_i2c_read(HW_S_HW_MPU_ADDR, _MPU_DEVICE_ID, I2C_MEMADD_SIZE_8BIT, temp);
    //	 temp[0] = w_mpu_read_device_address();
    // userPrint("mpu_set_sensor complete.\r\n");
    HAL_Delay(1);
    userPrint("Device_ID:0x%x\r\n", temp[0]);
    HAL_Delay(1);

    // ���ID, ����ϵͳ��λ
    if (temp[0] != 0x68)
        NVIC_SystemReset();

    if (!mpu_init()) {
        // �����ڲ��Ĵ�����Ĭ��,
        if (!mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL))
            userPrint(".");
        else
            userPrint("\r\nmpu_set_sensor failed ......\r\n");
        // ����FIFO
        if (!mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL))
            userPrint(".");
        else
            userPrint("\r\nmpu_configure_fifo failed ......\r\n");
        // ���ò���Ƶ�ʷ�Ƶ�Ĵ���
        if (!mpu_set_sample_rate(DEFAULT_MPU_HZ))
            userPrint(".");
        else
            userPrint("\r\nmpu_set_sample_rate failed ......\r\n");

        // DMP���ض�����������̼����
        if (!dmp_load_motion_driver_firmware())
            userPrint(".");
        else
            userPrint("\r\ndmp_load_motion_driver_firmware ......\r\n");

        // DMP���������Ƿ������
        if (!dmp_set_orientation(inv_orientation_matrix_to_scalar(gyro_orientation)))
            userPrint(".");
        else
            userPrint("\r\ndmp_set_orientation failed ......\r\n");

        // DMPʹ��������
        if (!dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_TAP |
                                DMP_FEATURE_ANDROID_ORIENT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_CAL_GYRO |
                                DMP_FEATURE_GYRO_CAL))
            userPrint(".");
        else
            userPrint("\r\ndmp_enable_feature failed ......\r\n");

        // DMP�趨FIFO�������
        if (!dmp_set_fifo_rate(DEFAULT_MPU_HZ))
            userPrint(".");
        else
            userPrint("\r\ndmp_set_fifo_rate failed ......\r\n");

        // �Բ���������,����ͨ��. ���ǿ������������ǵ����ݣ�������������DMP
        run_self_test();

        // mpu����DMP״̬���
        if (!mpu_set_dmp_state(1))
            userPrint(".\r\n");
        else
            userPrint("\r\nmpu_set_dmp_state failed ......\r\n");
        my_hal_delay(20);

        // �ȶ�ȡ��̬��һ��ʱ��, ���ȶ�
        for (uint16_t rd = 0; rd < 5; rd++) // 6s
        {
            // ��ȡMPU����DMP����̬
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
    long quat[4]; // ��Ԫ��
    int16_t gyro[3], accel[3], sensors;
    unsigned long sensor_timestamp; // ������ʱ���

    float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

    // ��ȡFIFO�е�����
    if (dmp_read_fifo(gyro, accel, quat, &sensor_timestamp, &sensors, &more))
        return 1;

    if (sensors & INV_WXYZ_QUAT) {
        q0 = quat[0] / q30;
        q1 = quat[1] / q30;
        q2 = quat[2] / q30;
        q3 = quat[3] / q30;

        pose_msg->pitch = asin(-2 * q1 * q3 + 2 * q0 * q2) * 57.3f;                                      // ��Ԫ������, ������(Pitch)-->����X����ת
        pose_msg->roll  = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2 * q2 + 1) * 57.3f;      // ��Ԫ������, ������(Roll)-->����Y����ת
        pose_msg->yaw   = atan2(2 * (q1 * q2 + q0 * q3), q0 * q0 + q1 * q1 - q2 * q2 - q3 * q3) * 57.3f; // ��Ԫ������, ƫ����(Yaw)-->����Z����ת

        mpu_accel_msg->x = (4 * 9.8 * accel[0]) / 32768.0f;
        mpu_accel_msg->y = (4 * 9.8 * accel[1]) / 32768.0f;
        mpu_accel_msg->z = (4 * 9.8 * accel[2]) / 32768.0f;
        return 0;
    } else
        return 2;
}
