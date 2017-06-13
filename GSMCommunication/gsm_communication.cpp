// 
// 
// 

#include "gsm_communication.h"
#include <SoftwareSerial.h>

#define DB_MODULE "GSM Comm"
#include "debug.h"




#define SIM_POWER 2
#define SIM_RESET 7
#define SIM_TX 8
#define SIM_RX 9

SoftwareSerial sim_serial = SoftwareSerial(SIM_TX, SIM_RX);

#define SIM_APN "eseye.com"
#define SIM_USER "user"
#define SIM_PWD "pass"

#define POST_URL "posttestserver.com/post.php?dir=hen"


enum comm_status_code power_on(void);
enum comm_status_code power_off(void);
inline enum comm_status_code get_reply(const char * tosend, const char * expected_reply, uint16_t timeout);
enum comm_status_code get_reply(const uint8_t *tosend, const uint8_t *expected_reply, uint16_t timeout);
inline void flush_input(void);

#define UITOA_BUFFER_SIZE 6
void uitoa(uint16_t val, uint8_t *buff);

bool module_is_on;

extern const char ok_reply[] = "\r\nOK\r\n";


enum comm_status_code gsm_comm_setup(void) {
	db("Setup");
	// Configure pins
	digitalWrite(SIM_RESET, HIGH);
	pinMode(SIM_RESET, OUTPUT);

	digitalWrite(SIM_POWER, HIGH);
	pinMode(SIM_POWER, OUTPUT);

	// Start Serial
	sim_serial.begin(9600);

	gsm_comm_abort(); // Force a hardware reset and shut down the module

	flush_input();
	return COMM_OK;
}

inline void flush_input(void) {
	char c;
	delay(100);
	while (sim_serial.available()) {
		c = sim_serial.read();
		db_print(c);
	}
}

#define MAX_AT_TRIES 10 // Each try = 100ms 
enum comm_status_code power_on(void) {
	db("Power on");
	// Power on
	digitalWrite(SIM_POWER, LOW);
	delay(1000);
	digitalWrite(SIM_POWER, HIGH);
	// Open communication
	delay(500); // Module takes on average 2.5s on boot

	uint8_t tries = 0;
	// While not boot timeout
	while (tries < MAX_AT_TRIES) {
		// AT baud synchronism
		flush_input();
		if (get_reply("AT", ok_reply, 100) == COMM_OK) {  // AT OK
			delay(100);
			get_reply("ATE0", ok_reply, 100); // Disable echo
			module_is_on = true;
			return COMM_OK;
		}
		tries++;
	}
	return COMM_ERR_RETRY;
}

enum comm_status_code power_off(void) {
	db("Power off");
	enum comm_status_code code = get_reply("AT+CPOWD=1", "NORMAL POWER DOWN", 2000);
	if (code == COMM_OK) {
		delay(1200);
		flush_input();
		module_is_on = false;
	}
	return code;
}

inline enum comm_status_code get_reply(const char *tosend, const char *expected_reply, uint16_t timeout) {
	return get_reply((const uint8_t *)tosend, (const uint8_t *)expected_reply, timeout);
}

enum comm_status_code get_reply(const uint8_t *tosend, const uint8_t *expected_reply, uint16_t timeout) {
	uint8_t reply_index;
	uint8_t reply;
	reply_index = 0;

	if (tosend) {
		sim_serial.println((const char*)tosend);
		db(String("\n>>>")+((const char*)tosend));		
	}
	db_print("<<<");
	while (timeout > 0) {

		while (sim_serial.available()) {
			reply = sim_serial.read();
			db_print((char)reply);
			if (reply != expected_reply[reply_index]) { // No match, break sequence
				reply_index = 0;
				if (reply != expected_reply[reply_index]) { // No match on new sequence
					continue;
				}
			}
			reply_index++;  // Match
			if (expected_reply[reply_index] == 0x00) {  // End sequence
				db_println(""); // end line
				return COMM_OK;
			}
		}
		delay(1);
		timeout--;
	}
	db_println(""); // end line
	return COMM_ERR_RETRY;
}

enum comm_status_code gsm_comm_start_report(uint16_t totallen) {
	db("Start report");
	uint16_t timeout;
	timeout = 60000;  // timeout for GPRS connection

	// Power on
	if (!module_is_on && power_on() != COMM_OK) return COMM_ERR_RETRY;
	db("Module is on");
	// While not connection timeout
	db("Attempting connection");
	while (timeout > 0) {
		// Querry GPRS availability
		flush_input();
		if (get_reply("AT+CGATT?", "+CGATT: 1", 200) == COMM_OK) {
			break;
		}
		delay(800);
		timeout -= 1000;
	}
	// If timeout
	if (timeout == 0) {
		// Shutdown
		db("Connection timeout");
		power_off();
		return COMM_ERR_RETRY_LATER;
	}

