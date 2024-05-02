#ifndef HAL_CFG_H
#define HAL_CFG_H

#define HAL_MALLOC(x) malloc(x)
#define HAL_FREE(x) free(x)
#define HAL_INIT_EMPTY 1

/* store */
#define HAL_STORE_ENABLE 1 // 是否启用存储
#define HAL_STORE_PARAM_NUM_MAX UINT16_MAX // 最大存储参数个数
#define HAL_STORE_MAP_LIST_SIZE 13
#define HAL_STORE_LOCK_ENABLE 1

/* ccp: ctrl-sdk communication protocol*/
#define HAL_CCP_ERROR_CNT_MAX 3
#define HAL_CCP_WAIT_BACK_LIST_SIZE 13
#define HAL_CCP_TX_DATA_MAX UINT16_MAX
#define HAL_CCP_FRAME_HEAD 0x55 // 包头
#define HAL_CCP_FRAME_TAIL 0x22 // 包尾

typedef enum hal_ccp_order {
	HAL_CCP_ORDER_GET_PARAM = 0x01, // 获取参数系统的参数
	HAL_CCP_ORDER_SET_PARAM = 0x02, // 设置参数系统的参数
	HAL_CCP_ORDER_CUSTOMIZE = 0x7F, // 不要超过0x7F
} Hal_Ccp_Order;

/* ccp recv */
#define HAL_CCP_RECV_DATA_SIZE 64 // 最大接收长度
#define HAL_CCP_RECV_BUF_NUM 4 // 接收缓存队列
#define HAL_CCP_RECV_LOCK_ENABLE 0 // 是否启用互斥锁，1为启用

#endif // HAL_CFG_H
