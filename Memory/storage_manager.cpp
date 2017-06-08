#include "storage_manager.h"
#include <SPI.h>

// pins
#define DATAOUT MOSI//MOSI
#define DATAIN  MISO//MISO
#define SPICLOCK  SCK//sck
#define SLAVESELECT A5//ss

// opcodes
#define WREN  0x06
#define WRDI  0x04
#define RDSR  0x05
#define WRSR  0x01
#define READ  0x03
#define WRIT  0x02
#define CE    0xC7

// constants
#define WIP_MASK      0x01
#define SAMPLE_SIZE   18      // 18B of data stored each sample
#define PAGE_SIZE     128
#define MEMORY_SIZE   65536   // 64 kB

// variables
uint16_t adr_ecr = 0;
uint16_t adr_lir = 0;
uint16_t adr_lir_committed = 0;

// Prototypes
bool memory_is_busy();
void write_eeprom(uint8_t *data, uint16_t address, uint16_t len);
void write_eeprom_page(uint8_t *data, uint16_t address, uint16_t len);
void read_eeprom(uint8_t *buffer, uint16_t address, uint16_t len);

/*
	Set up memory interface
*/
void stor_setup(void) {
	SPI.begin();

	pinMode(8, OUTPUT);
	digitalWrite(8, HIGH); //disable radio device to avoid conflicts

	pinMode(SLAVESELECT, OUTPUT);
	digitalWrite(SLAVESELECT, HIGH); //disable device

									 //erase device
	stor_erase_eeprom();
}

/*
* Ce fonction va stocker un échantillon de données dans la mémoire à partir de la
* première addresse qui est libre.
*/
void stor_write_sample(uint8_t *data)
{
	write_eeprom(data, adr_ecr, SAMPLE_SIZE);
	adr_ecr += SAMPLE_SIZE;
}

/*
* Ce fonction va lire et retourner les données avec longueur 'len' qui sont stockés
* dans la mémoire à partir de l'adresse 'addresse_lu'.
*/
uint16_t stor_read_sample(uint8_t *buffer, uint16_t maxlen)
{
	uint16_t len = min(stor_available(), maxlen);

	read_eeprom(buffer, adr_lir, len);
	adr_lir += len;
	return len;
}


void stor_confirm_read(bool do_commit)
{
	if (do_commit)
	{
		adr_lir_committed = adr_lir;
	}
	else
	{
		adr_lir = adr_lir_committed;
	}
}


uint16_t stor_available(void)
{
	if (adr_ecr >= adr_lir)
	{
		return adr_ecr - adr_lir;
	}
	else
	{
		return adr_ecr + (MEMORY_SIZE - adr_lir);
	}
}


void stor_erase_eeprom()
{
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(WREN); //write enable
	digitalWrite(SLAVESELECT, HIGH);
	while (memory_is_busy());
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(CE); //write instruction
	digitalWrite(SLAVESELECT, HIGH);
	while (memory_is_busy());
}

bool memory_is_busy()
{
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(RDSR);
	uint8_t status_reg = SPI.transfer(0xFF);
	digitalWrite(SLAVESELECT, HIGH);

	return ((status_reg & WIP_MASK) == 1);
}

/*
* This method will write the data in the buffer 'data' with length 'len' into the memory
* starting from address 'address'. Memory page overflows are handeled in this method.
*/
void write_eeprom(uint8_t *data, uint16_t address, uint16_t len)
{
	while (len > 0)
	{
		if ((address % PAGE_SIZE) + len >= PAGE_SIZE)
		{
			uint16_t len_part = PAGE_SIZE - (address % PAGE_SIZE);
			write_eeprom_page(data, address, len_part);
			len -= len_part;
			address += len_part;
			data += len_part;
		}
		else
		{
			write_eeprom_page(data, address, len);
			len = 0;
		}
	}
}

/*
* This method writes the data 'data' with length 'len' in the memory starting from the given
* address 'address'.
* If input satisfies following condition: (address % PAGE_SIZE) + len <= PAGE_SIGE
* Then the method will execute correctly.
*/
void write_eeprom_page(uint8_t *data, uint16_t address, uint16_t len)
{
	//fill eeprom w/ buffer
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(WREN); //write enable
	digitalWrite(SLAVESELECT, HIGH);
	while (memory_is_busy());
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(WRIT); //write instruction
	SPI.transfer((uint8_t)(address >> 8));   //send MSByte address first
	SPI.transfer((uint8_t)(address));      //send LSByte address
										//write len bytes
	for (uint16_t i = 0; i<len; i++)
	{
		SPI.transfer(data[i]); //write data byte
	}
	digitalWrite(SLAVESELECT, HIGH); //release chip
									 //wait for eeprom to finish writing
	while (memory_is_busy());
}

/*
* This method reads a given number of bytes 'len' from the memory starting from address
* 'address' and stores them in the given data buffer 'buf'.
*
*/
void read_eeprom(uint8_t *buffer, uint16_t address, uint16_t len)
{
	digitalWrite(SLAVESELECT, LOW);
	SPI.transfer(READ); //transmit read opcode
	SPI.transfer((uint8_t)(address >> 8));   //send MSByte address first
	SPI.transfer((uint8_t)(address));      //send LSByte address
	for (uint16_t i = 0; i<len; i++)
	{
		buffer[i] = SPI.transfer(0xFF); //get data byte
	}
	digitalWrite(SLAVESELECT, HIGH);
}