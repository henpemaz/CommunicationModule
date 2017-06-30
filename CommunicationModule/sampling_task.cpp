// 
// 
// 

#define DB_MODULE "Sampling Task"
#include "debug.h"

#include "sampling_task.h"
#include "storage_manager.h"


#define SAMPLE_SIZE 18

// Fills 0 until the 16th, then preamble
const byte msg_header[] = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0xc5,0x6a,0x29};



struct command {
	byte bytes[8]; // The message 'string'
	uint8_t len; // The message lenght
	uint8_t anslen; // The lenght of the expected answer
	uint8_t datapos; // The position of the data on the answer
	uint8_t datalen; // The lenght of the data
};

//const struct command cmd_OPID = { { 0x07, 0x01, 0x0e, 0x9a }, 4, ???};
//const struct command cmd_PPID = { { 0x07, 0x08, 0x14, 0xcb }, 4, ???};
const struct command cmd_PS = { { 0x06, 0x09, 0x57 }, 3, 8, 5, 1 }; // PAYG state
const struct command cmd_OCS = { { 0x06, 0x0a, 0xb5 }, 3, 8, 5, 1 }; // Output state
const struct command cmd_SSC = { { 0x06, 0x0b, 0xeb }, 3, 8, 5, 1 }; // System Status Code
const struct command cmd_RPD = { { 0x06, 0x05, 0xf4 }, 3, 8, 5, 2 }; // Remaining PAYG days
const struct command cmd_RSOC = { { 0x06, 0x0c, 0x68 }, 3, 8, 5, 1 }; // Relative SOC
const struct command cmd_RC = { { 0x06, 0x0d, 0x36 }, 3, 8, 5, 2 }; // Remaining Capaciry
const struct command cmd_FCC = { { 0x06, 0x0e, 0xd4 }, 3, 8, 5, 2 }; // Full Charge Capacity
const struct command cmd_ACC = { { 0x06, 0x0f, 0x8a }, 3, 8, 5, 2 }; // Accumulative energy output
const struct command cmd_AC = { { 0x06, 0x10, 0x56 }, 3, 8, 5, 2 }; // Discharge cycles
const struct command cmd_PD = { { 0x06, 0x07, 0x48 }, 3, 8, 5, 2 }; // Top up days
const struct command cmd_RDB = { { 0x06, 0x013, 0xb4 }, 3, 8, 5, 2 }; // Running days
const struct command cmd_HTOP = { { 0x06, 0x11, 0x08 }, 3, 14, 5, 8 }; // HASH TOP
const struct command cmd_CV1 = { { 0x08, 0x00, 0x3f, 0x02, 0x0b }, 5, 10, 7, 2 };
const struct command cmd_CV2 = { { 0x08, 0x00, 0x3e, 0x02, 0xcf }, 5, 10, 7, 2 };
const struct command cmd_CV3 = { { 0x08, 0x00, 0x3d, 0x02, 0x9a }, 5, 10, 7, 2 };
const struct command cmd_CV4 = { { 0x08, 0x00, 0x3c, 0x02, 0x5e }, 5, 10, 7, 2 };
const struct command cmd_BV = { { 0x08, 0x00, 0x09, 0x02, 0x8c }, 5, 10, 7, 2 };
const struct command cmd_BC = { { 0x08, 0x00, 0x0a, 0x02, 0xd9 }, 5, 10, 7, 2 };
const struct command cmd_BT = { { 0x08, 0x00, 0x08, 0x02, 0x48 }, 5, 10, 7, 2 };
// cmd_passcode write passcode ...



const struct command *msg_commands[] = {
	&cmd_RSOC,
	&cmd_RC,
	&cmd_FCC,
	&cmd_CV1,
	&cmd_CV2,
	&cmd_CV3,
	&cmd_CV4,
	&cmd_BV,
	&cmd_BT,
	&cmd_BC
};

void sampling_setup(void) {
	db("Setup");

}

static uint16_t total_samples = 0;

