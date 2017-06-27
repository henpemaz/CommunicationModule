



#include "communication.h"

#define DB_MODULE "LORA Communication"
#include "debug.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif

// This EUI must be in little-endian format, so least-significant-byte
// first. 
static const u1_t PROGMEM APPEUI[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8] = { 0x07, 0x04, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). 
static const u1_t PROGMEM APPKEY[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01 };
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

static char mydata[] = "Frame Counter : ";
static osjob_t sendjob;
static osjob_t blinkjob;
static osjob_t retry_joinjob;

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 60;

// If JOIN failed, retry JOIN after this many seconds
const unsigned RETRY_JOIN_INTERVAL = 120;

// interval with which the LED blinks in seconds
// used to give information about the LoRa state
// 5 s    - default
// 500 ms - LoRa module trying to join network
// 1 s    - LoRa module successfully joined network
// 100 ms - LoRa module not joined network after retrying.
uint8_t BLINK_INTERVAL = 5;

// Pin mapping
const lmic_pinmap lmic_pins = {
	.nss = 8,
	.rxtx = LMIC_UNUSED_PIN,
	.rst = 4,
	.dio = { 3,SDA,6 },
};

void onEvent(ev_t ev) {
	Serial.print(os_getTime());
	Serial.print(": ");
	switch (ev) {
	case EV_SCAN_TIMEOUT:
		Serial.println(F("EV_SCAN_TIMEOUT - unhandled event"));
		break;
	case EV_BEACON_FOUND:
		Serial.println(F("EV_BEACON_FOUND - unhandled event"));
		break;
	case EV_BEACON_MISSED:
		Serial.println(F("EV_BEACON_MISSED - unhandled event"));
		break;
	case EV_BEACON_TRACKED:
		Serial.println(F("EV_BEACON_TRACKED - unhandled event"));
		break;
	case EV_JOINING:
		Serial.println(F("EV_JOINING"));
		BLINK_INTERVAL = 500;
		break;
	case EV_JOINED:
		Serial.println(F("EV_JOINED"));
		BLINK_INTERVAL = 1;
		os_setCallback(&sendjob, do_send);
		break;
	case EV_RFU1:
		Serial.println(F("EV_RFU1 - unhandled event"));
		break;
	case EV_JOIN_FAILED:
		Serial.println(F("EV_JOIN_FAILED"));
		BLINK_INTERVAL = 100;
		os_setTimedCallback(&retry_joinjob, os_getTime() + sec2osticks(RETRY_JOIN_INTERVAL), retry_join);
		break;
	case EV_REJOIN_FAILED:
		Serial.println(F("EV_REJOIN_FAILED - unhandled event"));
		break;
	case EV_TXCOMPLETE:
		Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
		if (LMIC.txrxFlags & TXRX_ACK)
			Serial.println(F("Received ack"));
		if (LMIC.dataLen) {
			Serial.println(F("Received "));
			Serial.println(LMIC.dataLen);
			Serial.println(F(" bytes of payload"));
		}
		// Schedule next transmission
		os_setCallback(&sendjob, do_send);
		break;
	case EV_LOST_TSYNC:
		Serial.println(F("EV_LOST_TSYNC - unhandled event"));
		break;
	case EV_RESET:
		Serial.println(F("EV_RESET - unhandled event"));
		break;
	case EV_RXCOMPLETE:
		// data received in ping slot
		Serial.println(F("EV_RXCOMPLETE - unhandled event"));
		break;
	case EV_LINK_DEAD:
		Serial.println(F("EV_LINK_DEAD - unhandled event"));
		break;
	case EV_LINK_ALIVE:
		Serial.println(F("EV_LINK_ALIVE - unhandled event"));
		break;
	default:
		Serial.println(F("Unknown event"));
		break;
	}
}
uint16_t counter = 0;

void do_send(osjob_t* j) {
	// Check if there is not a current TX/RX job running
	if (LMIC.opmode & OP_TXRXPEND) {
		Serial.println(F("OP_TXRXPEND, not sending"));
	}
	else {
		// Prepare upstream data transmission at the next possible time.
		uint8_t buf[sizeof(mydata) + 1];
		memcpy(buf, mydata, sizeof(buf));
		counter++;
		buf[sizeof(buf) - 2] = (counter >> 8);
		buf[sizeof(buf) - 1] = counter;
		LMIC_setTxData2(1, buf, sizeof(buf), 0);
		Serial.println(F("Packet queued"));
	}
	// Next TX is scheduled after TX_COMPLETE event.
}


int count = 0;

void blink(osjob_t* job)
{
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
	//	Serial.print(mydata);
	//	Serial.println(count);
	//	count++;

	if (BLINK_INTERVAL >= 10)
		os_setTimedCallback(job, os_getTime() + ms2osticks(BLINK_INTERVAL), blink);
	else
		os_setTimedCallback(job, os_getTime() + sec2osticks(BLINK_INTERVAL), blink);
}

int retry_join_count = 0;

void retry_join(osjob_t* job)
{
	retry_join_count++;
	Serial.print(os_getTime());
	Serial.print(": Rerying join operation, attempt Nb: ");
	Serial.println(retry_join_count);
	LMIC_startJoining();
}

// initial job 
static void initfunc(osjob_t* j)
{
	// reset MAC state 
	LMIC_reset();

	// start joining 
	LMIC_startJoining();

	// init done - onEvent() callback will be invoked... 
}

void setup()
{
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	//	while (!Serial);
	Serial.begin(115200);
	delay(1000);
	Serial.println(F("Starting"));

	osjob_t initjob;

	//  initialize run-time env 
	os_init();

	// setup LED blink job
	os_setCallback(&blinkjob, blink);

	//  setup initial job 
	os_setCallback(&initjob, initfunc);

	//  execute scheduled jobs and events 
	os_runloop();

	//  (not reached) 
}


void loop() {
	// never reached
}
