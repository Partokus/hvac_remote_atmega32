#include "EEPROM.h"

#include <avr/eeprom.h>

uint8_t CEEPROM::read(int address)
{
	return eeprom_read_byte((unsigned char*)address);
}

void CEEPROM::read(int address, uint8_t* buffer, size_t size)
{
	for (size_t i = 0; i < size; ++i)
		buffer[i] = read(address + i);
}

void CEEPROM::write(int address, uint8_t value)
{
	eeprom_write_byte((unsigned char*)address, value);
}

void CEEPROM::write(int address, const uint8_t* buffer, size_t size)
{
	for (size_t i = 0; i < size; ++i)
		write(address + i, buffer[i]);
}

CEEPROM EEPROM;
