








#include "task_scheduler.h"
#include "lora_communication.h"


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


	// TODO Board specific POWER SAVING FEATURES



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
