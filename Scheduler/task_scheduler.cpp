#include "task_scheduler.h"

#ifdef __AVR_ATmega32U4__ /* Using ATmega32u4 - GSM module - Watchdog used for sleep management*/
#include <avr/wdt.h>
#include <avr/sleep.h>

#elif __SAMD21G18A__ /* Using SAM-D21 - LORA module - RTC used for sleep management*/
#include <samd.h>
// cheat with the macros
#define SLEEP_MODE_PWR_DOWN 0
#define set_sleep_mode(...)   SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk
#define sleep_enable(...) 
#define sleep_cpu(...); __DSB();__WFI();

#endif

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
inline void tick_callback(void);

void sched_setup(void) {
	// Structure setup
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) {
		task_list[i].task = NULL;
	}

	// Clocks setup
#ifdef __AVR_ATmega32U4__ /* Using ATmega32u4 - GSM module */
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
	tick_callback();
}  // After the IRC returns, the CPU runs the mainloop

#elif __SAMD21G18A__
	// Using the RTC module

	// Enable clock source
	PM->APBAMASK.reg |= PM_APBAMASK_RTC;
	SYSCTRL->XOSC32K.reg = //SYSCTRL_XOSC32K_ONDEMAND | // Avoid unecessary syncs
		SYSCTRL_XOSC32K_RUNSTDBY |
		//SYSCTRL_XOSC32K_EN32K |  // Output not being routed anywhere
		SYSCTRL_XOSC32K_XTALEN |
		SYSCTRL_XOSC32K_STARTUP(2) | // We can use a very short startup, precision is not an issue
		SYSCTRL_XOSC32K_ENABLE;

	// Configure GCLK2
	GCLK->GENDIV.reg = GCLK_GENDIV_ID(2) | GCLK_GENDIV_DIV(4); // With DIVSEL the division factor is 2^(DIV + 1), so we use 2^5 for a 32 factor (1khz out)
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
	GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_XOSC32K | GCLK_GENCTRL_ID(2) | GCLK_GENCTRL_DIVSEL);
	while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY);
	GCLK->CLKCTRL.reg = (uint32_t)((GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK2 | (RTC_GCLK_ID << GCLK_CLKCTRL_ID_Pos)));
	while (GCLK->STATUS.bit.SYNCBUSY);
	
	// Reseting the module
	// Using MODE 1 for set period overflow
	RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_SWRST; // software reset and disable
	while (RTC->MODE1.STATUS.bit.SYNCBUSY);

	RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_MODE_COUNT16 // 16bit counter mode
		| RTC_MODE1_CTRL_PRESCALER_DIV1024; // 1hz count
	RTC->MODE1.PER.reg |= RTC_MODE1_PER_PER(1); // count to 1 at 1hz for 1s period

	RTC->MODE1.INTENSET.reg |= RTC_MODE1_INTENSET_OVF; // enable overflow interrupt
	RTC->MODE1.INTENCLR.reg |= RTC_MODE1_INTENCLR_CMP0 | RTC_MODE1_INTENCLR_CMP1 | RTC_MODE1_INTENCLR_SYNCRDY; // Disable other interrupts

	NVIC_EnableIRQ(RTC_IRQn); // enable RTC interrupt 
	NVIC_SetPriority(RTC_IRQn, 0x00);

	RTC->MODE1.CTRL.reg |= RTC_MODE1_CTRL_ENABLE; // enable RTC
	while (RTC->MODE1.STATUS.bit.SYNCBUSY);
}

void RTC_Handler(void)
{
	tick_callback();

	RTC->MODE1.INTFLAG.reg = RTC_MODE1_INTFLAG_OVF; // must clear flag at end
}
#endif

inline void tick_callback(void) {
	uint8_t i;
	for (i = 0; i < SCHED_MAX_TASKS; i++) { // For every (valid) task
		if (task_list[i].task != NULL) {
			task_list[i].delay -= 1;  // Update time by 1 tick
		}
	}
}


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