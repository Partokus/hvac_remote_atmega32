#ifndef SHT3X_H_
#define SHT3X_H_

#include <inttypes.h>

class CSHT3X
{
public:
	CSHT3X(uint8_t address = 0x44);
	uint8_t get();
	
	float cTemp;
	float humidity;

private:
	uint8_t _address;
};

#endif /* SHT3X_H_ */
