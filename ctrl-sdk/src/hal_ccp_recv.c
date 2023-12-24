/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <string.h>
#include "hal_ccp_recv.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void hal_ccp_recv_init(hal_ccp_recv_handle_t *recv, uint8_t id,
		       hal_ccp_recv_processing_fn fn)
{
	if (recv == NULL) {
		return;
	}
	if (recv->id != 0 || id == 0) {
		return;
	}
	memset(recv, 0, sizeof(hal_ccp_recv_handle_t));
	recv->id = id;
	recv->processing_fn = fn;
	lib_queue_static_init(&recv->queue, recv->data,
			      sizeof(hal_ccp_recv_data_t),
			      HAL_CCP_RECV_BUF_NUM);
	hal_ccp_recv_prot_init(recv);
}

void hal_ccp_recv_group_reg(hal_ccp_recv_group_handle_t *group,
			   hal_ccp_recv_handle_t *recv)
{
	if (group == NULL || recv == NULL) {
		return;
	}
	uint8_t id = recv->id;
	for (hal_ccp_recv_handle_t *pos = group->first; pos != NULL;
	     pos = pos->next) {
		if (pos->id == id) {
			return;
		}
	}
	if (group->first == NULL) {
		group->first = recv;
	} else {
		hal_ccp_recv_handle_t *pos = group->first;
		while (pos->next != NULL) {
			pos = pos->next;
		}
		pos->next = recv;
	}
	++group->num;
}

hal_ccp_recv_status hal_ccp_recv_handler(hal_ccp_recv_handle_t *recv)
{
	lib_queue_status state;
	if (recv == NULL) {
		return HAL_CCP_RECV_ERROR;
	}

	HAL_CCP_RECV_LOCK();
	state = lib_queue_static_push(&recv->queue, &recv->buffer);
	HAL_CCP_RECV_UNLOCK();
	if (state == LIB_QUEUE_STATE_TRUE) {
		return HAL_CCP_RECV_TRUE;
	} else if (state == LIB_QUEUE_STATE_FULL) {
		return HAL_CCP_RECV_OVERFLOW;
	} else {
		return HAL_CCP_RECV_ERROR;
	}
}

void hal_ccp_recv_processor(hal_ccp_recv_group_handle_t *center)
{
	if (center == NULL) {
		return;
	}
	/* 数据处理 */
	hal_ccp_recv_data_t data = { 0 };
	lib_queue_status state = LIB_QUEUE_STATE_EMPTY;
	for (hal_ccp_recv_handle_t *recv = center->first; recv != NULL;
	     recv = recv->next) {
HAL_CCP_RECV_EACH_MEMBER:
		HAL_CCP_RECV_LOCK();
		state = lib_queue_static_pop(&recv->queue, &data);
		HAL_CCP_RECV_UNLOCK();
		switch (state) {
		case LIB_QUEUE_STATE_TRUE:
			if (recv->processing_fn != NULL) {
				recv->processing_fn(data.data, data.size);
			}
			/* 遍历队列成员，直到把所待处理的数据全部处理完 */
			/* @warning 发送数据过快可能会导致一直卡在这一段程序 */
			goto HAL_CCP_RECV_EACH_MEMBER;
		case LIB_QUEUE_STATE_EMPTY:
			continue;
		default:
			/* error */
			break;
		}
	}
}
