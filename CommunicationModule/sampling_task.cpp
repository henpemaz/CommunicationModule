// 
// 
// 

#include "sampling_task.h"
#include "storage_manager.h"


#define SAMPLE_SIZE 18

#ifdef _DEBUG
#define db(val) Serial.print("sampling: ") + Serial.println(val)
#else
#define db(val) 
#endif

void sampling_setup(void) {
	db("Setup");



}

inline void get_data_from_box(uint8_t *buffer) {
	db("Getting data from box");

	// (Re)Configure serial interface to the box
	// TODO

	// Request and read sample from box	
	// TODO

	db("Generating dummy data");
	for (uint8_t i = 0; i < SAMPLE_SIZE; i++) {
		buffer[i] = i + 'a';
	}
}

void sampling_task(void) {
	//while (!Serial);
	Serial.begin(115200);
	delay(100);
	db("Running Sampling task");

	uint8_t buff[SAMPLE_SIZE];
	get_data_from_box(buff);

	// Start the EEPROM
	stor_start();
	// Store this sample to the external eeprom
	db("Writting sample to database");
	stor_write_sample(buff);

	stor_end();

	delay(100);

	// Go back to sleep
	db("end");
}






























