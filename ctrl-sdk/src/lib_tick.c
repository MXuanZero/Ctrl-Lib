/* Includes ------------------------------------------------------------------*/
#include "lib_tick.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint32_t lib_millis = 0;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void lib_update_clock(uint32_t ms)
{
	lib_millis += ms;
}

uint32_t lib_get_millis(void)
{
	return lib_millis;
}

uint32_t lib_get_tick(void)
{
	return 0;
}

