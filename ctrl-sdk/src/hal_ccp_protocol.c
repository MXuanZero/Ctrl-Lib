/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "hal_ccp.h"
#include "hal_ccp_protocol.h"
#include "hal_store.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

hal_ccp_status hal_ccp_send_customize(uint8_t receiver, uint8_t *tx_data,
				      uint16_t len, uint32_t delay,
				      uint32_t timeout)
{
	return hal_ccp_transmit(receiver, HAL_CCP_ORDER_CUSTOMIZE, tx_data, len,
				delay, timeout);
}

void hal_ccp_get_store_param(uint8_t receiver, uint16_t uid, uint32_t delay,
			     uint32_t timeout)
{
	hal_ccp_transmit(receiver, HAL_CCP_ORDER_GET_PARAM, (uint8_t *)&uid,
			 HAL_STORE_UID_SIZE, delay, timeout);
}

hal_ccp_status hal_ccp_answer_get_store_param(uint8_t sender, uint16_t cmd_id,
					      const uint8_t *data, uint16_t len)
{
	hal_ccp_status state;
	uint16_t uid = *(uint16_t *)data;
	hal_store_info_t *info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_CCP_NOT_FIND;
	}
	uint16_t tx_data_size = info->size + HAL_STORE_UID_SIZE + 1;
	uint8_t *tx_data = malloc(tx_data_size);

	memcpy(tx_data, &uid, HAL_STORE_UID_SIZE);
	tx_data[2] = hal_store_get_param(
		uid, (void *)(tx_data + HAL_STORE_UID_SIZE + 1));
	state = hal_ccp_answer(sender, HAL_CCP_ORDER_GET_PARAM, cmd_id, tx_data,
			       tx_data_size);
	free(tx_data);
	return state;
}

void hal_ccp_rx_answer_get_store_param(uint8_t sender, uint16_t cmd_id,
				       uint8_t *data, uint16_t len)
{
	uint16_t uid = *(uint16_t *)data;
	uint8_t state = *(data + HAL_STORE_UID_SIZE);
	if (!state) {
		hal_store_set_param(
			uid, (void *)(data + HAL_STORE_UID_SIZE + 1), NULL);
	}
}

void hal_ccp_set_store_param(uint8_t receiver, uint16_t uid, void *val,
			     uint32_t delay, uint32_t timeout)
{
	if (val == NULL) {
		return;
	}
	hal_store_info_t *info = hal_store_find(uid);
	if (info == NULL) {
		return;
	}
	uint8_t *tx_data = malloc(info->size + HAL_STORE_UID_SIZE);
	memcpy(tx_data, &uid, HAL_STORE_UID_SIZE);
	memcpy(tx_data + HAL_STORE_UID_SIZE, val, info->size);
	hal_ccp_transmit(receiver, HAL_CCP_ORDER_SET_PARAM, tx_data,
			 info->size + HAL_STORE_UID_SIZE, delay, timeout);
	free(tx_data);
}

hal_ccp_status hal_ccp_answer_set_store_param(uint8_t sender, uint16_t cmd_id,
					      uint8_t *data, uint16_t len)
{
	union {
		uint16_t uid;
		uint8_t data[3];
	} ret;
	ret.uid = *(uint16_t *)data;
	ret.data[2] = hal_store_set_param(
		ret.uid, (void *)(data + HAL_STORE_UID_SIZE), NULL);
	return hal_ccp_answer(sender, HAL_CCP_ORDER_SET_PARAM, cmd_id, ret.data,
			      3);
}

__attribute__((weak)) void hal_ccp_customize_handler(uint8_t sender,
						     uint16_t cmd_id,
						     uint8_t *data,
						     uint16_t len)
{
	hal_ccp_answer(sender, HAL_CCP_ORDER_CUSTOMIZE, cmd_id, (uint8_t *)"ok",
		       2);
}

void hal_ccp_protocol_parse(uint8_t *pack, uint16_t pack_size)
{
	hal_ccp_cover_head_t *head;
	hal_ccp_cover_tail_t *tail;
	uint8_t *data;
	uint8_t order;
	_Bool answer;

	head = (hal_ccp_cover_head_t *)HAL_CCP_GET_PACK_HEAD(pack);
	tail = (hal_ccp_cover_tail_t *)HAL_CCP_GET_PACK_TAIL(pack, head->len);
	data = (uint8_t *)HAL_CCP_GET_PACK_DATA(head);
	order = (head->order >> 1) & 0x7F;
	answer = (((head->order) >> (0)) & 0x01);

	if (answer == 1) {
		/* ���յ������ź� */
		switch (order) {
		case HAL_CCP_ORDER_GET_PARAM:
			hal_ccp_answer_get_store_param(
				head->sender, head->cmd_id, data, head->len);
			break;
		case HAL_CCP_ORDER_SET_PARAM:
			hal_ccp_answer_set_store_param(
				head->sender, head->cmd_id, data, head->len);
			break;
		case HAL_CCP_ORDER_CUSTOMIZE:
			hal_ccp_customize_handler(head->sender, head->cmd_id,
						  data, head->len);
			break;
		default:
			break;
		}
	} else {
		/* ���յ�Ӧ���ź� */
		switch (order) {
		case HAL_CCP_ORDER_GET_PARAM:
			hal_ccp_rx_answer_get_store_param(
				head->sender, head->cmd_id, data, head->len);
			break;
		case HAL_CCP_ORDER_SET_PARAM:
			break;
		case HAL_CCP_ORDER_CUSTOMIZE:
			break;
		default:
			break;
		}
	}
}
