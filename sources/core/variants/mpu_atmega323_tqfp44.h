#ifndef _mpu_atmega323_tqfp44_h_
#define _mpu_atmega323_tqfp44_h_

#include "../mpu.h"

const uint8_t SCL = 19;
const uint8_t SDA = 20;

const uint8_t PROGMEM pin_to_port_PGM[] = {
	NOT_A_PROGRAMMABLE_PIN,
	PB, // 1 - PB5
	PB,	// 2 - PB6
	PB, // 3 - PB7
	NOT_A_PROGRAMMABLE_PIN,	// 4 - RES
	NOT_A_PROGRAMMABLE_PIN,	// 5 - VCC
	NOT_A_PROGRAMMABLE_PIN, // 6 - GND
	NOT_A_PROGRAMMABLE_PIN, // 7 - X2
	NOT_A_PROGRAMMABLE_PIN, // 8 - X1
	PD, // 9 - PD0(RXD)
	PD,	// 10 - PD1(TXD)
	PD, // 11 - PD2(INT0)
	
	PD,	// 12 - PD3
	PD, // 13 - PD4
	PD, // 14 - PD5
	PD,	// 15 - PD6
	PD,	// 16 - PD7
	NOT_A_PROGRAMMABLE_PIN,	// 17 - VCC
	NOT_A_PROGRAMMABLE_PIN,	// 18 - GND
	PC,	// 19 - PC0(SCL)
	PC, // 20 - PC1(SDA)
	PC, // 21 - PC2(TCK)
	PC, // 22 - PC3(TMS)
	
	PC, // 23 - PC4
	PC, // 24 - PC5
	PC, // 25 - PC6
	PC, // 26 - PC7
	NOT_A_PROGRAMMABLE_PIN, // 27 - AVCC
	NOT_A_PROGRAMMABLE_PIN, // 28 - AGND
	NOT_A_PROGRAMMABLE_PIN, // 29 - AREF
	PA, // 30 - PA7
	PA, // 31 - PA6
	PA, // 32 - PA5
	PA, // 33 - PA4
	
	PA, // 34 - PA3
	PA, // 35 - PA2
	PA, // 36 - PA1
	PA, // 37 - PA0
	NOT_A_PROGRAMMABLE_PIN, // 38 - VCC
	NOT_A_PROGRAMMABLE_PIN, // 39 - GND
	PB, // 40 - PB0
	PB, // 41 - PB1
	PB, // 42 - PB2
	PB, // 43 - PB3
	PB, // 44 - PB4
};

const uint8_t PROGMEM pin_to_bit_mask_PGM[] = {
	NOT_A_PROGRAMMABLE_PIN,
	_BV(5), // 1 - PB5
	_BV(6),	// 2 - PB6
	_BV(7), // 3 - PB7
	NOT_A_PROGRAMMABLE_PIN,	// 4 - RES
	NOT_A_PROGRAMMABLE_PIN,	// 5 - VCC
	NOT_A_PROGRAMMABLE_PIN, // 6 - GND
	NOT_A_PROGRAMMABLE_PIN, // 7 - X2
	NOT_A_PROGRAMMABLE_PIN, // 8 - X1
	_BV(0), // 9 - PD0(RXD)
	_BV(1),	// 10 - PD1(TXD)
	_BV(2), // 11 - PD2(INT0)
	
	_BV(3),	// 12 - PD3
	_BV(4), // 13 - PD4
	_BV(5), // 14 - PD5
	_BV(6),	// 15 - PD6
	_BV(7),	// 16 - PD7
	NOT_A_PROGRAMMABLE_PIN,	// 17 - VCC
	NOT_A_PROGRAMMABLE_PIN,	// 18 - GND
	_BV(0),	// 19 - PC0(SCL)
	_BV(1), // 20 - PC1(SDA)
	_BV(2), // 21 - PC2(TCK)
	_BV(3), // 22 - PC3(TMS)
	
	_BV(4), // 23 - PC4
	_BV(5), // 24 - PC5
	_BV(6), // 25 - PC6
	_BV(7), // 26 - PC7
	NOT_A_PROGRAMMABLE_PIN, // 27 - AVCC
	NOT_A_PROGRAMMABLE_PIN, // 28 - AGND
	NOT_A_PROGRAMMABLE_PIN, // 29 - AREF
	_BV(7), // 30 - PA7
	_BV(6), // 31 - PA6
	_BV(5), // 32 - PA5
	_BV(4), // 33 - PA4
	
	_BV(3), // 34 - PA3
	_BV(2), // 35 - PA2
	_BV(1), // 36 - PA1
	_BV(0), // 37 - PA0
	NOT_A_PROGRAMMABLE_PIN, // 38 - VCC
	NOT_A_PROGRAMMABLE_PIN, // 39 - GND
	_BV(0), // 40 - PB0
	_BV(1), // 41 - PB1
	_BV(2), // 42 - PB2
	_BV(3), // 43 - PB3
	_BV(4), // 44 - PB4
};

const uint16_t PROGMEM port_to_mode_PGM[] = {
	NOT_A_PROGRAMMABLE_PIN,
	(uint16_t) &DDRA,
	(uint16_t) &DDRB,
	(uint16_t) &DDRC,
	(uint16_t) &DDRD,
};

const uint16_t PROGMEM port_to_output_PGM[] = {
	NOT_A_PROGRAMMABLE_PIN,
	(uint16_t) &PORTA,
	(uint16_t) &PORTB,
	(uint16_t) &PORTC,
	(uint16_t) &PORTD,
};

const uint16_t PROGMEM port_to_input_PGM[] = {
	NOT_A_PROGRAMMABLE_PIN,
	(uint16_t) &PINA,
	(uint16_t) &PINB,
	(uint16_t) &PINC,
	(uint16_t) &PIND,
};

uint8_t analogPinToChannel(uint8_t P);

#endif