//inline void get_dummy_data(uint8_t *buffer) {
//	db("Generating dummy data");
//	for (uint8_t i = 0; i < SAMPLE_SIZE; i++) {
//		buffer[i] = '0';
//	}
//	uint8_t i;
//	uint16_t val = total_samples;
//	total_samples++;
//	// Common exception
//	if (val == 0) {
//		return;
//	}
//	i = SAMPLE_SIZE;
//	while (val) {
//		buffer[--i] = 48 + val % 10;  // Fill in each digit (--i happens first, so i still points to the digit when done)
//		val /= 10;
//	}
//}

inline void format_buffer(uint8_t *buffer, uint8_t soc, int bc)
{
	for (int i = 0; i < 18; i++)
		buffer[i] = '0';
	buffer[0] = '#';
	int val = total_samples;
	total_samples++;
	// fill in 1..5 with sample number
	uint8_t i = 6;
	while (val) {
		buffer[--i] = 48 + val % 10;  // Fill in each digit (--i happens first, so i still points to the digit when done)
		val /= 10;
	}
	buffer[6] = ',';
	// fill in 7-8 with SoC level
	val = soc; // from OV box
	i = 9;
	while (val) {
		buffer[--i] = 48 + val % 10;  // Fill in each digit (--i happens first, so i still points to the digit when done)
		val /= 10;
	}
	buffer[9] = 'P'; //  % sign
	buffer[10] = ',';
	// fill in 11-15 with battery current
	int curr = bc; // from OV box
	if (curr < 0) {
		buffer[11] = '-';
		curr = abs(curr);
	}
	else
		buffer[11] = '+';

	i = 16;
	while (curr) {
		buffer[--i] = 48 + curr % 10;  // Fill in each digit (--i happens first, so i still points to the digit when done)
		curr /= 10;
	}
	buffer[16] = 'm';
	buffer[17] = 'A';
	//for(int k=0; k < 18; k++)
	//	Serial.print((char) buffer[k]);
	//Serial.println("");
}

inline void get_data_from_box(uint8_t *buffer) {
	db("Getting data from box");

	// (Re)Configure serial interface to the box
	Serial1.begin(38400);

	// Request and read sample from box	
	// run all sampling commands
	uint8_t recv[32];
	for (int i = 0; i < 32; i++)
		recv[i] = 0;
	//for (int i = 0; i < sizeof(msg_commands); i++) {
	//	// send the message
	//	Serial1.write(msg_header, 19);
	//	Serial1.write(msg_commands[i]->bytes, msg_commands[i]->len);
	//	// get the answer
	//	Serial1.setTimeout(1000);
	//	Serial1.readBytes(recv, msg_commands[i]->anslen);
	//	// copy the date
	//	memcpy(buffer, recv+ msg_commands[i]->datapos, msg_commands[i]->datalen);
	//	// move on
	//	buffer += msg_commands[i]->datalen;
	//}

	Serial1.write(msg_header, 19);
	Serial1.write(msg_commands[0]->bytes, msg_commands[0]->len);
	Serial1.setTimeout(1000);
	Serial1.readBytes(recv, msg_commands[0]->anslen);

	uint8_t soc = recv[5];
	//Serial.print("SOC: ");
	//Serial.println(recv[5]);

	Serial1.write(msg_header, 19);
	Serial1.write(msg_commands[9]->bytes, msg_commands[9]->len);
	Serial1.setTimeout(1000);
	Serial1.readBytes(recv, msg_commands[9]->anslen);

	int bc = (int)((recv[8] << 8) + recv[7]);
	//Serial.print("BC: ");
	//Serial.println(bc);

	format_buffer(buffer, soc, bc);

	//get_dummy_data(buffer);
}

void sampling_task(void) {
	db("Running Sampling task");

	// Start the EEPROM SPI
	stor_start();
	
	// Get sample data
	uint8_t buff[SAMPLE_SIZE];
	get_data_from_box(buff);

	// Store this sample to the external eeprom
	db("Writting sample to database");
	stor_write_sample(buff);

	stor_end();

	// Go back to sleep
	db("end");
	delay(100);
}






























