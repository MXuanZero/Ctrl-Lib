/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "hal_ccp.h"
#include "lib_list.h"
#include "hal_ccp_protocol.h"

/* Private define ------------------------------------------------------------*/
#define HAL_CCP_MALLOC(x) HAL_MALLOC(x)
#define HAL_CCP_FREE(x) HAL_FREE(x)
#define HAL_CCP_INIT_EMPTY HAL_INIT_EMPTY // 创建链表后是否需要清零

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static lib_list_t hal_ccp_wait_back_list[HAL_CCP_WAIT_BACK_LIST_SIZE] = { 0 };
static lib_list_t hal_ccp_delay_list = { 0 };
static lib_list_t hal_ccp_ready_list = { 0 };
static uint16_t hal_ccp_cmd_id = 33; // 千万不能等于0
static size_t hal_ccp_node_num = 0;
static uint8_t hal_ccp_local_id = 0;

/* Private function prototypes -----------------------------------------------*/
static void hal_ccp_init_list(void);
static inline hal_ccp_info_t *hal_ccp_creat_tail_node(lib_list_t *list);
static inline hal_ccp_info_t *hal_ccp_creat_head_node(lib_list_t *list);
static inline void hal_ccp_delete_node(lib_list_t *list,
				       hal_ccp_cache_node_t **node);

/* Private functions ---------------------------------------------------------*/
static void hal_ccp_init_list(void)
{
	for (uint16_t i = 0; i < HAL_CCP_WAIT_BACK_LIST_SIZE; ++i) {
		lib_list_init(&hal_ccp_wait_back_list[i],
			      HAL_CCP_COCHE_NODE_SIZE);
	}
	lib_list_init(&hal_ccp_delay_list, HAL_CCP_COCHE_NODE_SIZE);
	lib_list_init(&hal_ccp_ready_list, HAL_CCP_COCHE_NODE_SIZE);
}

static inline hal_ccp_info_t *hal_ccp_creat_head_node(lib_list_t *list)
{
	lib_node_t *node;
	void *new;

	new = HAL_CCP_MALLOC(HAL_CCP_COCHE_NODE_SIZE);
	if (new == NULL) {
		return NULL;
	}
#if HAL_CCP_INIT_EMPTY == 1
	memset(new, 0, HAL_CCP_COCHE_NODE_SIZE);
#endif
	node = (lib_node_t *)((char *)new +
			      addr_offset(hal_ccp_cache_node_t, node));
	lib_list_inc_head(list, node);
	++hal_ccp_node_num;
	// return (hal_ccp_info_t *)container_addr(new,
	// hal_ccp_cache_node_t, info);
	return (hal_ccp_info_t *)new;
}

static inline hal_ccp_info_t *hal_ccp_creat_tail_node(lib_list_t *list)
{
	lib_node_t *node;
	void *new;

	new = HAL_CCP_MALLOC(HAL_CCP_COCHE_NODE_SIZE);
	if (new == NULL) {
		return NULL;
	}
#if HAL_CCP_INIT_EMPTY == 1
	memset(new, 0, HAL_CCP_COCHE_NODE_SIZE);
#endif
	node = (lib_node_t *)((char *)new +
			      addr_offset(hal_ccp_cache_node_t, node));
	lib_list_inc_tail(list, node);
	++hal_ccp_node_num;
	return (hal_ccp_info_t *)container_addr(new, hal_ccp_cache_node_t,
						info);
}

static inline void hal_ccp_delete_node(lib_list_t *list,
				       hal_ccp_cache_node_t **node)
{
	if (node == NULL) {
		return;
	}
	if (*node == NULL) {
		return;
	}
	lib_list_delete(list, &(*node)->node);
	HAL_CCP_FREE((*node)->info.pack);
	HAL_CCP_FREE(*node);
	--hal_ccp_node_num;
	*node = NULL;
}

static inline void hal_ccp_change_node(lib_list_t *old, lib_list_t *new,
				       hal_ccp_cache_node_t *node, bool mod)
{
	lib_list_change_node(old, new, &node->node, mod);
}

