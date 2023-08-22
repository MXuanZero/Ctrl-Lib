#ifndef __MATH_BASE_H__
#define __MATH_BASE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define DEF_E 2.7182818284590452354f
#define DEF_LOG2E 1.4426950408889634074f
#define DEF_LOG10E 0.43429448190325182765f
#define DEF_LN2 0.69314718055994530942f
#define DEF_LN10 2.30258509299404568402f
#define DEF_PI 3.14159265358979323846f
#define DEF_PI_2 1.57079632679489661923f
#define DEF_PI_4 0.78539816339744830962f
#define DEF_1_PI 0.31830988618379067154f
#define DEF_2_PI 0.63661977236758134308f
#define DEF_2_SQRTPI 1.12837916709551257390f
#define DEF_SQRT2 1.41421356237309504880f
#define DEF_SQRT1_2 0.70710678118654752440f
#define DEF_SQRT_3 1.732050807568877f

#define DEF_MIN(a, b) ((a) < (b) ? (a) : (b))
#define DEF_MAX(a, b) ((a) > (b) ? (a) : (b))
#define DEF_ABS(x) ((x) > 0 ? (x) : -(x))
#define DEF_CONSTRAIN(amt, min, max) \
	((amt) < (min) ? (min) : ((amt) > (max) ? (max) : (amt)))

float m_rsqrtf(float number);
float m_sqrtf(float number);

#ifdef __cplusplus
}
#endif

#endif //__MATH_BASE_H__
