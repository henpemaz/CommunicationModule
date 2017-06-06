








#include "task_scheduler.h"


#ifdef __AVR_ATmega32U4__ /* Using ATmega32u4 - GSM module */
#include <avr/power.h>
#include "gsm_communication.h"

#elif __SAMD21G18A__ /* Using SAM-D21 - LORA module*/
#include "lora_communication.h"
#endif

void dummy_func() {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
}
#define stor_setup dummy_func
#define sampling_setup dummy_func
#define reporting_setup dummy_func
#define sampling_task dummy_func
#define reporting_task dummy_func
#define SAMPLING_LOOP_TIME 10
#define REPORTING_LOOP_TIME 60


void setup()
{
	// Board setup
#ifdef __AVR_ATmega32U4__ /* Using ATmega32u4 - GSM module */

	wdt_disable();
	// Shut down unused peripherals
	power_adc_disable();
	power_twi_disable();
	// power_timer0_disable(); // Used by "delay" and "millis", automatically shut down during sleep (except IDLE)
	power_timer1_disable();
	power_timer2_disable();
	power_timer3_disable();
	power_timer4_disable();  // Requires you to patch iom32u4.h -> add (#define PRTIM4 4) and (#define __AVR_HAVE_PRR1_PRTIM4) and add {| (1 << PRTIM4)} to the existing #define __AVR_HAVE_PRR1
	#ifndef (_DEBUG | DEBUG | __DEBUG__)
	power_usb_disable();
	#endif

#elif __SAMD21G18A__ /* Using SAM-D21 - LORA module*/

	// TODO POWER SAVING FEATURES

#endif

	


	// Set up communication storage and box interface

	comm_setup();
	
	stor_setup();

	sampling_setup();

	reporting_setup();

	// Launch tasks
	sched_setup();

	sched_add_task(sampling_task, SAMPLING_LOOP_TIME, SAMPLING_LOOP_TIME);

	sched_add_task(reporting_task, REPORTING_LOOP_TIME, REPORTING_LOOP_TIME);

	sched_mainloop();
}

void loop()
{

}
