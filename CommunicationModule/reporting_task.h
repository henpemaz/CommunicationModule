// reporting_task.h

#ifndef _REPORTING_TASK_h
#define _REPORTING_TASK_h

#include "arduino.h"

#define REPORTING_LOOPTIME  100

#define MAX_SAMPLES_PER_REPORT 65536u

#define START_COMM_MAX_RETRIES 5

#define RETRY_CONNECTION_MAX_TRIES 5
#define RETRY_CONNECTION_TIME REPORTING_LOOPTIME/(RETRY_CONNECTION_MAX_TRIES + 1)

#define FETCH_BUFFER_MAX_SIZE 512u


void reporting_setup(void);

void reporting_task(void);



#endif

