#include <SPI.h>
#include "sampling_task.h"
#include <SoftwareSerial.h>


#define DB_MODULE "Communication Module"
#include "debug.h"


// NOTE : Define either GSM or LORA on the project settings !

#ifdef GSM
#ifndef __AVR_ATmega32U4__
#error Wrong board !
#endif

#include "gsm_communication.h"

#endif

#ifdef LORA
#ifndef __SAMD21G18A__
#error Wrong board !
#endif

#include "lora_communication.h"

#endif


#include "task_scheduler.h"
#include "storage_manager.h"

#include "sampling_task.h"

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

//#define gsm_comm_setup dummy_func
//#define stor_setup dummy_func
#define reporting_setup dummy_func
#define reporting_task dummy_func

#define SAMPLING_LOOP_TIME 10
#define REPORTING_LOOP_TIME 30
///////////// \DEBUG

void setup()
{
	db_start();

	db("Starting");
	db("gsm setup");
	gsm_comm_setup();

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
	sched_add_task(sampling_task, SAMPLING_LOOP_TIME, SAMPLING_LOOP_TIME);

	db("scheduler add task");
	sched_add_task(reporting_task, REPORTING_LOOP_TIME, REPORTING_LOOP_TIME);

	db("scheduler mainloop");
	delay(100);
	sched_mainloop();
}

void loop()
{

}