void hal_ccp_init(uint8_t local_id)
{
	hal_ccp_prot_init();
	hal_ccp_init_list();
	hal_ccp_local_id = local_id;
}

static void *Hal_Ccp_Packet(uint8_t receiver, uint8_t order, uint16_t cmd_id,
			    uint8_t *tx_data, uint16_t len)
{
	size_t pack_size = HAL_CCP_GET_PACK_SIZE(len);
	uint8_t *pack = HAL_CCP_MALLOC(pack_size);
	if (pack == NULL) {
		return NULL;
	}
#if HAL_CCP_INIT_EMPTY == 1
	memset(pack, 0, pack_size);
#endif

	/* Lib_TSP_TaskListHead */
	hal_ccp_cover_head_t *head =
		(hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack);
	head->start = HAL_CCP_FRAME_HEAD;
	head->cmd_id = cmd_id;
	head->sender = hal_ccp_local_id;
	head->receiver = receiver;
	head->order = order;
	head->len = len;

	/* data */
	memcpy((char *)(HAL_CCP_GET_PACK_DATA(pack)), (char *)tx_data, len);

	/* check */
	uint8_t check = 0;
	for (size_t i = 1; i < pack_size - HAL_CCP_COVER_Tail_SIZE; ++i) {
		check += pack[i];
	}
	/* Lib_TSP_TaskListTail */
	hal_ccp_cover_tail_t *tail =
		(hal_ccp_cover_tail_t *)HAL_CCP_GET_PACK_TAIL(pack, len);
	tail->check = check;
	tail->end = HAL_CCP_FRAME_TAIL;
	return pack;
}

hal_ccp_status hal_ccp_transmit(uint8_t receiver, uint8_t order,
				uint8_t *tx_data, uint16_t len, uint32_t delay,
				uint32_t timeout)
{
	if (len >= HAL_CCP_TX_DATA_MAX) {
		return HAL_CCP_TOO_LONG;
	}

	/* lock */
	uint16_t cmd_id = timeout != 0 ? ++hal_ccp_cmd_id : 0;
	hal_ccp_cmd_id = hal_ccp_cmd_id != 0 ? hal_ccp_cmd_id : 1; // cannot 0
	order = ((order << 1) & 0xFE) + 1; // 最后一位为1
	uint8_t *pack = Hal_Ccp_Packet(receiver, order, cmd_id, tx_data, len);
	if (pack == NULL) {
		return HAL_CCP_CREATE_ERROR;
	}

	/* send */
	hal_ccp_info_t *info = NULL;
	if (delay != 0 && timeout != 0) {
		info = hal_ccp_creat_tail_node(&hal_ccp_delay_list);
	} else if (delay != 0) {
		info = hal_ccp_creat_tail_node(&hal_ccp_delay_list);
	} else if (timeout != 0) {
		info = hal_ccp_creat_head_node(&hal_ccp_ready_list);
	} else {
		info = hal_ccp_creat_head_node(&hal_ccp_ready_list);
	}
	if (info == NULL) {
		HAL_CCP_FREE(pack);
		return HAL_CCP_CREATE_ERROR;
	}
	info->pack = pack;
	info->timeout = timeout;
	info->time = 0;
	info->delay = delay;
	info->cmd_id =
		((hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack))->cmd_id;
	info->pack_size = HAL_CCP_GET_PACK_SIZE(len);
	info->error_cnt = 0;

	/* unlock */
	return HAL_CCP_TRUE;
}

hal_ccp_status hal_ccp_answer(uint8_t receiver, uint8_t order, uint16_t cmd_id,
			      uint8_t *answer_data, uint16_t len)
{
	uint8_t *pack = NULL;
	order = ((order << 1) & 0xFE); // 最后一位为0
	pack = Hal_Ccp_Packet(receiver, order, cmd_id, answer_data, len);
	if (pack == NULL) {
		return HAL_CCP_CREATE_ERROR;
	}
	volatile hal_ccp_info_t *info = NULL;
	info = hal_ccp_creat_tail_node(&hal_ccp_ready_list);
	if (info == NULL) {
		HAL_CCP_FREE(pack);
		return HAL_CCP_CREATE_ERROR;
	}
	info->pack = pack;
	info->cmd_id =
		((hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack))->cmd_id;
	info->pack_size = HAL_CCP_GET_PACK_SIZE(len);
	info->error_cnt = 0;
	return HAL_CCP_TRUE;
}

