#ifndef LIB_TIMER_H
#define LIB_TIMER_H
/* Includes ------------------------------------------------------------------*/
#include "lib_list.h"
/* Define --------------------------------------------------------------------*/
#define LIB_TIMER_UNIT uint32_t
#define LIB_TIMER_PERIOD_MAX (UINT32_MAX >> 1)
#define LIB_USE_SELF_DATA 1
/* Exported macro ------------------------------------------------------------*/
/* Exported typedef ----------------------------------------------------------*/
typedef enum lib_timer_event {
	LIB_TIMER_NORMAL = 0,
	LIB_TIMER_TIMEOUT = 1,
} lib_timer_event;

typedef enum lib_timer_status {
	LIB_TIMER_READY = 0,
	LIB_TIMER_DELAY = 1,
	LIB_TIMER_STOP = 2,
} lib_timer_status;

typedef enum lib_timer_error {
	LIB_TIMER_OK = 0,
	LIB_TIEMR_ERROR,
} lib_timer_error;

typedef void (*lib_timer_callback_fn)(lib_timer_event event, void *user_data);

/**
 * @next next lib_timer_t
 * @cb_fn callback function
 * @period between each execution
 * @timeout timeout
 * @repeat_count if the number becomes 0, the list is immediately disconnected
 */
typedef struct lib_timer_t {
	lib_node_t node;
	lib_timer_callback_fn cb_fn;
#if LIB_USE_SELF_DATA
	void *user_data;
#endif // LIB_USE_SELF_DATA
	uint32_t run_time;
	lib_timer_status state;
	LIB_TIMER_UNIT period;
	LIB_TIMER_UNIT timeout;
	LIB_TIMER_UNIT last_run;
	int32_t repeat_cnt;
} lib_timer_t;

typedef struct lib_timer_group_t {
	lib_list_t list;
	lib_node_t node;
	uint16_t id;
} lib_timer_group_t;
/* Exported constants --------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void lib_timer_init(lib_timer_t *timer, LIB_TIMER_UNIT period,
		    LIB_TIMER_UNIT timeout, lib_timer_callback_fn cb_fn);
void lib_timer_group_init(lib_timer_group_t *group);
void lib_timer_reg(lib_timer_group_t *group, lib_timer_t *timer);
void lib_timer_handler(lib_timer_group_t *group);
void lib_timer_group_handler(void);
lib_timer_error lib_timer_delay(lib_timer_t *timer, LIB_TIMER_UNIT ms);
lib_timer_error lib_timer_group_delay(lib_timer_group_t *group,
				      LIB_TIMER_UNIT ms);
void lib_timer_stop(lib_timer_t *timer);
void lib_timer_restart(lib_timer_t *timer);
#endif //LIB_TIMER_H
