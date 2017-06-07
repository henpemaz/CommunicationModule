// 
// 
// 






#include "storage_manager.h"


void setup() {

	while (!Serial);
	Serial.begin(115200);
	
	while (Serial.available() == 0);
	Serial.println("Begin");

	stor_setup();
}

void loop() {

	int maxlen = 148;
	int address = 0;
	byte buf[148];
	for (int I = 0; I<maxlen; I++)
	{
		buf[I] = 0;
	}

	int len = stor_read_sample(buf, maxlen);
	print_samples(buf, len);

	byte sample[18];
	for (int I = 0; I<18; I++)
	{
		sample[I] = I;
	}
	for (int I = 0; I<15; I++)
	{
		stor_write_sample(sample);
	}

	len = stor_read_sample(buf, maxlen);
	print_samples(buf, len);

	erase_eeprom();
	len = stor_read_sample(buf, maxlen);
	print_samples(buf, len);

	// end of execution
	while (1) {}
}

void print_samples(byte *buf, int len)
{
	Serial.print("Length: ");
	Serial.println(len);
	for (int adr = 0; adr < len; adr++)
	{
		Serial.print(buf[adr], DEC);
		Serial.print(" ");
	}
	Serial.print('\n');
}