hal_ccp_status hal_ccp_transmit_handler(uint32_t tick)
{
	lib_node_t *node = NULL;
	lib_node_t *buf = NULL;
	lib_list_t *wait_list = NULL;
	lib_list_t *delay_list = &hal_ccp_delay_list;
	lib_list_t *ready_list = &hal_ccp_ready_list;
	hal_ccp_cache_node_t *cache = NULL;
	hal_ccp_info_t *info = NULL;

	lib_list_for_each_safe(delay_list, node, buf) {
		cache = (hal_ccp_cache_node_t *)container_addr(
			node, hal_ccp_cache_node_t, node);

		info = &cache->info;
		info->time += tick;
		if (info->time >= info->delay) {
			/* move nodes to ready list Lib_TSP_TaskListTail*/
			info->time = 0;
			hal_ccp_change_node(delay_list, ready_list, cache, 0);
		}
	}

	for (uint16_t i = 0; i < HAL_CCP_WAIT_BACK_LIST_SIZE; ++i) {
		wait_list = &hal_ccp_wait_back_list[i];
		lib_list_for_each_safe(wait_list, node, buf) {
			cache = (hal_ccp_cache_node_t *)container_addr(
				node, hal_ccp_cache_node_t, node);
			info = &cache->info;
			info->time += tick;
			if (info->time >= info->timeout) {
				info->time = 0;
				++info->error_cnt;

				if (info->error_cnt >= HAL_CCP_ERROR_CNT_MAX) {
					/* error */
					hal_ccp_delete_node(wait_list, &cache);
				} else {
					/* move nodes to ready list
					 * Lib_TSP_TaskListTail */
					hal_ccp_change_node(wait_list,
							    ready_list, cache,
							    0);
				}
			}
		}
	}

	lib_list_for_each_safe(ready_list, node, buf) {
		cache = (hal_ccp_cache_node_t *)container_addr(
			node, hal_ccp_cache_node_t, node);
		info = &cache->info;
		if (hal_ccp_prot_send(info->pack, info->pack_size)) {
			/* error */
			++info->error_cnt;
			if (info->error_cnt >= HAL_CCP_ERROR_CNT_MAX) {
				hal_ccp_delete_node(ready_list, &cache);
			}
			return HAL_CCP_SEND_ERROR;
		}
		if (info->timeout != 0) {
			/* move nodes to wait back list by id */
			uint16_t id =
				info->cmd_id % HAL_CCP_WAIT_BACK_LIST_SIZE;
			hal_ccp_change_node(ready_list,
					    &hal_ccp_wait_back_list[id], cache,
					    0);
		} else {
			hal_ccp_delete_node(ready_list, &cache);
		}
		return HAL_CCP_TRUE;
	}
	return HAL_CCP_TRUE;
}

hal_ccp_status Hal_Ccp_ReceiveCheck(const uint8_t *pack, uint16_t pack_size)
{
	hal_ccp_cover_head_t *head =
		(hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack);
	hal_ccp_cover_tail_t *tail =
		(hal_ccp_cover_tail_t *)HAL_CCP_GET_PACK_TAIL(pack, head->len);

	/* 包头包尾判断 */
	if (head->start != HAL_CCP_FRAME_HEAD ||
	    tail->end != HAL_CCP_FRAME_TAIL) {
		return HAL_CCP_FALSE;
	}

	/* 校验和判断 */
	uint8_t check = 0;
	for (size_t i = 1; i < pack_size - HAL_CCP_COVER_Tail_SIZE; ++i) {
		check += pack[i];
	}

	if (check != tail->check) {
		return HAL_CCP_FALSE;
	}

	/* 判断该信息是否应由本机接收 */
	if (head->receiver != hal_ccp_local_id && head->receiver != 0xFF) {
		return HAL_CCP_FALSE;
	}

	return HAL_CCP_TRUE;
}

