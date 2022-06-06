#include "sht3x.h"

#include "core/mpu.h"
#include "twi/wire.h"

CSHT3X::CSHT3X(uint8_t address)
: cTemp(0.0f), humidity(0)
{
	Wire.begin();
	_address = address;
}

uint8_t CSHT3X::get()
{
	unsigned int data[6];

	// Start I2C Transmission
	Wire.beginTransmission(_address);
	// Send measurement command
	Wire.write(0x2C);
	Wire.write(0x06);
	// Stop I2C transmission
	if (Wire.endTransmission() != 0)
		return 1;

	delay(100);

	// Request 6 bytes of data
	Wire.requestFrom(_address, 6);

	// Read 6 bytes of data
	// cTemp msb, cTemp lsb, cTemp crc, humidity msb, humidity lsb, humidity crc
	for (int i = 0; i < 6; i++)
	{
		data[i] = Wire.read();
	};
	
	delay(50);
	
	if (Wire.available() != 0)
		return 2;

	// Convert the data
	cTemp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
	
	humidity = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);
	
	return 0;
}
