#ifndef _LORA_COMMUNICATION_H_
#define _LORA_COMMUNICATION_H_

// Arduino
#include "Arduino.h"

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

// Radio
#define CFG_eu868
#define CFG_wimod_board
#define CFG_sx1272_radio

//LMIC Library
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>



/*
	Communication library for the Feather FONA GSM board and the Feather LORA

*/

enum comm_status_code {
	COMM_OK,          // Function executed
	COMM_ERR_RETRY,       // Module unexpected error, retry a few times or abort
	COMM_ERR_RETRY_LATER    // Connection error, connection closed, data discarded, report aborted, module shutdown
};

/*
	Configure Serial and IO pins to operate the communication module. If the module is ON, turn it OFF
	Always returns COMM_OK
*/
enum comm_status_code comm_setup(void);
void lora_comm_setup(void);
/*
	Turn on the module, connect to the network, start a session and prepare to send data
	Returns COMM_OK if the module is connected and a data session was open.
	Returns COMM_ERR_RETRY if the boot or some of the issued commands failed.
	Returns COMM_ERR_RETRY_LATER if the timeout of the network subscription was reached
*/
enum comm_status_code comm_start_report(uint16_t totallen);
void lora_comm_report(osjob_t* job);
/*
	Send binary data for the report contents
	Always returns COMM_OK, no overflow check is performed
*/
enum comm_status_code comm_fill_report(const uint8_t *buffer, int length);

/*
	Issue the report and await for results, then shut down the module
	Returns COMM_OK on a successfuly sent report. Returns COMM_ERR_RETRY on module error. Returns COMM_ERR_RETRY_LATER on timeouts and connection errors
*/
enum comm_status_code comm_send_report(void);

/*
	Stop any on-going opperation and shut down the module. Performs a hardware reset if the module is not responding (might take several seconds)
	Returns COMM_OK if the module was shut down, COMM_ERR_RETRY if the module didn't answer to the shutdown command even after reset
*/
enum comm_status_code comm_abort(void);

// LoRa

//int comm_setup(void);
// Setup
static osjob_t inittest;
void initjob(osjob_t* job);
static osjob_t rapport;
void rapportjob(osjob_t* job);

// Blink Job
static osjob_t blinkjob;
void blinkfunc(osjob_t* job);

// Retry_join job
static osjob_t retry_join;
void retry_joinjob(osjob_t* job);

// Sending job
static osjob_t reportjob;

#endif // !_LORA_COMMUNICATION_H_