hal_ccp_status hal_ccp_clear_cache(uint8_t *pack, uint16_t len)
{
	hal_ccp_cover_head_t *head =
		(hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack);
	hal_ccp_cover_tail_t *tail =
		(hal_ccp_cover_tail_t *)HAL_CCP_GET_PACK_TAIL(pack, head->len);
	if (head->cmd_id != 0) {
		hal_ccp_cache_node_t *cache = NULL;
		hal_ccp_info_t *info = NULL;
		lib_node_t *node = NULL;
		lib_node_t *buf = NULL;
		lib_list_t *wait_list =
			&hal_ccp_wait_back_list[head->cmd_id %
						HAL_CCP_WAIT_BACK_LIST_SIZE];
		lib_list_t *ready_list = &hal_ccp_ready_list;

		lib_list_for_each_safe(wait_list, node, buf) {
			cache = (hal_ccp_cache_node_t *)container_addr(
				node, hal_ccp_cache_node_t, node);
			info = &cache->info;
			if (info->cmd_id == head->cmd_id) {
				hal_ccp_delete_node(wait_list, &cache);
				goto HAL_CCP_RECEIVE_TASK;
			}
		}

		lib_list_for_each_safe(ready_list, node, buf) {
			cache = (hal_ccp_cache_node_t *)container_addr(
				node, hal_ccp_cache_node_t, node);
			info = &cache->info;
			if (info->cmd_id == head->cmd_id) {
				hal_ccp_delete_node(ready_list, &cache);
				goto HAL_CCP_RECEIVE_TASK;
			}
		}

		lib_list_for_each_safe(wait_list, node, buf) {
			cache = (hal_ccp_cache_node_t *)container_addr(
				node, hal_ccp_cache_node_t, node);
			info = &cache->info;
			if (info->cmd_id == head->cmd_id) {
				hal_ccp_delete_node(wait_list, &cache);
				goto HAL_CCP_RECEIVE_TASK;
			}
		}
	}
HAL_CCP_RECEIVE_TASK:
	return HAL_CCP_TRUE;
}

hal_ccp_status hal_ccp_receive_handler(uint8_t *pack, uint16_t len)
{
	if (pack == NULL || len == 0) {
		return HAL_CCP_FALSE;
	}

	/* 校验 */
	if (Hal_Ccp_ReceiveCheck(pack, len)) {
		return HAL_CCP_CHECK_ERROR;
	}

	/* 判断应答是否成功 */
	hal_ccp_clear_cache(pack, len);

	/* 指令解析 */
	hal_ccp_protocol_parse(pack, len);
	return HAL_CCP_TRUE;
}

size_t hal_ccp_get_existing_node(void)
{
	return hal_ccp_node_num;
}

void PrintHex(const uint8_t *data, size_t len)
{
	for (size_t i = 0; i < len; ++i) {
		printf("%.2X ", data[i]);
	}
	printf("\r\n");
}

void hal_ccp_pack_analysis(const uint8_t *pack)
{
	if (pack == NULL) {
		return;
	}
	hal_ccp_cover_head_t *head =
		(hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack);
	uint8_t *data = (uint8_t *)HAL_CCP_GET_PACK_DATA(pack);
	hal_ccp_cover_tail_t *tail =
		(hal_ccp_cover_tail_t *)HAL_CCP_GET_PACK_TAIL(pack, head->len);

	printf("cmd_id:%d\r\n", head->cmd_id);
	printf("sender:%d\r\n", head->sender);
	printf("receiver:%d\r\n", head->receiver);
	printf("order:%X\r\n", head->order);
	printf("len:%d\r\n", head->len);
	printf("reserve1:%d\r\n", head->reserve1);
	printf("reserve2:%d\r\n", head->reserve2);
	printf("check:0X%X\r\n", tail->check);
	printf("data:");
	PrintHex(data, head->len);
	printf("\r\n");
}
