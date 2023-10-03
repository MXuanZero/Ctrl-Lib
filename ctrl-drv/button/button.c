/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include "button.h"
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define BUTTON_MIN(a, b) ((a) < (b) ? (a) : (b))
#define BUTTON_MAX(a, b) ((a) > (b) ? (a) : (b))
#define BUTTON_CONSTRAIN(amt, low, high) \
	((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))
#define BUTTON_BIT_READ(value, bit) (((value) >> (bit)) & 0x01)
#define BUTTON_BIT_SET(value, bit) ((value) |= (1UL << (bit)))
#define BUTTON_BIT_CLEAR(value, bit) ((value) &= ~(1UL << (bit)))
#define BUTTON_BIT_TOGGLE(value, bit) ((value) ^= (1UL << (bit)))
#define BUTTON_BIT_WRITE(value, bit, bitvalue)      \
	((bitvalue) ? BUTTON_BIT_READ(value, bit) : \
		      BUTTON_BIT_CLEAR(value, bit))

#define BUTTON_MALLOC(x) malloc(x)
#define BUTTON_FREE(x) free(x)
#define BUTTON_INIT_EMPT 1

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
button_t *button_init(uint8_t id, uint32_t lpt, uint32_t dct,
		      button_callback_fn cb_fn)
{
	button_t *btn = malloc(DEF_HAL_BUTTON_T_SIZE);
	if (btn == NULL) {
		return NULL;
	}
	memset(btn, 0, DEF_HAL_BUTTON_T_SIZE);
	btn->id = id;
	btn->long_press_time = lpt;
#if BUTTON_DOUBLE_CLICK_ENABLE == 1
	btn->double_click_interval_time = dct;
#endif
	btn->callback_fn = cb_fn;
	return btn;
}

void button_init_stiaic(button_t *btn, uint8_t id, uint32_t lpt, uint32_t dct,
			button_callback_fn cb_fn)
{
	if (btn == NULL) {
		return;
	}
	memset(btn, 0, DEF_HAL_BUTTON_T_SIZE);
	btn->id = id;
	btn->long_press_time = lpt;
#if BUTTON_DOUBLE_CLICK_ENABLE == 1
	btn->double_click_interval_time = dct;
#endif
	btn->callback_fn = cb_fn;
}

void hal_button_reg(button_group_t *btn_group, button_t *btn)
{
	if (btn_group == NULL || btn == NULL) {
		return;
	}
	if (btn_group->first == NULL) {
		btn_group->first = btn;
	} else {
		button_t *tail = btn_group->first;
		while (tail->next != NULL) {
			tail = tail->next;
		}
		tail->next = btn;
	}
	btn->next = NULL;
	++btn_group->num;
}

/* clang-format off */
void button_update_tick(button_group_t *btn_group, uint32_t tick)
{
	if (btn_group == NULL) {
		return;
	}
	btn_group->tick += tick;
	for (button_t *btn = btn_group->first; btn != NULL; btn = btn->next) {
		bool state = button_prot_read_io(btn->id);
#if BUTTON_DOUBLE_CLICK_ENABLE == 1 // 用一个if判断双击方便阅读，不要改动
		if (btn->double_click_interval_time != 0) {
			if (btn->double_click_time != 0) {
				btn->double_click_time = BUTTON_MAX(btn->double_click_time - tick, 0);
			}

			if (state != btn->last_state) {
				btn->last_state = state;
				BUTTON_BIT_CLEAR(btn->event, HAL_BUTTON_NORMAL);
				if (state) {
					// 按下
					btn->press_time = btn_group->tick;
					BUTTON_BIT_SET(btn->event,HAL_BUTTON_DOWN);
				} else {
					// 抬起
					BUTTON_BIT_SET(btn->event,HAL_BUTTON_UP);
					if (btn_group->tick - btn->press_time >btn->long_press_time
					    && btn->long_press_time != 0) {
						BUTTON_BIT_SET(btn->event, HAL_BUTTON_LONG_PRESS);
					} else if (btn->double_click_time > 0) {
						// 双击
						btn->double_click_time = 0;
						BUTTON_BIT_SET(btn->event, HAL_BUTTON_DOUBLE_CLICK);
					} else {
						// 单击
						if (btn->double_click_time == 0) {
							btn->double_click_time = btn->double_click_interval_time;
						}
						BUTTON_BIT_SET(btn->event, HAL_BUTTON_CLICK);
					}
				}
			} else {
				BUTTON_BIT_SET(btn->event, HAL_BUTTON_NORMAL);
				BUTTON_BIT_SET(btn->event, HAL_BUTTON_DOWN_PRESS);
			}
		}
#endif
		if (state != btn->last_state) {
			btn->last_state = state;
			BUTTON_BIT_CLEAR(btn->event, HAL_BUTTON_NORMAL);
			if (state) {
				// 按下
				btn->press_time = btn_group->tick;
				BUTTON_BIT_SET(btn->event, HAL_BUTTON_DOWN);
			} else {
				// 抬起
				BUTTON_BIT_SET(btn->event, HAL_BUTTON_UP);
				if (btn_group->tick - btn->press_time > btn->long_press_time
				    && btn->long_press_time != 0) {
					BUTTON_BIT_SET(btn->event, HAL_BUTTON_LONG_PRESS);
				} else { // 单击
					BUTTON_BIT_SET(btn->event, HAL_BUTTON_CLICK);
				}
			}
		} else if (state == 1) {
			BUTTON_BIT_CLEAR(btn->event, HAL_BUTTON_NORMAL);
			BUTTON_BIT_SET(btn->event, HAL_BUTTON_DOWN_PRESS);
		}
	}
}
/* clang-format on */

void button_handler(button_group_t *btn_group)
{
	if (btn_group == NULL) {
		return;
	}

	for (button_t *btn = btn_group->first; btn != NULL; btn = btn->next) {
		if (BUTTON_BIT_READ(btn->event, HAL_BUTTON_NORMAL) ||
		    btn->callback_fn == NULL) {
			continue; // Hal_Button_Normal置1说明没有任何事件产生
		}
		for (button_event i = HAL_BUTTON_UP; i < HAL_BUTTON_EVENT_MAX;
		     ++i) {
			if (BUTTON_BIT_READ(btn->event, i)) {
				btn->callback_fn(i);
			}
		}

		btn->event = 0;
		BUTTON_BIT_SET(btn->event, HAL_BUTTON_NORMAL);
	}
}
