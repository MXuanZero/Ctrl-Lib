/* ccp: ctrl-sdk communication protocol */
/* 目前ccp模块暂不支持操作系统 */
#ifndef HAL_CCP_H
#define HAL_CCP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "ctrl_defs.h"
#include "hal_cfg.h"
#include "lib_list.h"
/* Define --------------------------------------------------------------------*/
#define HAL_CCP_INFO_SIZE sizeof(Hal_Ccp_Info_t)
#define HAL_CCP_COCHE_NODE_SIZE sizeof(hal_ccp_cache_node_t)
#define HAL_CCP_COVER_HEAD_SIZE sizeof(hal_ccp_cover_head_t)
#define HAL_CCP_COVER_Tail_SIZE sizeof(hal_ccp_cover_tail_t)
#define HAL_CCP_GET_PACK_HEAD(p) (char *)(p) /* p:pack, l:len */
#define HAL_CCP_GET_PACK_DATA(p) \
	(HAL_CCP_GET_PACK_HEAD(p) + HAL_CCP_COVER_HEAD_SIZE)
#define HAL_CCP_GET_PACK_TAIL(p, l) \
	(HAL_CCP_GET_PACK_HEAD(p) + HAL_CCP_COVER_HEAD_SIZE + l)
#define HAL_CCP_GET_PACK_SIZE(l) \
	(HAL_CCP_COVER_HEAD_SIZE + HAL_CCP_COVER_Tail_SIZE + l)
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum hal_ccp_state {
	HAL_CCP_TRUE = 0,
	HAL_CCP_FALSE,
	HAL_CCP_CREATE_ERROR,
	HAL_CCP_TOO_LONG,
	HAL_CCP_NOT_FIND,
	HAL_CCP_SEND_ERROR,
	HAL_CCP_CHECK_ERROR,
} hal_ccp_status;

typedef struct hal_ccp_cover_head {
	uint8_t start;
	uint16_t cmd_id;
	uint8_t sender;
	uint8_t receiver;
	uint8_t order;
	uint16_t len;
	uint8_t reserve1;
	uint8_t reserve2;
} __attribute__((packed, aligned(1))) hal_ccp_cover_head_t;

typedef struct hal_ccp_cover_tail_t {
	uint8_t check;
	uint8_t end;
} __attribute__((packed)) hal_ccp_cover_tail_t;

typedef struct hal_ccp_info_t {
	uint8_t *pack; // 数据
	uint32_t time; // 时间
	uint32_t timeout; // 超时时间
	uint32_t delay; // 延时时间
	uint16_t cmd_id; //
	uint16_t pack_size; // 数据长度
	uint8_t error_cnt; // 错误次数
	// _Bool    finish;   // 传输完成标志位
} hal_ccp_info_t;

typedef struct hal_ccp_cache_node_t {
	hal_ccp_info_t info;
	lib_node_t node;
} hal_ccp_cache_node_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief 初始化时间
 * @param local_id 本机地址
 */
void hal_ccp_init(uint8_t local_id);

/**
 * @brief 发送函数
 * @param receiver 接收者
 * @param order 命令
 * @param tx_data 发送数据
 * @param len 发送数据
 * @param delay 延时
 * @param timeout 超时时间
 */
hal_ccp_status hal_ccp_transmit(uint8_t receiver, uint8_t order,
				uint8_t *tx_data, uint16_t len, uint32_t delay,
				uint32_t timeout);
/**
 * @brief 应答
 */
hal_ccp_status hal_ccp_answer(uint8_t receiver, uint8_t order, uint16_t cmd_id,
			      uint8_t *answer_data, uint16_t len);

/**
 * @brief 发送处理
 */
hal_ccp_status hal_ccp_transmit_handler(uint32_t tick);

/**
 * @brief 接收处理
 */
hal_ccp_status hal_ccp_receive_handler(uint8_t *pack, uint16_t len);

/**
 * @brief 如果cmd_id不为0，则需清理缓存节点
 * @return
 */
hal_ccp_status hal_ccp_clear_cache(uint8_t *pack, uint16_t len);

/**
 * @brief 返回现存的节点
 * @return
 */
size_t hal_ccp_get_existing_node(void);

/**
 * @brief 初始化函数，需实现
 */
hal_ccp_status hal_ccp_prot_init(void);

/**
 * @brief 通讯发送函数，需实现
 * @param data
 */
hal_ccp_status hal_ccp_prot_send(uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // HAL_CCP_H
