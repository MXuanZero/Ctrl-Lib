#ifndef HAL_CCP_PROTORCOL_H
#define HAL_CCP_PROTORCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hal_ccp.h"
/* Define --------------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief 其他命令
 * @param receiver
 * @return
 */
hal_ccp_status hal_ccp_send_customize(uint8_t receiver, uint8_t *tx_data,
				      uint16_t len, uint32_t delay,
				      uint32_t timeout);

void hal_ccp_customize_handler(uint8_t sender, uint16_t cmd_id, uint8_t *data,
			       uint16_t len);

/**
 * @brief 获取参数系统的参数值
 * @param receiver
 * @param uid
 */
void hal_ccp_get_store_param(uint8_t receiver, uint16_t uid, uint32_t delay,
			     uint32_t timeout);

/**
 * @brief 设置参数系统的参数值
 * @param receiver
 * @param uid
 */
void hal_ccp_set_store_param(uint8_t receiver, uint16_t uid, void *val,
			     uint32_t delay, uint32_t timeout);

/**
 * @brief 应答
 */
hal_ccp_status hal_ccp_answer_set_store_param(uint8_t sender, uint16_t cmd_id,
					      uint8_t *data, uint16_t len);

/**
 * @brief 协议处理
 */
void hal_ccp_protocol_parse(uint8_t *pack, uint16_t pack_size);

#ifdef __cplusplus
}
#endif

#endif // HAL_CCP_PROTORCOL_H
