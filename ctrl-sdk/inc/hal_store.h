#ifndef HAL_STORE_H
#define HAL_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "hal_cfg.h"
#include <stdint.h>
#include "lib_list.h"

/* Define --------------------------------------------------------------------*/
#define HAL_STORE_READ_PRESERVE 1
#define HAL_STORE_WRITE_PRESERVE 1
#define HAL_STORE_READ_NOT_PRESERVE 0
#define HAL_STORE_WRITE_NOT_PRESERVE 0

#define HAL_STORE_INFO_SIZE sizeof(hal_store_info_t)
#define HAL_STORE_NODE_SIZE sizeof(hal_store_node_t)
#define HAL_STORE_UID_SIZE sizeof(uint16_t)

/* Exported macro ------------------------------------------------------------*/
#if HAL_STORE_LOCK_ENABLE == 1
#define HAL_STORE_LOCK() Hal_Store_Prot_Lock() // ����
#define HAL_STORE_UNLOCK() Hal_Store_Prot_Unlock() // ����
#else
#define HAL_STORE_LOCK()
#define HAL_STORE_UNLOCK()
#endif
/* Exported types ------------------------------------------------------------*/
typedef void (*hal_store_update_cb)(void *, void *);

typedef enum {
	HAL_STORE_OK,
	HAL_STORE_ERROR,
	HAL_STORE_CREATE_ERROR,
	HAL_STORE_FULL,
	HAL_STORE_NOT_FIND,
	HAL_STORE_READ_ERROR,
	HAL_STORE_WRITE_ERROR,
} hal_store_status;

typedef enum {
	HAL_STORE_U8 = 1,
	HAL_STORE_S8,
	HAL_STORE_U16,
	HAL_STORE_S16,
	HAL_STORE_U32,
	HAL_STORE_S32,
	HAL_STORE_U64,
	HAL_STORE_S64,
	HAL_STORE_FLOAT,
	HAL_STORE_DOUBLE,
	HAL_STORE_STRING,
	HAL_STORE_STRUCT,
} hal_store_type;

typedef struct hal_store_info {
	hal_store_update_cb update_cb_fn;
	void *data;
	uint32_t size;
	hal_store_type type;
	uint16_t uid;
	_Bool read;
	_Bool write;
} hal_store_info_t;

typedef struct hal_store_node {
	hal_store_info_t info;
	lib_node_t node;
} hal_store_node_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
hal_store_status hal_store_init(void);

hal_store_status hal_store_reg(hal_store_info_t *info);

hal_store_info_t *hal_store_find(uint16_t uid);

hal_store_status hal_store_set_param(uint16_t uid, void *param, void *param2);

hal_store_status hal_store_get_param(uint16_t uid, void *data);

hal_store_status hal_store_get_size(uint16_t uid, uint32_t *size);

hal_store_status hal_store_replace_cb(uint16_t uid, hal_store_update_cb fn);

hal_store_status hal_store_del_update_cb_fn(uint16_t uid);

hal_store_status hal_store_del(uint16_t uid);

/**
 * @brief 删除所有数据（未实现）
 */
void hal_store_del_all(void);

hal_store_status hal_store_set_read_permission(uint16_t uid, _Bool i);

hal_store_status hal_store_get_read_permission(uint16_t uid, _Bool *i);

hal_store_status hal_store_set_write_permission(uint16_t uid, _Bool i);

hal_store_status hal_store_get_write_permission(uint16_t uid, _Bool *i);

void Hal_Store_Prot_Init(void);

void Hal_Store_Prot_Lock(void);

void Hal_Store_Prot_Unlock(void);

#ifdef __cplusplus
}
#endif

#endif //__LIB_STORE_H__
