#ifndef BUTTON_H
#define BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif

/* include */
#include <stdint.h>
#include <stdbool.h>

/* define */
#ifndef BUTTON_DOUBLE_CLICK_ENABLE
#define BUTTON_DOUBLE_CLICK_ENABLE 1 // 是否启用双击
#endif

#define DEF_HAL_BUTTON_T_SIZE sizeof(button_t)

/* typedef */
typedef enum {
	BUTTON_NORMAL = 0, // 无时间
	BUTTON_UP, // 抬起瞬间
	BUTTON_DOWN, // 按下瞬间
	BUTTON_DOWN_PRESS, // 按下中
	BUTTON_CLICK, // 点击事件
	BUTTON_LONG_PRESS, // 长按事件
	BUTTON_DOUBLE_CLICK, // 双击事件
	BUTTON_EVENT_MAX,
} button_event;

typedef void (*button_callback_fn)(button_event e);
typedef bool (*button_read_io_fn)(uint8_t id);

typedef struct button_handle {
	struct button_handle *next;
	button_callback_fn callback_fn; // 按键事件回调函数
	uint32_t press_time; // 时间
	uint32_t long_press_time; // 长按判定时间
#if BUTTON_DOUBLE_CLICK_ENABLE == 1
	uint32_t double_click_time; // 双击间隔时间
	uint32_t double_click_interval_time;
#endif
	uint8_t event;
	uint8_t id;
	bool last_state; // 按键上次按下状态
} button_t;

typedef struct button_group_handle {
	struct button_handle *first;
	button_read_io_fn read_io_fn;
	uint32_t tick; // 时刻
	uint8_t num; // 按键组数量
} button_group_t;

/* Exported constants --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */


/**
 * @brief 按键动态创建函数
 */
button_t *button_init(uint8_t id, uint32_t lpt, uint32_t dct,
			  button_callback_fn cb_fn);

/**
 * @brief 按键静态创建函数
 */
void button_init_stiaic(button_t *btn, uint8_t id, uint32_t lpt,
			uint32_t dct, button_callback_fn cb_fn);

/**
 * @brief 按键注册函数
 */
void button_reg(button_group_t *btn_group, button_t *btn);

/**
 * @brief 按键时间更新
 */
void button_update_tick(button_group_t *btn_group, uint32_t tick);

/**
 * @brief 按键任务
 */
void button_handler(button_group_t *btn_group);

#ifdef __cplusplus
}
#endif

#endif //__HAL_BUTTON_H__
