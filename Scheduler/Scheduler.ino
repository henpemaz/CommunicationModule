








#include "task_scheduler.h"


void short_blink() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(20);
	digitalWrite(LED_BUILTIN, LOW); // Test to commit
	delay(20);
}
void long_blink() {
	digitalWrite(LED_BUILTIN, HIGH);
	delay(1000);
	digitalWrite(LED_BUILTIN, LOW);
	delay(1000);
}


void setup()
{
	// Board setup
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// Launch tasks
	sched_setup();

	sched_add_task(long_blink, 10, 10);

	sched_add_task(short_blink, 1, 1);

	sched_mainloop();
}

void loop()
{

}
