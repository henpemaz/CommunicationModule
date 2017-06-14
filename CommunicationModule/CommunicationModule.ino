#include <SPI.h>
#include <SoftwareSerial.h>


#define DB_MODULE "Communication Module"
#include "debug.h"


// NOTE : Define either GSM or LORA on the project settings !

#ifdef GSM
#ifndef __AVR_ATmega32U4__
#error Wrong board !
#endif
#endif

#ifdef LORA
#ifndef __SAMD21G18A__
#error Wrong board !
#endif
#endif

#include "communication.h"

#include "task_scheduler.h"
#include "storage_manager.h"

#include "sampling_task.h"
#include "reporting_task.h"


///////////// DEBUG
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
///////////// \DEBUG

void setup()
{
	db_start();

	db("Starting");
	db("gsm setup");
	comm_setup();

	db("storage setup");
	stor_setup();

	db("sampling setup");
	sampling_setup();

	db("reporting setup");
	reporting_setup();

	// Launch tasks
	db("scheduler setup");
	delay(100);
	sched_setup();

	db("scheduler add task");
	sched_add_task(sampling_task, SAMPLING_LOOPTIME, SAMPLING_LOOPTIME);

	db("scheduler add task");
	sched_add_task(reporting_task, REPORTING_LOOPTIME, REPORTING_LOOPTIME);

	db("scheduler mainloop");
	delay(100);
	sched_mainloop();
}

void loop()
{

}