	db("Connection stablished");
	// GPRS available

	delay(1000);
	flush_input(); // Dismiss unrequested messages
	db("Configuring APN");
	if (get_reply("AT+SAPBR=3,1,\"Contype\", \"GPRS\"", ok_reply, 200) != COMM_OK
		|| get_reply("AT+SAPBR=3,1,\"APN\", \"" SIM_APN "\"", ok_reply, 200) != COMM_OK
		|| get_reply("AT+SAPBR=3,1,\"USER\", \"" SIM_USER "\"", ok_reply, 200) != COMM_OK
		|| get_reply("AT+SAPBR=3,1,\"PWD\", \"" SIM_PWD "\"", ok_reply, 200) != COMM_OK
		|| get_reply("AT+SAPBR=1,1", ok_reply, 30000) != COMM_OK) {  // 1.85s max connection bringup time on the specifications, but sometimes...
		db("Failed to configure APN");
		return COMM_ERR_RETRY;
	}
	flush_input();
	db("Configuring HTTP module");
	if (get_reply("AT+HTTPINIT", ok_reply, 200) != COMM_OK
		|| get_reply("AT+HTTPPARA=\"CID\",1", ok_reply, 200) != COMM_OK
		|| get_reply("AT+HTTPPARA=\"URL\",\"" POST_URL "\"", ok_reply, 200) != COMM_OK) {
		db("Failed to configure HTTP module");
		return COMM_ERR_RETRY;
	}
	flush_input();
	db("Starting HTTPDATA session");
	sim_serial.print("AT+HTTPDATA=");  // Start data session
	uint8_t lenght_buffer[UITOA_BUFFER_SIZE];  // Used to store the ASCII representation of totallen
	uitoa(totallen, lenght_buffer); // Custom fixed base uitoa function (see end of file)
	sim_serial.print((char*)lenght_buffer);  // <totallen> bytes to send
	if (get_reply(",60000", "\r\nDOWNLOAD\r\n", 1000) != COMM_OK) {  // POST data transmission, timeout 60 secs
		db("Failed to start DATA session");
		return COMM_ERR_RETRY;
	}
	db("DATA session started");
	return COMM_OK;
}


enum comm_status_code gsm_comm_fill_report(const uint8_t *buffer, int lenght) {
	db("Fill report");
	sim_serial.write(buffer, lenght);  // Write binary data to serial
	db_module(); db_print(lenght); db_println(" bytes of data sent");
	return COMM_OK;
}


enum comm_status_code gsm_comm_send_report(void) {
	db("Send Report");
	flush_input();
	if (get_reply("AT+HTTPACTION=1", ok_reply, 500) != COMM_OK) { // Do POST
		db("POST action failed");
		return COMM_ERR_RETRY;
	}
	if (get_reply(NULL, "+HTTPACTION: 1,", 60000) != COMM_OK) { // Send nothing, wait for the +httaction response
		db("Module did not respond");                            // somehow no http timeout ???
		return COMM_ERR_RETRY;
	}
	db("Got answer from server");
	char http_code[4];
	while (!sim_serial.available())delay(1);
	http_code[0] = sim_serial.read();
	while (!sim_serial.available())delay(1);
	http_code[1] = sim_serial.read();
	while (!sim_serial.available())delay(1);
	http_code[2] = sim_serial.read();

	http_code[3] = 0;
	db_module(); db_print("HTTP code : "); db_println(http_code);

	get_reply("AT+HTTPTERM", ok_reply, 500);  // No error handling
	//get_reply("AT+SAPBR=0,1", ok_reply, 500);  // We don't really have to, happens on shutdown 

	power_off();

	if (http_code[0] == '2' && http_code[2] == '0') { // OK
		return COMM_OK;
	}

	return COMM_ERR_RETRY_LATER;
}


enum comm_status_code gsm_comm_abort(void) {
	db("Abort");
	if (get_reply("AT", ok_reply, 200) != COMM_OK) { // Module stuck
		digitalWrite(SIM_RESET, LOW);// Hardware reset
		delay(200);
		digitalWrite(SIM_RESET, HIGH);
		delay(1200);
		power_on();
		delay(3000);
	}
	return power_off();
}

// Custom uitoa, fixed base, expects buffer of the right size
void uitoa(uint16_t val, uint8_t *buff) {
	uint8_t i;
	// Common exception
	if (val == 0) {
		buff[0] = 48;
		buff[1] = 0;
		return;
	}

	// Do the magic
	i = UITOA_BUFFER_SIZE - 1;
	buff[i] = 0;  // From the tail, we'll go towards the head
	while (val) {
		buff[--i] = 48 + val % 10;  // Fill in each digit (--i happens first, so i still points to the digit when done)
		val /= 10;
	}

	// Left shift the result (remove padding)
	if (i) do { buff[0] = buff[i]; } while (*(buff++) != 0);
}
