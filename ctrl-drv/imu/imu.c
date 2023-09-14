/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <math.h>
#include "imu.h"
#include "math_base.h"
#include "usb_utils.h"
/* Private define ------------------------------------------------------------*/
#define DEF_DEG_TO_RAD 0.017453292519943295769236907684886f
#define RAD_TO_DEG 57.295779513082320876798154814105f
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void MadgwickAHRSupdateIMU(imu_quart_t *quart, float gx, float gy,
				  float gz, float ax, float ay, float az)
{
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2, _8q1, _8q2, q0q0, q1q1,
		q2q2, q3q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-quart->q1 * gx - quart->q2 * gy - quart->q3 * gz);
	qDot2 = 0.5f * (quart->q0 * gx + quart->q2 * gz - quart->q3 * gy);
	qDot3 = 0.5f * (quart->q0 * gy - quart->q1 * gz + quart->q3 * gx);
	qDot4 = 0.5f * (quart->q0 * gz + quart->q1 * gy - quart->q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN
	// in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
		// Normalise accelerometer measurement
		recipNorm = m_rsqrtf(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * quart->q0;
		_2q1 = 2.0f * quart->q1;
		_2q2 = 2.0f * quart->q2;
		_2q3 = 2.0f * quart->q3;
		_4q0 = 4.0f * quart->q0;
		_4q1 = 4.0f * quart->q1;
		_4q2 = 4.0f * quart->q2;
		_8q1 = 8.0f * quart->q1;
		_8q2 = 8.0f * quart->q2;
		q0q0 = quart->q0 * quart->q0;
		q1q1 = quart->q1 * quart->q1;
		q2q2 = quart->q2 * quart->q2;
		q3q3 = quart->q3 * quart->q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * quart->q1 -
		     _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * quart->q2 + _2q0 * ax + _4q2 * q3q3 -
		     _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * quart->q3 - _2q1 * ax +
		     4.0f * q2q2 * quart->q3 - _2q2 * ay;
		recipNorm = m_rsqrtf(s0 * s0 + s1 * s1 + s2 * s2 +
				     s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= quart->beta * s0;
		qDot2 -= quart->beta * s1;
		qDot3 -= quart->beta * s2;
		qDot4 -= quart->beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	quart->q0 += qDot1 * quart->sample_p;
	quart->q1 += qDot2 * quart->sample_p;
	quart->q2 += qDot3 * quart->sample_p;
	quart->q3 += qDot4 * quart->sample_p;

	// Normalise quaternion
	recipNorm = m_rsqrtf(quart->q0 * quart->q0 + quart->q1 * quart->q1 +
			     quart->q2 * quart->q2 + quart->q3 * quart->q3);
	quart->q0 *= recipNorm;
	quart->q1 *= recipNorm;
	quart->q2 *= recipNorm;
	quart->q3 *= recipNorm;
}

static void MadgwickAHRSupdate(imu_quart_t *quart, float gx, float gy, float gz,
			       float ax, float ay, float az, float mx, float my,
			       float mz)
{
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2bx, _2bz, _4bx, _4bz, _2q0,
		_2q1, _2q2, _2q3, _2q0q2, _2q2q3, q0q0, q0q1, q0q2, q0q3, q1q1,
		q1q2, q1q3, q2q2, q2q3, q3q3;

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in
	// magnetometer normalisation)
	if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MadgwickAHRSupdateIMU(quart, gx, gy, gz, ax, ay, az);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-quart->q1 * gx - quart->q2 * gy - quart->q3 * gz);
	qDot2 = 0.5f * (quart->q0 * gx + quart->q2 * gz - quart->q3 * gy);
	qDot3 = 0.5f * (quart->q0 * gy - quart->q1 * gz + quart->q3 * gx);
	qDot4 = 0.5f * (quart->q0 * gz + quart->q1 * gy - quart->q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN
	// in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
		// Normalise accelerometer measurement
		recipNorm = m_rsqrtf(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Normalise magnetometer measurement
		recipNorm = m_rsqrtf(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0mx = 2.0f * quart->q0 * mx;
		_2q0my = 2.0f * quart->q0 * my;
		_2q0mz = 2.0f * quart->q0 * mz;
		_2q1mx = 2.0f * quart->q1 * mx;
		_2q0 = 2.0f * quart->q0;
		_2q1 = 2.0f * quart->q1;
		_2q2 = 2.0f * quart->q2;
		_2q3 = 2.0f * quart->q3;
		_2q0q2 = 2.0f * quart->q0 * quart->q2;
		_2q2q3 = 2.0f * quart->q2 * quart->q3;
		q0q0 = quart->q0 * quart->q0;
		q0q1 = quart->q0 * quart->q1;
		q0q2 = quart->q0 * quart->q2;
		q0q3 = quart->q0 * quart->q3;
		q1q1 = quart->q1 * quart->q1;
		q1q2 = quart->q1 * quart->q2;
		q1q3 = quart->q1 * quart->q3;
		q2q2 = quart->q2 * quart->q2;
		q2q3 = quart->q2 * quart->q3;
		q3q3 = quart->q3 * quart->q3;

		// Reference direction of Earth's magnetic field
		hx = mx * q0q0 - _2q0my * quart->q3 + _2q0mz * quart->q2 +
		     mx * q1q1 + _2q1 * my * quart->q2 + _2q1 * mz * quart->q3 -
		     mx * q2q2 - mx * q3q3;
		hy = _2q0mx * quart->q3 + my * q0q0 - _2q0mz * quart->q1 +
		     _2q1mx * quart->q2 - my * q1q1 + my * q2q2 +
		     _2q2 * mz * quart->q3 - my * q3q3;
		_2bx = m_sqrtf(hx * hx + hy * hy);
		_2bz = -_2q0mx * quart->q2 + _2q0my * quart->q1 + mz * q0q0 +
		       _2q1mx * quart->q3 - mz * q1q1 + _2q2 * my * quart->q3 -
		       mz * q2q2 + mz * q3q3;
		_4bx = 2.0f * _2bx;
		_4bz = 2.0f * _2bz;

		// Gradient decent algorithm corrective step
		s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) +
		     _2q1 * (2.0f * q0q1 + _2q2q3 - ay) -
		     _2bz * quart->q2 *
			     (_2bx * (0.5f - q2q2 - q3q3) +
			      _2bz * (q1q3 - q0q2) - mx) +
		     (-_2bx * quart->q3 + _2bz * quart->q1) *
			     (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) -
			      my) +
		     _2bx * quart->q2 *
			     (_2bx * (q0q2 + q1q3) +
			      _2bz * (0.5f - q1q1 - q2q2) - mz);
		s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) +
		     _2q0 * (2.0f * q0q1 + _2q2q3 - ay) -
		     4.0f * quart->q1 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) +
		     _2bz * quart->q3 *
			     (_2bx * (0.5f - q2q2 - q3q3) +
			      _2bz * (q1q3 - q0q2) - mx) +
		     (_2bx * quart->q2 + _2bz * quart->q0) *
			     (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) -
			      my) +
		     (_2bx * quart->q3 - _4bz * quart->q1) *
			     (_2bx * (q0q2 + q1q3) +
			      _2bz * (0.5f - q1q1 - q2q2) - mz);
		s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) +
		     _2q3 * (2.0f * q0q1 + _2q2q3 - ay) -
		     4.0f * quart->q2 * (1 - 2.0f * q1q1 - 2.0f * q2q2 - az) +
		     (-_4bx * quart->q2 - _2bz * quart->q0) *
			     (_2bx * (0.5f - q2q2 - q3q3) +
			      _2bz * (q1q3 - q0q2) - mx) +
		     (_2bx * quart->q1 + _2bz * quart->q3) *
			     (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) -
			      my) +
		     (_2bx * quart->q0 - _4bz * quart->q2) *
			     (_2bx * (q0q2 + q1q3) +
			      _2bz * (0.5f - q1q1 - q2q2) - mz);
		s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) +
		     _2q2 * (2.0f * q0q1 + _2q2q3 - ay) +
		     (-_4bx * quart->q3 + _2bz * quart->q1) *
			     (_2bx * (0.5f - q2q2 - q3q3) +
			      _2bz * (q1q3 - q0q2) - mx) +
		     (-_2bx * quart->q0 + _2bz * quart->q2) *
			     (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) -
			      my) +
		     _2bx * quart->q1 *
			     (_2bx * (q0q2 + q1q3) +
			      _2bz * (0.5f - q1q1 - q2q2) - mz);
		recipNorm = m_rsqrtf(s0 * s0 + s1 * s1 + s2 * s2 +
				     s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= quart->beta * s0;
		qDot2 -= quart->beta * s1;
		qDot3 -= quart->beta * s2;
		qDot4 -= quart->beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
	quart->q0 += qDot1 * quart->sample_p;
	quart->q1 += qDot2 * quart->sample_p;
	quart->q2 += qDot3 * quart->sample_p;
	quart->q3 += qDot4 * quart->sample_p;

	// Normalise quaternion
	recipNorm = m_rsqrtf(quart->q0 * quart->q0 + quart->q1 * quart->q1 +
			     quart->q2 * quart->q2 + quart->q3 * quart->q3);
	quart->q0 *= recipNorm;
	quart->q1 *= recipNorm;
	quart->q2 *= recipNorm;
	quart->q3 *= recipNorm;
}

