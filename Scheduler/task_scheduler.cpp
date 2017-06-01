#include "task_scheduler.h"

#define SCHED_MAX_TASKS 8
#define SCHED_MAX_WAIT UINT16_MAX

uint32_t sleep_duration;

struct task_handle {
	void (*task)(void);
	int32_t delay;
	int32_t looptime;
};

struct task_handle task_list[SCHED_MAX_TASKS];
struct task_handle *current_task;


void scheduler_step(void);
void scheduler_reschedule(void);


void sched_setup(void){
	// Structure setup
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		task_list[i].task = NULL;
	}
	current_task = NULL;

	// Clocks setup
	// We'll be using the Watchdog since it's always ON

	// TODO FIXME can't use MILLIs inbetween sleeps because of sleep mode
	

	sleep_duration = SCHED_MAX_WAIT;
}


uint8_t sched_add_task(void (*task)(void), uint32_t delay, uint32_t looptime) {
	noInterrupts();

	scheduler_step();

	// Add to list
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		if (task_list[i].task == NULL) {
			task_list[i].task = task;
			task_list[i].delay = delay;
			task_list[i].looptime = looptime;
			break;
		}
	}
	if (i == SCHED_MAX_TASKS) {
		// Max tasks limit reached
		return -1;
	}

	scheduler_reschedule();

	interrupts();

	return i;
}


void scheduler_step(void) {
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		task_list[i].delay -= sleep_duration;
	}
}

void scheduler_reschedule(void) {
	uint8_t i;


}