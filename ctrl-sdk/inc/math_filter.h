#ifndef MATH_FILTER_H
#define MATH_FILTER_H
/* Includes ------------------------------------------------------------------*/
#include "math_base.h"
/* Define --------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef struct math_lpf_1st {
	float alpha;
	float prev;
} math_lpf_1st_t;
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
 * 低通滤波初始化
 */
void math_lpf_1st_init(math_lpf_1st_t *f, float sample_freq, float cutoff_freq);

/**
 * 低通滤波器使用
 */
float math_lpf_1st_apply(math_lpf_1st_t *f, float input);

#endif // MATH_FILTER_H
