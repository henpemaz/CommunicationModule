







#define __GSM_TEST__
#include "gsm_communication.h"



#define db(str) Serial.print(str);

void setup() {
	const char report_string[] = "Hello World, this is FONA";

	// put your setup code here, to run once:
	Serial.begin(115200);
	while (!Serial);

	db("\nCommunication module test\n");

	enum comm_status_code code = comm_setup();

	db("\nsetup return code :");
	db(code);
	db("\n");

	delay(1000);

	if (get_reply("AT", ok_reply, 100) == COMM_OK) {
		db("\ngot repply\n");
	}
	else {
		db("\nno reply\n");
	}

	db("\npower on\n");
	code = power_on();
	db("\npower on return code :");
	db(code);
	db("\n");

	delay(1000);

	if (get_reply("AT", ok_reply, 100) == COMM_OK) {
		db("\ngot repply\n");
	}
	else {
		db("\nno reply\n");
	}

	db("\ncomm abort\n");
	code = comm_abort();
	db("\n comm abort return code :");
	db(code);
	db("\n");

	while (1);

	db("\npower off\n");
	code = power_off();
	db("\npower off return code :");
	db(code);
	db("\n");

	unsigned long report_start_time = millis();

	db("\nstart report\n");
	code = comm_start_report(strlen(report_string));
	db("\nstart report return code :");
	db(code);
	db("\n");

	unsigned long report_fill_time = millis();

	db("\nfill report\n");
	code = comm_fill_report((const uint8_t *)report_string, strlen(report_string));
	db("\nfill return code :");
	db(code);
	db("\n");

	unsigned long report_send_time = millis();

	db("\nsend report\n");
	code = comm_send_report();
	db("\nsend report return code :");
	db(code);
	db("\n");

	unsigned long report_end_time = millis();

	db("\n ------- Time to start report: ");
	db(report_fill_time - report_start_time);
	db("\n");

	db("\n ------- Time to fill report: ");
	db(report_send_time - report_fill_time);
	db("\n");

	db("\n ------- Time to send report: ");
	db(report_end_time - report_send_time);
	db("\n");

	db("\n ------- Time to perform all operations: ");
	db(report_end_time - report_start_time);
	db("\n");

}

void loop() {
	// put your main code here, to run repeatedly:
	;
}