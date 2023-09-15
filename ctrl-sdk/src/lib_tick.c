/* Includes ------------------------------------------------------------------*/
#include "lib_tick.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static time_ms_t lib_millis = 0; // ms
static time_us_t lib_micros = 0; // us
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void lib_update_clock(time_us_t us)
{
	volatile static time_us_t t_us = 0;
	time_ms_t t_ms;

	lib_micros += us;
	t_us += us;

	t_ms = t_us / 1000;
	if (t_ms > 0) {
		lib_millis += t_ms;
		t_us %= 1000;
	}
}

time_ms_t lib_get_millis(void)
{
	return lib_millis;
}

time_us_t lib_get_micros(void)
{
	return lib_micros;
}
