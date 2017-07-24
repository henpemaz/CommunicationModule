#include "lora_communication.h"

/*
Communication library for the Feather FONA GSM board and the Feather LORA
*/

//test data

static char data[] = "Frame Counter : ";

// CONFIGURATION 

// application router ID -> Gateway EUI (little-endian format)
static const u1_t PROGMEM APPEUI[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
void os_getArtEui(u1_t* buf) { memcpy_P(buf, APPEUI, 8); }

// Device EUI (little endian format)
static const u1_t PROGMEM DEVEUI[8] = { 0x07, 0x04, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 };
void os_getDevEui(u1_t* buf) { memcpy_P(buf, DEVEUI, 8); }
devaddr_t DevAddr = 0x74345678;

// Device-specific AES key (big endian format) 
static const u1_t PROGMEM APPKEY[16] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x23, 0x45, 0x67, 0x89, 0x01 };
void os_getDevKey(u1_t* buf) { memcpy_P(buf, APPKEY, 16); }

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

// LMIC Event Callback

void onEvent(ev_t ev) {
	Serial.print(os_getTime());
	Serial.print(": ");
	switch (ev) {
	case EV_JOINING:
		Serial.println(F("EV_JOINING"));
		BLINK_INTERVAL = 500;
		break;
	case EV_JOINED:
		Serial.println(F("EV_JOINED"));
		BLINK_INTERVAL = 1;
    /*
    LMIC.frame[0] = LMIC.snr;
    LMIC_setTxData2(1,LMIC.frame,1,0);
    Serial.println(F("Packet queued"));
    */
    //lora_comm_report(&reportjob);
    os_setCallback(&inittest,initjob);
    break;
	case EV_RFU1:
		Serial.println(F("EV_RFU1 - unhandled event"));
		break;
	case EV_JOIN_FAILED:
		Serial.println(F("EV_JOIN_FAILED"));
		BLINK_INTERVAL = 100;
		os_setTimedCallback(&retry_join, os_getTime() + sec2osticks(RETRY_JOIN_INTERVAL), retry_joinjob);
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
		//lora_comm_report(&reportjob);
   	break;
	default:
		Serial.println(F("Unknown event"));
		break;
	}
}

///// JOBS  /////

void blinkfunc(osjob_t* job)
{
	digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));

	if (BLINK_INTERVAL >= 10)
		os_setTimedCallback(job, os_getTime() + ms2osticks(BLINK_INTERVAL), blinkfunc);
	else
		os_setTimedCallback(job, os_getTime() + sec2osticks(BLINK_INTERVAL), blinkfunc);
}

// Setup

void lora_comm_setup(void)
{
	// Configure pins
	pinMode(LED_BUILTIN, OUTPUT);
	pinMode(lmic_pins.rst, OUTPUT);

	digitalWrite(LED_BUILTIN, HIGH);
	// Start Serial
	while (!Serial);
	Serial.begin(115200);
	delay(1000);
	Serial.println(F("Starting"));

	// Hard-resetting the radio
	digitalWrite(lmic_pins.rst, LOW);
	digitalWrite(LED_BUILTIN, LOW);
	delay(2000);
	digitalWrite(lmic_pins.rst, HIGH);
	digitalWrite(LED_BUILTIN, HIGH);
	delay(2000);

	// LMIC init
	os_init();
	// Reset the MAC state. Session and pending data transfers will be discarded.
	LMIC_reset();

	LMIC_startJoining();

  //LMIC.shutdown() ?? 
}

void initjob(osjob_t* job)
{
  LMIC.frame[0] = LMIC.snr;
  LMIC_setTxData2(1,LMIC.frame,1,0);
  Serial.println(F("Packet queued"));
}

enum comm_status_code comm_setup(void)
{
	if (LMIC.txrxFlags)
	{
		return COMM_OK;
	}
	else
	{
		return COMM_ERR_RETRY;
	}
}

enum comm_status_code code;

void rapportjob(osjob_t* job)
{
  //int code;
  code = comm_setup();
  Serial.println(F(code));
  if (!code)
  {
  os_setTimedCallback(job, os_getTime() + sec2osticks(20), rapportjob);
  }
}
// Fonction ci-dessous pour tester le retour (pas de retour avec comm_status_code sur Arduino IDE)
/*
int comm_setup(void)
{
  if (LMIC.txrxFlags)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}
*/


int retry_join_count = 0;
void retry_joinjob(osjob_t* job) 
{
	retry_join_count++;
	Serial.print(os_getTime());
	Serial.print(": Rerying join operation, attempt Nb: ");
	Serial.println(retry_join_count);
	LMIC_startJoining();
}

// Reporting

uint16_t counter = 0;
void lora_comm_report(osjob_t* job)
{
	// Check if there is not a current TX/RX job running
	if (LMIC.opmode & OP_TXRXPEND)
	{
		Serial.println(F("OP_TXRXPEND, not sending"));
	}
	else
	{
		// Prepare upstream data transmission at the next possible time.
		uint8_t buf[sizeof(data) + 1];
		memcpy(buf, data, sizeof(buf));
		counter++;
		buf[sizeof(buf) - 2] = (counter >> 8);
		buf[sizeof(buf) - 1] = counter;
		LMIC_setTxData2(1, buf, sizeof(buf), 0);
		Serial.println(F("Packet queued"));
	}
	// Next TX is scheduled after TX_COMPLETE event.
}

