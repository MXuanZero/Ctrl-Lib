#ifndef HAL_CFG_H
#define HAL_CFG_H

#define DEF_HAL_MALLOC(x) malloc(x)
#define DEF_HAL_FREE(x) free(x)
#define DEF_HAL_INIT_EMPTY 1

/* store */
#define HAL_STORE_ENABLE 1 			// 是否启用存储
#define HAL_STORE_PARAM_NUM_MAX UINT16_MAX 	// 最大存储参数个数
#define HAL_STORE_MAP_LIST_SIZE 13
#define HAL_STORE_LOCK_ENABLE 1

#endif // HAL_CFG_H