//---------------------------------------------------------------------------------------------------
// IMU algorithm update

static void MahonyAHRSupdateIMU(imu_quart_t *quart, float gx, float gy,
				float gz, float ax, float ay, float az)
{
	float recipNorm;
	float halfvx, halfvy, halfvz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Compute feedback only if accelerometer measurement valid (avoids NaN
	// in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
		// Normalise accelerometer measurement
		recipNorm = m_rsqrtf(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Estimated direction of gravity and vector perpendicular to
		// magnetic flux
		halfvx = quart->q1 * quart->q3 - quart->q0 * quart->q2;
		halfvy = quart->q0 * quart->q1 + quart->q2 * quart->q3;
		halfvz = quart->q0 * quart->q0 - 0.5f + quart->q3 * quart->q3;

		// Error is sum of cross product between estimated and measured
		// direction of gravity
		halfex = (ay * halfvz - az * halfvy);
		halfey = (az * halfvx - ax * halfvz);
		halfez = (ax * halfvy - ay * halfvx);

		// Compute and apply integral feedback if enabled
		if (quart->two_ki > 0.0f) {
			quart->integralFBx += quart->two_ki * halfex *
					      quart->sample_p; // integral error
							       // scaled by Ki
			quart->integralFBy +=
				quart->two_ki * halfey * quart->sample_p;
			quart->integralFBz +=
				quart->two_ki * halfez * quart->sample_p;
			gx += quart->integralFBx; // apply integral feedback
			gy += quart->integralFBy;
			gz += quart->integralFBz;
		} else {
			quart->integralFBx = 0.0f; // prevent integral windup
			quart->integralFBy = 0.0f;
			quart->integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += quart->two_kp * halfex;
		gy += quart->two_kp * halfey;
		gz += quart->two_kp * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * quart->sample_p); // pre-multiply common factors
	gy *= (0.5f * quart->sample_p);
	gz *= (0.5f * quart->sample_p);
	qa = quart->q0;
	qb = quart->q1;
	qc = quart->q2;
	quart->q0 += (-qb * gx - qc * gy - quart->q3 * gz);
	quart->q1 += (qa * gx + qc * gz - quart->q3 * gy);
	quart->q2 += (qa * gy - qb * gz + quart->q3 * gx);
	quart->q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	recipNorm = m_rsqrtf(quart->q0 * quart->q0 + quart->q1 * quart->q1 +
			     quart->q2 * quart->q2 + quart->q3 * quart->q3);
	quart->q0 *= recipNorm;
	quart->q1 *= recipNorm;
	quart->q2 *= recipNorm;
	quart->q3 *= recipNorm;
}

static void MahonyAHRSupdate(imu_quart_t *quart, float gx, float gy, float gz,
			     float ax, float ay, float az, float mx, float my,
			     float mz)
{
	float recipNorm;
	float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
	float hx, hy, bx, bz;
	float halfvx, halfvy, halfvz, halfwx, halfwy, halfwz;
	float halfex, halfey, halfez;
	float qa, qb, qc;

	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in
	// magnetometer normalisation)
	if ((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MahonyAHRSupdateIMU(quart, gx, gy, gz, ax, ay, az);
		return;
	}

	// Compute feedback only if accelerometer measurement valid (avoids NaN
	// in accelerometer normalisation)
	if (!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
		// Normalise accelerometer measurement
		recipNorm = m_rsqrtf(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Normalise magnetometer measurement
		recipNorm = m_rsqrtf(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		q0q0 = quart->q0 * quart->q0;
		q0q1 = quart->q0 * quart->q1;
		q0q2 = quart->q0 * quart->q2;
		q0q3 = quart->q0 * quart->q3;
		q1q1 = quart->q1 * quart->q1;
		q1q2 = quart->q1 * quart->q2;
		q1q3 = quart->q1 * quart->q3;
		q2q2 = quart->q2 * quart->q2;
		q2q3 = quart->q2 * quart->q3;
		q3q3 = quart->q3 * quart->q3;

		// Reference direction of Earth's magnetic field
		hx = 2.0f * (mx * (0.5f - q2q2 - q3q3) + my * (q1q2 - q0q3) +
			     mz * (q1q3 + q0q2));
		hy = 2.0f * (mx * (q1q2 + q0q3) + my * (0.5f - q1q1 - q3q3) +
			     mz * (q2q3 - q0q1));
		bx = m_sqrtf(hx * hx + hy * hy);
		bz = 2.0f * (mx * (q1q3 - q0q2) + my * (q2q3 + q0q1) +
			     mz * (0.5f - q1q1 - q2q2));

		// Estimated direction of gravity and magnetic field
		halfvx = q1q3 - q0q2;
		halfvy = q0q1 + q2q3;
		halfvz = q0q0 - 0.5f + q3q3;
		halfwx = bx * (0.5f - q2q2 - q3q3) + bz * (q1q3 - q0q2);
		halfwy = bx * (q1q2 - q0q3) + bz * (q0q1 + q2q3);
		halfwz = bx * (q0q2 + q1q3) + bz * (0.5f - q1q1 - q2q2);

		// Error is sum of cross product between estimated direction and
		// measured direction of field vectors
		halfex = (ay * halfvz - az * halfvy) +
			 (my * halfwz - mz * halfwy);
		halfey = (az * halfvx - ax * halfvz) +
			 (mz * halfwx - mx * halfwz);
		halfez = (ax * halfvy - ay * halfvx) +
			 (mx * halfwy - my * halfwx);

		// Compute and apply integral feedback if enabled
		if (quart->two_ki > 0.0f) {
			// integral error scaled by Ki
			quart->integralFBx +=
				quart->two_ki * halfex * quart->sample_p;
			quart->integralFBy +=
				quart->two_ki * halfey * quart->sample_p;
			quart->integralFBz +=
				quart->two_ki * halfez * quart->sample_p;
			gx += quart->integralFBx; // apply integral feedback
			gy += quart->integralFBy;
			gz += quart->integralFBz;
		} else {
			quart->integralFBx = 0.0f; // prevent integral windup
			quart->integralFBy = 0.0f;
			quart->integralFBz = 0.0f;
		}

		// Apply proportional feedback
		gx += quart->two_kp * halfex;
		gy += quart->two_kp * halfey;
		gz += quart->two_kp * halfez;
	}

	// Integrate rate of change of quaternion
	gx *= (0.5f * quart->sample_p); // pre-multiply common factors
	gy *= (0.5f * quart->sample_p);
	gz *= (0.5f * quart->sample_p);
	qa = quart->q0;
	qb = quart->q1;
	qc = quart->q2;
	quart->q0 += (-qb * gx - qc * gy - quart->q3 * gz);
	quart->q1 += (qa * gx + qc * gz - quart->q3 * gy);
	quart->q2 += (qa * gy - qb * gz + quart->q3 * gx);
	quart->q3 += (qa * gz + qb * gy - qc * gx);

	// Normalise quaternion
	recipNorm = m_rsqrtf(quart->q0 * quart->q0 + quart->q1 * quart->q1 +
			     quart->q2 * quart->q2 + quart->q3 * quart->q3);
	quart->q0 *= recipNorm;
	quart->q1 *= recipNorm;
	quart->q2 *= recipNorm;
	quart->q3 *= recipNorm;
}

static void imu_update_rpy(imu_data_t *data, float qx, float qy, float qw, float qz)
{
	// Define output variables from updated quaternion---these are
	// Tait-Bryan angles, commonly used in aircraft orientation. In this
	// coordinate system, the positive z-axis is down toward Earth. Yaw is
	// the angle between Sensor x-axis and Earth magnetic North (or true
	// North if corrected for local declination, looking down on the sensor
	// positive yaw is counterclockwise. Pitch is angle between sensor
	// x-axis and Earth ground plane, toward the Earth is positive, up
	// toward the sky is negative. Roll is angle between sensor y-axis and
	// Earth ground plane, y-axis up is positive roll. These arise from the
	// definition of the homogeneous rotation matrix constructed from
	// quaternions. Tait-Bryan angles as well as Euler angles are
	// non-commutative; that is, the get the correct orientation the
	// rotations must be applied in the correct order which for this
	// configuration is yaw, pitch, and then roll. For more see
	// http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles
	// which has additional links.
	float a12, a22, a31, a32, a33; // rotation matrix coefficients for Euler
				       // angles and gravity components
	a12 = 2.0f * (qx * qy + qw * qz);
	a22 = qw * qw + qx * qx - qy * qy - qz * qz;
	a31 = 2.0f * (qw * qx + qy * qz);
	a32 = 2.0f * (qx * qz - qw * qy);
	a33 = qw * qw - qx * qx - qy * qy + qz * qz;
	data->roll = atan2f(a31, a33);
	data->pitch = -asinf(a32);
	data->yaw = atan2f(a12, a22);
	data->roll *= 180.0f / DEF_PI;
	data->pitch *= 180.0f / DEF_PI;
	data->yaw *= 180.0f / DEF_PI;
	if (data->yaw >= +180.f)
		data->yaw -= 360.f;
	else if (data->yaw < -180.f)
		data->yaw += 360.f;
}

void imu_init(imu_t *imu, float sample_freq, IMU_ALGORITHM mode,
	      imu_pos_update pos_update_fn)
{
	if (imu == NULL) {
		return;
	}

	imu->pos_update_fn = pos_update_fn;

	imu_quart_t *quart = &imu->quart;
	quart->q0 = 1.0f;
	quart->q1 = 0.0f;
	quart->q2 = 0.0f;
	quart->q3 = 0.0f;
	quart->sample_p = 1.0f / sample_freq;

	switch (mode) {
	case IMU_ALGORITHM_NO_FILTER:
		break;
	case IMU_ALGORITHM_MAHONY:
		imu->quart_update_fn = MahonyAHRSupdate;
		imu->quart.two_kp = 2.0f * 20.0f;
		imu->quart.two_ki = 2.0f * 0.00f;
		break;
	case IMU_ALGORITHM_MADGWICK:
		imu->quart_update_fn = MadgwickAHRSupdate;
		imu->quart.beta = 0.1f;
		break;
	}
}

void imu_filter_init(imu_t *imu, float f, float gyro_cutoff, float acc_cutoff)
{
	if (imu == NULL) {
		return;
	}
	for (int i = 0; i < 3; i++) {
		biquadFilterInitLPF(&imu->gyro_filter[i], f, gyro_cutoff);
		biquadFilterInitLPF(&imu->acc_filter[i], f, acc_cutoff);
	}
	imu->enable_filter = true;
}

void imu_update(imu_t *imu)
{
	if (imu == NULL) {
		return;
	}
	imu_data_t *data = &imu->data;
	imu_quart_t *quart = &imu->quart;
	imu->pos_update_fn(&data->ax, &data->gx, &data->mx);

	if (imu->enable_filter) {
		data->ax = biquadFilterApply(&imu->acc_filter[0], data->ax);
		data->ay = biquadFilterApply(&imu->acc_filter[1], data->ay);
		data->az = biquadFilterApply(&imu->acc_filter[2], data->az);
		data->gx = biquadFilterApply(&imu->gyro_filter[0], data->gx);
		data->gy = biquadFilterApply(&imu->gyro_filter[1], data->gy);
		data->gz = biquadFilterApply(&imu->gyro_filter[2], data->gz);
	}

	float an = -data->ax;
	float ae = +data->ay;
	float ad = +data->az;
	float gn = +data->gx * DEF_DEG_TO_RAD;
	float ge = -data->gy * DEF_DEG_TO_RAD;
	float gd = -data->gz * DEF_DEG_TO_RAD;
	float mn = +data->my;
	float me = -data->mx;
	float md = +data->mz;

	imu->quart_update_fn(quart, gn, ge, gd, an, ae, ad, mn, me, md);

	imu_update_rpy(data, quart->q0, quart->q1, quart->q2, quart->q3);
}