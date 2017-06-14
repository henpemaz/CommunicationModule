// 
// 
// 

#define DB_MODULE "Reporting Task"
#include "debug.h"

#include "reporting_task.h"
#include "storage_manager.h"


void reporting_setup(void){
	db("Setup");
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
		Try once more in 30s
		Still not ? Abort and reschedule
	If other errors: Retry once more
	If success: Commit read head
*/
void reporting_task(void){
	// Wake up stuff
	stor_start();

	// Query data from memory
	uint16_t totallen = 0;
	// If not enough samples available, break out (back to sleep)
	// Start Comm session
	// If connection error: Schedule task for later (unread samples)
	// If module error: Try ONCE more (then unread samples)
	// While enough samples:
	//	Fetch a reasonable amount of samples
	//	Fill in Comm report
	// Dispatch report
	// If connection error:
	//	Try once more in 30s
	//	Still not ? Abort and reschedule
	// If other errors: Retry once more
	// If success: Commit read head
}
