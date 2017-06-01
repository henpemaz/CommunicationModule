#include "task_scheduler.h"

#define SCHED_MAX_TASKS 8
#define SCHED_MAX_WAIT UINT16_MAX





struct task_handle {
	void (*task)(void);
	int32_t delay;
	int32_t looptime;
};

struct task_handle task_list[SCHED_MAX_TASKS];


void sched_setup(void){
	// Structure setup
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		task_list[i].task = NULL;
	}

	// Clocks setup

	// We'll be using the Watchdog since it's always ON

	// TODO FIXME can't use MILLIs because of sleep mode
	

	last_count = millis();
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

uint32_t last_count;
void scheduler_step(void) {
	uint32_t count = millis();
	int32_t diff = (count - last_count); // Arithmetic over/underflow still gives the right result
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		task_list[i].delay -= diff;
	}
	last_count = count;
}

void scheduler_reschedule(void) {
	uint8_t i;
	uint8_t best;

}