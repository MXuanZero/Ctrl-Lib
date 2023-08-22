#include <stdint.h>
#include "lib_time.h"

static uint32_t lib_tick = 0;

uint32_t lib_get_tick(void)
{
    return lib_tick;
}

void lib_tick_update(uint32_t tick)
{
    lib_tick += tick;
}
