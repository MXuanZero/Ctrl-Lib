#include "main_task.h"
#include "driver_mpu9250_basic.h"
#include "usb_utils.h"
#include "imu.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
imu_t imu = { 0 };
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void mpu9250_update(float gyro[3], float acc[3], float mag[3])
{
	mpu9250_basic_read(acc, gyro, mag);
}

void imu_test_main(void)
{
	mpu9250_basic_init(MPU9250_INTERFACE_IIC, MPU9250_ADDRESS_AD0_LOW);
	imu_init(&imu, 200.0f, IMU_ALGORITHM_MAHONY, mpu9250_update);
	imu_filter_init(&imu, 200, 50, 50);

	for (;;) {
		imu_update(&imu);
		HAL_Delay(5);
	}
}
