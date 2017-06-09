// 
// 
// 

#include "sampling_task.h"
#include "storage_manager.h"


#define SAMPLE_SIZE 18

#define db(val) Serial.println(val)

void sampling_setup(void) {










}

void sampling_task(void) {
	Serial.begin(115200);
	//while (!Serial);
	delay(100);
	db("Running Sampling task");
	// (Re)Configure serial interface to the box
	// TODO

	// Request and read sample from box	
	// TODO

	db("Generating dummy data");
	uint8_t buff[SAMPLE_SIZE];
	for (uint8_t i = 0; i < SAMPLE_SIZE; i++) {
		buff[i] = i + 'a';
	}

	// Store this sample to the external eeprom
	db("Writting sample to database");
	db("Available to read before :");
	db(stor_available());
	stor_write_sample(buff);
	db("Available to read after :");
	db(stor_available());

	delay(100);
}






























