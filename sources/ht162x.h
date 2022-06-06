#ifndef _ht162x_h_
#define _ht162x_h_

#include <inttypes.h>

class CHT162x
{
public:
	enum
	{
		SYS_DIS   = 0b00000000,
		SYS_EN    = 0b00000001,
		LCD_OFF   = 0b00000010,
		LCD_ON    = 0b00000011,
		TIMER_DIS = 0b00000100,
		WDT_DIS   = 0b00000101,
		TIMER_EN  = 0b00000110,
		WDT_EN    = 0b00000111,
		TONE_OFF  = 0b00001000,
		TONE_ON   = 0b00001001,
		
		// Set bias to 1/2 or 1/3 cycle
		// Set to 2,3 or 4 connected COM lines
		BIAS_HALF_2_COM  = 0b00100000,
		BIAS_HALF_3_COM  = 0b00100100,
		BIAS_HALF_4_COM  = 0b00101000,
		BIAS_THIRD_2_COM = 0b00100001,
		BIAS_THIRD_3_COM = 0b00100101,
		BIAS_THIRD_4_COM = 0b00101001,
		
		// Don't use
		TEST_ON   = 0b11100000,
		TEST_OFF  = 0b11100011
	} Commands;
	
	CHT162x(uint8_t data, uint8_t wr, uint8_t rd, uint8_t cs);
	
	bool init();
	
	void sendCommand(uint8_t cmd);
	
	uint8_t read(uint8_t address);
	
	void write(uint8_t address, uint8_t data);
	
	void memset(uint8_t address, uint8_t data, uint8_t cnt);
	
protected:
	uint8_t _data_pin;
	uint8_t _wr_pin;
	uint8_t _rd_pin;
	uint8_t _cs_pin;
	
private:
	uint8_t readBits(uint8_t cnt);
	
	void writeBits(uint8_t data, uint8_t cnt);
	
	bool testMem();
};

#endif
