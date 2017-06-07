#ifndef _GSM_COMMUNICATION_H_
#define _GSM_COMMUNICATION_H_

#include "Arduino.h"

/*
	GSM Communication library for the Feather FONA board

*/

enum comm_status_code {
	COMM_OK,          // Function executed
	COMM_ERR_RETRY,       // Module unexpected error, retry a few times or abort
	COMM_ERR_RETRY_LATER    // Connection error, connection closed, data discarded, report aborted, module shutdown
};

/*
	Configure Serial and IO pins to operate the communication module. If the module is ON, turn it OFF
	Always returns COMM_OK, no check is performed
*/
enum comm_status_code comm_setup(void);

/*
	Turn on the module, connect to GPRS, start an HTTP session and prepare to send POST data
	Returns COMM_OK if the module is connected and a POST DATA session was open.
	Returns COMM_ERR_RETRY if the boot or some of the issued commands failed.
	Returns COMM_ERR_RETRY_LATER if the timeout of the GPRS subscription was reached
*/
enum comm_status_code comm_start_report(int totallen);

/*
	Send binary data for the POST action
	Always returns COMM_OK, no overflow check is performed
*/
enum comm_status_code comm_fill_report(const uint8_t *buffer, int length);

/*
	Issue the POST action and await for results, then shut down the module
	Returns COMM_OK  on HTTP 200 OK code. Returns COMM_ERR_RETRY on module error. Returns COMM_ERR_RETRY_LATER on any different HTTP code, including timeouts and connection errors
*/
enum comm_status_code comm_send_report(void);

/*
	Stop any on-going opperation and shut down the module. Performs a hardware reset if the module is not responding (might take several seconds)
	Returns COMM_OK if the module was shut down, COMM_ERR_RETRY if the module didn't answer to the shutdown command
*/
enum comm_status_code comm_abort(void);


#endif // !_GSM_COMMUNICATION_H_
