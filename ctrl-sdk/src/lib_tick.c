/* Includes ------------------------------------------------------------------*/
#include "lib_tick.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static time_ms_t lib_sys_ms = 0; // ms
static time_us_t lib_sys_us = 0; // us
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
__attribute__((weak)) void lib_update_clock(time_us_t us)
{
	volatile static time_us_t t_us = 0;
	time_ms_t t_ms;

	lib_sys_us += us;
	t_us += us;

	t_ms = t_us / 1000;
	if (t_ms > 0) {
		lib_sys_ms += t_ms;
		t_us %= 1000;
	}
}

time_ms_t lib_get_ms(void)
{
	return lib_sys_ms;
}

time_us_t lib_get_us(void)
{
	return lib_sys_us;
}
