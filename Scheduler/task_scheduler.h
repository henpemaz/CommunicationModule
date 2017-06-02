#ifndef _TASK_SCHEDULER_H_
#define _TASK_SCHEDULER_H_

#include "Arduino.h"


void sched_setup(void);
void sched_mainloop(void);
uint8_t sched_add_task(void(*task)(void), int32_t delay, int32_t looptime);

#endif // !_TASK_SCHEDULER_H_

