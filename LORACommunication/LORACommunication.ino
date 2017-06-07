








#include "task_scheduler.h"
#include "lora_communication.h"


void short_blink() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(20);
	digitalWrite(LED_BUILTIN, LOW);
	delay(20);
}
void long_blink() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
}
#define stor_setup long_blink
#define sampling_setup long_blink
#define reporting_setup long_blink
#define sampling_task long_blink
#define reporting_task long_blink
#define SAMPLING_LOOP_TIME 10
#define REPORTING_LOOP_TIME 60

void setup()
{
	// Board setup
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// TODO Board specific POWER SAVING FEATURES
	USBDevice.standby();

	// Set up communication storage and box interface

	comm_setup();

	stor_setup();

	sampling_setup();

	reporting_setup();

	// Launch tasks
	sched_setup();

	sched_add_task(sampling_task, SAMPLING_LOOP_TIME, SAMPLING_LOOP_TIME);

	sched_add_task(reporting_task, REPORTING_LOOP_TIME, REPORTING_LOOP_TIME);

	sched_add_task(short_blink, 1, 1);
	
	sched_mainloop();
}

void loop()
{

}
