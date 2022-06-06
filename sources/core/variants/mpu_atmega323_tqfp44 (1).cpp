#include "mpu_atmega323_tqfp44.h"

uint8_t analogPinToChannel(uint8_t P)
{
	if (P >= 30 && P <= 37)
		return ( 37 - P ); // 30 - ADC7, 31 - ADC6, ..., 37 - ADC0
	return P;
}
