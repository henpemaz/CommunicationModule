#include "lora_communication.h"

void setup() 
{
  // Setup
	lora_comm_setup();  
	os_setCallback(&blinkjob, blinkfunc); 
  os_setCallback(&rapport,rapportjob);
}

void loop() 
{
   os_runloop_once();
}
