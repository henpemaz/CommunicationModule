// 
// 
// 

#define DB_MODULE "Reporting Task"
#include "debug.h"

#include "task_scheduler.h"
#include "reporting_task.h"
#include "storage_manager.h"
#include "communication.h"

uint8_t connection_retries;

void reporting_setup(void){
	db("Setup");

	connection_retries = 0;
}

/*
	Query data from memory
	If not enough samples available, break out (back to sleep)
	Start Comm session
	If connection error: Schedule task for later (unread samples)
	If module error: Try ONCE more (then unread samples)
	While enough samples:
		Fetch a reasonable amount of samples
		Fill in Comm report
	Dispatch report
	If connection error:
		Try again
		Still not ? Abort and reschedule
	If other errors: Retry once more
	If success: Commit read head
*/
void reporting_task(void){
	// Wake up stuff
	db_start();
	stor_start();

	// Query data from memory
	db("querrying samples");
	uint16_t totallen = stor_available();
	// If not enough samples available, break out (back to sleep)
	if (totallen == 0) {
		db("not enough samples, abort");
		stor_end();
		return;
	}
	// If too many samples, trim and signal that the task has to be re-run later
	bool samples_remaining = false;
	if (totallen > MAX_SAMPLES_PER_REPORT) {
		db("too many samples, trim");
		totallen = MAX_SAMPLES_PER_REPORT;
		samples_remaining = true;
	}
	db("got amount of samples");
	// Start Comm session
	comm_status_code code;
	uint8_t tries = 0;
	while (tries < START_COMM_MAX_RETRIES) {
		db("attempting to start report");
		code = comm_start_report(totallen);
		// If connection error: 
		if (code == COMM_ERR_RETRY_LATER) { // If not already too many retries: Reschedule task for later
			db("connection failed");
			if (connection_retries < RETRY_CONNECTION_MAX_TRIES) {
				db("rescheduling");
				sched_add_task(reporting_task, RETRY_CONNECTION_TIME, 0);
				connection_retries++;
			}// Else: nothing special, the task will be run again soon anyways...
			stor_end();
			//comm_abort(); RETRY_LATER shuts down the module already
			return;
		}
		// If module error: Try a few more times and die
		if (code == COMM_ERR_RETRY) {
			db("module error");
			tries++;
			continue;
		}
		// Else: We did it !
		break;
	}
	if (tries == START_COMM_MAX_RETRIES) { // Failed to start report.
		db("reached max retries on starting module, aborting");
		comm_abort();
		stor_end();
		return;
	}
	db("module connected");
	connection_retries = 0; // Actually connected, for real

	// Fill in the samples
	uint8_t buffer[FETCH_BUFFER_MAX_SIZE];
	while (totallen != 0) {
	//	Fetch a reasonable amount of samples
		db("reading samples from memory");
		uint16_t fetchlen = FETCH_BUFFER_MAX_SIZE;
		if (fetchlen > totallen) {
			fetchlen = totallen;
		}
		stor_read_sample(buffer, fetchlen);
		// These storage functions have such useless returns values...
		// TODO how to tell if it's a OK_READ ?
		totallen -= fetchlen;

	//	Fill in Comm report
		comm_fill_report(buffer, fetchlen);
	}
	// Dispatch report
	db("sending report");
	tries = 0;
	while (tries < START_COMM_MAX_RETRIES) {
		code = comm_send_report();
		// If connection error:
		if (code == COMM_ERR_RETRY_LATER) {
			db("connection error, retrying in 10s");
			//	Try once more in 10s
			delay(10000);
			tries++;
			continue;
		}
		// If other errors: Retry once more
		if (code == COMM_ERR_RETRY) {
			db("module error, retrying");
			tries++;
			continue;
		}
		// else: We did it!
		db("report sent");
		break;
	}
	if (tries == START_COMM_MAX_RETRIES) { // Failed to send report.
		db("reached max retries on sending report, aborting");
		comm_abort();
		stor_confirm_read(false);
		stor_end();
		// We *could* re-schedule it...
		// nah
		return;
	}
	// success: Commit read head
	// Communication closed on successful send_report :)
	db("confirming read data");
	stor_confirm_read(true);
	stor_end();

	if (samples_remaining) {
		db("there are samples remaining, scheduling extra job");
		sched_add_task(reporting_task, RETRY_CONNECTION_TIME, 0);
	}
}
