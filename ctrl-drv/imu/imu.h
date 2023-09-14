#ifndef IMU_H
#define IMU_H
/* Includes ------------------------------------------------------------------*/
#include <stdint-gcc.h>
#include <stdbool.h>
#include "biquad_filter.h"
/* Define --------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef enum {
	IMU_ALGORITHM_NO_FILTER,
	IMU_ALGORITHM_MAHONY,
	IMU_ALGORITHM_MADGWICK
} IMU_ALGORITHM;

typedef struct {
	float q0, q1, q2, q3;
	float sample_p, beta;
	float two_kp, two_ki;
	float integralFBx, integralFBy, integralFBz;
} imu_quart_t;

typedef void (*imu_pos_update)(float *, float *, float *);
typedef void (*imu_quart_update)(imu_quart_t *, float, float, float, float,
				 float, float, float, float, float);

typedef struct {
	float roll;
	float pitch;
	float yaw;
	float ax;
	float ay;
	float az;
	float gx;
	float gy;
	float gz;
	float mx;
	float my;
	float mz;
} imu_data_t;

typedef struct {
	imu_pos_update pos_update_fn;
	imu_quart_update quart_update_fn;
	BiquadFilter_t acc_filter[3];
	BiquadFilter_t gyro_filter[3];
	imu_quart_t quart;
	imu_data_t data;
	bool enable_filter;
} imu_t;
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void imu_init(imu_t *imu, float sample_freq, IMU_ALGORITHM mode,
	      imu_pos_update pos_update_fn);
void imu_filter_init(imu_t *imu, float f, float gyro_cutoff, float acc_cutoff);
void imu_update(imu_t *imu);
#endif // IMU_H
