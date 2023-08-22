#include "math_base.h"

// 快速开平方的倒数
// https://reprap.org/forum/read.php?147,219210
// https://en.wikipedia.org/wiki/Fast_inverse_square_root
float m_rsqrtf(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long *)&y; // evil floating point bit level hacking
	i = 0x5f375a86 - (i >> 1); // what the fuck?
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y)); // 1st iteration
	// y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration,
	//this can be removed

	return y;
}

float m_sqrtf(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y = number;
	i = *(long *)&y; // evil floating point bit level hacking
	i = 0x5f375a86 - (i >> 1); // what the fuck?
	y = *(float *)&i;
	y = y * (threehalfs - (x2 * y * y)); // 1st iteration
	// y  = y * ( threehalfs - ( x2 * y * y ) );   // 2nd iteration,
	//this can be removed

	return y * number;
}
