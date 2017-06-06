#ifndef _COMMUNICATION_H_
#define _COMMUNICATION_H_

#include "Arduino.h"

enum comm_status_code {
	COMM_OK,          // Function executed
	COMM_ERR_RETRY,       // Module unexpected error, retry a few times or abort
	COMM_ERR_RETRY_LATER    // Connection error, connection closed, data discarded, report aborted, module shutdown
};


///<summary>Configure the communication module. Turn it OFF</summary>
///<returns>Always returns COMM_OK, no check is performed</returns>
enum comm_status_code comm_setup(void);

///<summary>Turn on the module and prepare to send</summary>
///<returns>Returns COMM_OK if the module is connected
///			Returns COMM_ERR_RETRY if the boot or some of the issued commands failed.
///			</returns>
enum comm_status_code comm_start_report(int totallen);

///<summary>Send binary data for the POST action</summary>
///<returns>Always returns COMM_OK, no overflow check is performed</returns>
	enum comm_status_code comm_fill_report(const uint8_t *buffer, int length);

///<summary>Issue the POST action and await for results, then shut down the module</summary>
///<returns>Returns COMM_OK  on HTTP 200 OK code. Returns COMM_ERR_RETRY on module error. Returns COMM_ERR_RETRY_LATER on any different HTTP code, including timeouts and connection errors</returns>
enum comm_status_code comm_send_report(void);

///<summary>Stop any on-going opperation and shut down the module. Performs a hardware reset if the module is not responding (might take several seconds)</summary>
///<returns>Returns COMM_OK if the module was shut down, COMM_ERR_RETRY if the module didn't answer to the shutdown command</returns>
enum comm_status_code comm_abort(void);


#endif // !_COMMUNICATION_H_
