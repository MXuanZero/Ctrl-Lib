#ifndef LIB_TICK_H
#define LIB_TICK_H
/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Define --------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void lib_update_clock(uint32_t ms);
uint32_t lib_get_millis(void);
uint32_t lib_get_tick(void);
#endif // LIB_TICK_H
