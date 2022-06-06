#include "ht162x.h"

#include "core/mpu.h"

#define COMMAND_MODE 0b100
#define WRITE_MODE 0b101
#define READ_MODE 0b110

#define ADDR_MAX 128

#define TAKE_CS()    digitalWrite(_cs_pin, LOW)
#define RELEASE_CS() digitalWrite(_cs_pin, HIGH)

CHT162x::CHT162x(uint8_t data, uint8_t wr, uint8_t rd, uint8_t cs)
{
	_data_pin = data;
	_wr_pin = wr;
	_rd_pin = rd;
	_cs_pin = cs;
}

bool CHT162x::init()
{
	// set pins as output
	pinMode(_data_pin, OUTPUT);
	pinMode(_wr_pin, OUTPUT);
	pinMode(_rd_pin, OUTPUT);
	pinMode(_cs_pin, OUTPUT);
	
	// set all pins high
	digitalWrite(_data_pin, HIGH);
	digitalWrite(_wr_pin, HIGH);
	digitalWrite(_rd_pin, HIGH);
	digitalWrite(_cs_pin, HIGH);
	
	delay(100);
	
	if (!testMem())
		return false;
	
	memset(0, 0, ADDR_MAX);
	
	sendCommand(SYS_EN);
	sendCommand(LCD_ON);
	
	return true;
}

void CHT162x::sendCommand(uint8_t cmd)
{
	TAKE_CS();
	writeBits(COMMAND_MODE, 3);
	writeBits(cmd, 8);
	writeBits(0, 1); // Last bit - don't care
	RELEASE_CS();
}

uint8_t CHT162x::read(uint8_t address)
{
	uint8_t data;
	
	TAKE_CS();
	writeBits(READ_MODE, 3);
	writeBits(address, 6);
	data = readBits(4);
	RELEASE_CS();
	
	return data;
}

void CHT162x::write(uint8_t address, uint8_t data)
{
	TAKE_CS();
	writeBits(WRITE_MODE, 3);
	writeBits(address, 6);
	writeBits(data, 4);
	RELEASE_CS();
}

void CHT162x::memset(uint8_t address, uint8_t data, uint8_t cnt)
{
	TAKE_CS();
	writeBits(WRITE_MODE, 3);
	writeBits(address, 6);
	
	for (uint8_t i = 0; i < cnt; i++)
		writeBits(data, 4);
	
	RELEASE_CS();
}


uint8_t CHT162x::readBits(uint8_t cnt)
{
	uint8_t data = 0;
	pinMode(_data_pin, INPUT);
	
	while (cnt)
	{
		digitalWrite(_rd_pin, LOW);
		delayMicroseconds(7);
		digitalWrite(_rd_pin, HIGH);
		delayMicroseconds(7);
		data += digitalRead(_data_pin) << (cnt - 1);
		cnt--;
	}
	
	pinMode(_data_pin, OUTPUT);
	
	return data;
}

void CHT162x::writeBits(uint8_t data, uint8_t cnt)
{
	while (cnt)
	{
		digitalWrite(_wr_pin, LOW);
		uint8_t bitval = (data & (1 << (cnt - 1))) ? HIGH : LOW;
		digitalWrite(_data_pin, bitval);
		delayMicroseconds(4);
		digitalWrite(_wr_pin, HIGH);
		delayMicroseconds(4);
		cnt--;
	}
}

bool CHT162x::testMem()
{
	uint8_t test = 10;
	
	write(0x5a, test);
	
	if (read(0x5a) != test)
		return false;
	
	return true;
}
