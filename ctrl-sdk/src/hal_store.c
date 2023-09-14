/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "hal_store.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define DEF_HAL_STORE_MALLOC(x) malloc(x)
#define DEF_HAL_STORE_FREE(x) free(x)
#define DEF_HAL_STORE_INIT_EMPTY 1 // 创建链表后是否需要清零

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static lib_list_t hal_store_map[HAL_STORE_MAP_LIST_SIZE] = { 0 };
static uint16_t hal_store_node_num = 0;

/* Private function prototypes -----------------------------------------------*/
/**
 * @brief 初始化链表头节点
 */
static void hal_store_init_list(void);

/**
 * @brief 创建节点
 */
static inline hal_store_node_t *hal_store_create_node(uint16_t uid);

/**
 * @brief  删除节点
 */
static inline void hal_store_del_node(uint16_t uid, hal_store_node_t *node);

/* Private functions ---------------------------------------------------------*/
static void hal_store_init_list(void)
{
	for (uint16_t i = 0; i < HAL_STORE_MAP_LIST_SIZE; ++i) {
		lib_list_init(&hal_store_map[i], HAL_STORE_NODE_SIZE);
	}
}

static inline hal_store_node_t *hal_store_create_node(uint16_t uid)
{
	void *new = DEF_HAL_STORE_MALLOC(HAL_STORE_NODE_SIZE);
	if (new == NULL) {
		return NULL;
	}
#if DEF_HAL_STORE_INIT_EMPTY == 1
	memset(new, 0, HAL_STORE_NODE_SIZE);
#endif

	lib_node_t *node = (lib_node_t *)((char *)new +
					  addr_offset(hal_store_node_t, node));
	lib_list_inc_tail(&hal_store_map[uid % HAL_STORE_MAP_LIST_SIZE], node);
	return new;
}

static inline void hal_store_del_node(uint16_t uid, hal_store_node_t *node)
{
	lib_list_delete(&hal_store_map[uid % HAL_STORE_MAP_LIST_SIZE],
			&node->node);
	DEF_HAL_STORE_FREE(node);
	node = NULL;
}

hal_store_status hal_store_init(void)
{
	hal_store_init_list();
	hal_store_port_init();
	return HAL_STORE_OK;
}

hal_store_status hal_store_reg(hal_store_info_t *info)
{
	if (info == NULL) {
		return HAL_STORE_ERROR;
	}

	/* find */
	if (info->uid == 0 || info->size == 0) {
		return HAL_STORE_CREATE_ERROR;
	}
	if (hal_store_find(info->uid) != NULL) {
		return HAL_STORE_CREATE_ERROR;
	}

	/* add */
	if (hal_store_node_num >= HAL_STORE_PARAM_NUM_MAX) {
		return HAL_STORE_FULL;
	}
	hal_store_node_t *node = hal_store_create_node(info->uid);
	if (node == NULL) {
		return HAL_STORE_CREATE_ERROR;
	}
	memcpy(&node->info, info, HAL_STORE_INFO_SIZE);
	++hal_store_node_num;
	return HAL_STORE_OK;
}

hal_store_info_t *hal_store_find(uint16_t uid)
{
	lib_node_t *node = NULL;
	hal_store_node_t *store_node = NULL;
	if (uid == 0) {
		return NULL;
	}
	lib_list_t *list = &hal_store_map[uid % HAL_STORE_MAP_LIST_SIZE];
	lib_list_for_each(list, node) {
		store_node = container_addr(node, hal_store_node_t, node);
		if (store_node->info.uid == uid) {
			return &store_node->info;
		}
	}
	return NULL;
}

hal_store_status hal_store_set_param(uint16_t uid, void *param,
				    void *param2)
{
	hal_store_info_t *info = NULL;
	hal_store_update_cb update_cb_fn = NULL;
	if (uid == 0) {
		return HAL_STORE_ERROR;
	}
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	if (info->write == HAL_STORE_WRITE_PRESERVE) {
		return HAL_STORE_WRITE_ERROR;
	}
	memcpy(info->data, param, info->size);
	/* 更新回调函数 */
	update_cb_fn = info->update_cb_fn;
	HAL_STORE_UNLOCK();
	if (update_cb_fn != NULL) {
		update_cb_fn(param, param2);
	}
	return HAL_STORE_OK;
}

hal_store_status hal_store_get_param(uint16_t uid, void *data)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	if (info->read == HAL_STORE_READ_PRESERVE) {
		return HAL_STORE_READ_ERROR;
	}
	memcpy(data, info->data, info->size);
	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_get_size(uint16_t uid, uint32_t *size)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	*size = info->size;
	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_set_cb(uint16_t uid, hal_store_update_cb fn)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	info->update_cb_fn = fn;
	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_del_update_cb(uint16_t uid)
{
	return hal_store_set_cb(uid, NULL);
}

/**
 * @brief 删除该uid的节点
 */
hal_store_status hal_store_del(uint16_t uid)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	hal_store_node_t *node = container_addr(info, hal_store_node_t, info);
	hal_store_del_node(uid, node);
	--hal_store_node_num;

	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

void hal_store_del_all(void)
{
	HAL_STORE_LOCK();
	// for(uint16_t i = 0; i < HAL_STORE_MAP_LIST_SIZE; ++i)
	// {
	// }
	HAL_STORE_UNLOCK();
}

hal_store_status hal_store_set_read_permission(uint16_t uid, _Bool i)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	info->read = i;

	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_get_read_permission(uint16_t uid, _Bool *i)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	*i = info->read;

	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_set_write_permission(uint16_t uid, _Bool i)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	info->write = i;

	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}

hal_store_status hal_store_get_write_permission(uint16_t uid, _Bool *i)
{
	hal_store_info_t *info = NULL;
	HAL_STORE_LOCK();

	info = hal_store_find(uid);
	if (info == NULL) {
		return HAL_STORE_NOT_FIND;
	}
	*i = info->write;

	HAL_STORE_UNLOCK();
	return HAL_STORE_OK;
}
