








#include "task_scheduler.h"


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


void setup()
{
	// Board setup
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, LOW);

	// TODO Board specific POWER SAVING FEATURES
#ifdef __SAMD21G18A__
	USBDevice.standby();
#endif
#ifdef __AVR_ATmega32U4__
	wdt_disable();
	// Shut down unused peripherals
	power_adc_disable();
	power_twi_disable();
	// power_timer0_disable(); // Used by "delay" and "millis", automatically shut down during sleep (except IDLE)
	power_timer1_disable();
	power_timer2_disable();
	power_timer3_disable();
	power_timer4_disable();  // Requires you to patch iom32u4.h -> add (#define PRTIM4 4) and (#define __AVR_HAVE_PRR1_PRTIM4) and add {| (1 << PRTIM4)} to the existing #define __AVR_HAVE_PRR1
	power_usb_disable();
#endif

	// Launch tasks
	sched_setup();

	sched_add_task(long_blink, 10, 10);

	sched_add_task(short_blink, 1, 1);

	sched_mainloop();
}

void loop()
{

}
