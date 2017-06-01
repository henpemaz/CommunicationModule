








#include "gsm_communication.h"
#include "task_scheduler.h"

#include <avr/sleep.h>
#include <avr/power.h>





void setup()
{
	// Basic board setup
	// Shut down unused peripherals
	power_adc_disable();
	power_twi_disable();
	power_timer1_disable();
	power_timer2_disable();
	power_timer3_disable();
	// power_timer4_disable(); // ???
	#ifndef _DEBUG
	power_usb_disable();
	#endif


	// Set up communication storage and box interface

	comm_setup();
	
	stor_setup();

	box_setup();

	// Launch tasks
	sched_setup();

	sched_add_task(sampling_task, SAMPLING_LOOP_TIME, SAMPLING_LOOP_TIME);

	sched_add_task(reporting_task, REPORTING_LOOP_TIME, REPORTING_LOOP_TIME);

	// Go to sleep

	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_enable();
	while (1) {
		sleep_cpu();
	}
}

void loop()
{

}
