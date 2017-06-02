#include "task_scheduler.h"

/*
	Simple scheduler
	1Hz tick interruption updates the timers
	mainloop calls tasks ready to be executed (non-preemptivelly)
	tasks can be made cyclic (looptime > 0)
*/

#define SCHED_MAX_TASKS 8


struct task_handle {
	void (*task)(void);
	volatile int32_t delay;
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
	// Configure the Watchdog for 1Hz interrupts
	noInterrupts();
	wdt_reset();
	/* Setup Watchdog */ // Source : MICROCHIP APP NOTE AVR132
	// Use Timed Sequence for disabling Watchdog System Reset Mode if it has been enabled unintentionally.
	MCUSR &= ~(1 << WDRF);                                 // Clear WDRF if it has been unintentionally set.
	WDTCSR = (1 << WDCE) | (1 << WDE);                     // Enable configuration change.
	WDTCSR = (1 << WDIF) | (1 << WDIE) |                     // Enable Watchdog Interrupt Mode.
		(1 << WDCE) | (0 << WDE) |                     // Disable Watchdog System Reset Mode if unintentionally enabled.
		(0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (0 << WDP0); // Set Watchdog Timeout period to 1.0 sec.

	wdt_reset();
	interrupts();
}

ISR(WDT_vect) { // Watchdog interrupt. Interrupt driven tick, called every 1s, updates task delays
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) { // For every (valid) task
		if (task_list[i].task != NULL) {
			task_list[i].delay -= 1;  // Update time by 1 tick
		}
	}
}  // After the IRC returns, the CPU runs the mainloop

uint8_t sched_add_task(void (*task)(void), int32_t delay, int32_t looptime) {
	uint8_t i;
	// Add to list
	noInterrupts(); // Atomic access to the task list
	for (i = 0; i < SCHED_MAX_TASKS; i++) {// Find an empty slot
		if (task_list[i].task == NULL) {
			task_list[i].task = task;
			task_list[i].delay = delay;
			task_list[i].looptime = looptime;
			break;
		}
	}
	interrupts(); // End of atomic

	if (i == SCHED_MAX_TASKS) {  // Max tasks limit reached
		return -1;
	}
	return i; // Return task index
}

void sched_mainloop(void) {
	// Run tasks and go to sleep
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	while (1) {
		uint8_t i;
		for (i = 0; i < SCHED_MAX_TASKS; i++) { // For every (valid) task
			if (task_list[i].task != NULL) {
				if (task_list[i].delay <= 0) {  // If it can be run

					task_list[i].task();  // Run task
					noInterrupts(); // Atomic access to the task delay
					task_list[i].delay += task_list[i].looptime;
					interrupts(); // End of atomic
					break;  // Task will be run from MAINLOOP
				}
			}
		}

		sleep_cpu(); // Will wake up every 1s due to the WDT interrupt

		digitalWrite(LED_BUILTIN, HIGH);
		delay(50);
		digitalWrite(LED_BUILTIN, LOW);
		delay(50);
	}
}