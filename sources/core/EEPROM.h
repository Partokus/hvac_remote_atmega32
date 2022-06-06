#ifndef _EEPROM_h_
#define _EEPROM_h_

#include <inttypes.h>
#include <stddef.h>

class CEEPROM
{
public:
	uint8_t read(int address);
	void read(int address, uint8_t* buffer, size_t size);
	void write(int address, uint8_t value);
	void write(int address, const uint8_t* buffer, size_t size);
};

extern CEEPROM EEPROM;

#endif
