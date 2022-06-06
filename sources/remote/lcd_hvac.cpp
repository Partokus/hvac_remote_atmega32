#include "lcd_hvac.h"

#include "..\core\wiring.h"
#include "config.h"

CLCD_HVAC::CLCD_HVAC(uint8_t data, uint8_t wr, uint8_t rd, uint8_t cs)
: CHT162x(data, wr, rd, cs)
{
	for (uint8_t i = 0; i < DISPLAY_MEMORY_SIZE; ++i)
		_displayMemory[i] = 0;
}

void CLCD_HVAC::clear()
{
	for (uint8_t i = 0; i < DISPLAY_MEMORY_SIZE; ++i)
		_displayMemory[i] = 0;
		
	memset(0, 0, DISPLAY_MEMORY_SIZE);
}

void CLCD_HVAC::turnOn()
{
	sendCommand(CHT162x::SYS_EN);
	sendCommand(CHT162x::LCD_ON);
}

void CLCD_HVAC::turnOff()
{
	sendCommand(CHT162x::SYS_DIS);
	sendCommand(CHT162x::LCD_OFF);
}

void CLCD_HVAC::setDigit(uint8_t num, uint8_t value)
{
	--num;
	
	uint8_t segments = pgm_read_byte(segment_char_map_PGM + value);
	
	for (uint8_t i = 0; i < 7; ++i)
	{
		setSegment(pgm_read_byte(digit_segments_PGM + num * 7 + i), segments & 1);
		
		segments >>= 1;
	}
}

void CLCD_HVAC::setSegment(uint8_t segment, uint8_t value)
{
	uint8_t address = ADDRESS(segment);
	uint8_t bitmask = BITMASK(segment);
	
	if (value == 0)
	{
		_displayMemory[address] &= ~bitmask;
	}
	else
	{
		_displayMemory[address] |= bitmask;
	}
	
	write(address, _displayMemory[address]);
}

void CLCD_HVAC::setAll()
{
	for (uint8_t i = 0; i < DISPLAY_MEMORY_SIZE; ++i)
		_displayMemory[i] = 0xFF;
	
	memset(0, 0xFF, DISPLAY_MEMORY_SIZE);
}

void CLCD_HVAC::setChar(int8_t _char, bool visible)
{
	switch (_char)
	{
		case CHAR_HUB:
			setSegment(LCD_T1, visible);
			break;
		case CHAR_SEARCH:
			setSegment(LCD_T2, visible);
			break;
		case CHAR_NIGHT:
			setSegment(LCD_T10, visible);
			break;
		case CHAR_AUTO:
			setSegment(LCD_T11, visible);
			break;
		case CHAR_BREEZER_ERR:
			setSegment(LCD_Z1, visible);
			setSegment(LCD_Z5, visible);
			break;
		case CHAR_HUMIDIFIER_ERR:
			setSegment(LCD_Z2, visible);
			setSegment(LCD_Z6, visible);
			break;
		case CHAR_COND_ERR:
			setSegment(LCD_Z3, visible);
			setSegment(LCD_Z7, visible);
			break;
		case CHAR_ERR:
			setSegment(LCD_Z4, visible);
			break;
		case CHAR_DUCT:
			setSegment(LCD_T3, visible);
			setSegment(LCD_T4, visible);
			break;
	}
}

void CLCD_HVAC::setDecoration(bool visible)
{
	setSegment(LCD_S1, visible);
	setSegment(LCD_S11, visible);
}


void CLCD_HVAC::powerOff()
{
	//for (uint8_t i = 0; i < DISPLAY_MEMORY_SIZE; ++i)
	
	/*
	memset(0, 0, 1); // address, data, cnt
	_displayMemory[0] = 0;
	
	
	memset(1, 0b00000001, 1);
	_displayMemory[1] = 0b00000001;
		
	memset(2, 0b00000000, 1);
	_displayMemory[2] = 0b00000000;
	
	memset(3, 0b00000000, 1);
	_displayMemory[3] = 0b00000000;
	*/
	//memset(2, 0b00000000, 1);

	//setSegment(LCD_S1, visible);
	//setSegment(LCD_S11, visible);
	memset(4, 0x00, 2);
	_displayMemory[4] = 0x00;
	_displayMemory[5] = 0x00;
	
	memset(6, 0x00, 1);
	_displayMemory[6] = 0x00;
	
	memset(8, 0x00, 2);
	_displayMemory[8] = 0x00;
	
	//memset(9, 0x00, 1);
	_displayMemory[9] = 0x00;
	
	setSegment(LCD_S2, 0);
	setSegment(LCD_S3, 0); // ZzzZzZzZ
	//setSegment(LCD_Y0, 0);
	
	setSegment(LCD_S6, 0);
	setSegment(LCD_S7, 0); // ZzzZzZzZ
	setSegment(LCD_Y19, 0);
	
	setSegment(LCD_S4, 0);
	setSegment(LCD_S5, 0); // ZzzZzZzZ
	setSegment(LCD_Y18, 0);
	
	setSegment(LCD_S8, 0);
	setSegment(LCD_S9, 0); // ZzzZzZzZ
	setSegment(LCD_S10, 0);
	
	setDigit(1, 10);
	setDigit(2, 10);
	setDigit(3, 10);
	
	setSegment(LCD_P1, 0);
	
	//damper
	setSegment(LCD_T8, 0);
	setSegment(LCD_T9, 0);
	setSegment(LCD_T7, 0); // fresh air
	
	
	
	
	setSegment(LCD_T10, 0);

	setSegment(LCD_T11, 0);

	setSegment(LCD_Z1, 0);
	setSegment(LCD_Z5, 0);

	setSegment(LCD_Z2, 0);
	setSegment(LCD_Z6, 0);

	setSegment(LCD_Z3, 0);
	setSegment(LCD_Z7, 0);

	setSegment(LCD_Z4, 0);
	
}



void CLCD_HVAC::setSensorCO2ppm(int CO2ppm)
{
	uint8_t A, B, C, D;
	
	setSegment(LCD_Z8, HIGH);
	
	if (parse4digitNumber(CO2ppm, A, B, C, D))
	{		
		setDigit(4, A);
		setDigit(5, B);
		setDigit(6, C);
		setDigit(7, D);
	}
	else
	{
		setDigit(4, CH_EMPTY);
		setDigit(5, CH_MINUS);
		setDigit(6, CH_MINUS);
		setDigit(7, CH_MINUS);
	}
}

void CLCD_HVAC::setFirmwareVersion(int version_t)
{
	uint8_t A, B, C, D;
	
	setSegment(LCD_Z8, LOW);
	
	if (parse4digitNumber(version_t, A, B, C, D))
	{
		setDigit(4, A);
		setDigit(5, B);
		setDigit(6, C);
		setDigit(7, D);
	}
	else
	{
		setDigit(4, 10);
		setDigit(5, 10);
		setDigit(6, 10);
		setDigit(7, 10);
	}
}



void CLCD_HVAC::setSensorTemperature(float temperature)
{
	uint8_t A, B, C, D;
	
	setSegment(LCD_Z10, HIGH);
	
	int num = (int)(temperature * 10);
	
	if (parse4digitNumber(num, A, B, C, D))
	{
		setSegment(LCD_P2, HIGH);
		
		setDigit(10, B);
		setDigit(11, C);
		setDigit(12, D);
	}
}

void CLCD_HVAC::setSensorHumidity(int humidity)
{
	uint8_t A, B, C, D;
	
	setSegment(LCD_Z9, HIGH);
	
	if (parse4digitNumber(humidity, A, B, C, D))
	{		
		setDigit(8, C);
		setDigit(9, D);
	}
}

// 0 = no bars, 1 = 1 bar
void CLCD_HVAC::setUserBlowerSpeed(int speed, bool night_mode)
{
	const uint8_t segments[] = { LCD_Y7, LCD_Y6, LCD_Y5, LCD_Y4, LCD_Y3, LCD_Y2, LCD_Y1 };
	
	setSegment(LCD_S2, HIGH);
	setSegment(LCD_S3, night_mode); // ZzzZzZzZ
	setSegment(LCD_Y0, HIGH);
#ifdef MARKETING	
	int speed_t = 0;
	
	// ограничение бризера до 3-х скоростей, парсинг 
	switch (speed)
	{
		case 1:
		speed_t = 3;
		break;
		
		case 2:
		speed_t = 5;
		break;
		
		case 3:
		speed_t = 7;
		break;
	}
#endif	

	for (uint8_t i = 0; i < NUM_OF_ELEMENTS(segments); ++i)
	{				
		#ifdef MARKETING
		setSegment(segments[i], i < speed_t ? HIGH : LOW);
		#else
		setSegment(segments[i], i < speed ? HIGH : LOW);
		#endif
	}
}


// 0 = no bars, 1 = 1 bar
void CLCD_HVAC::setUserBlowerSpeed_optimized(int speed, bool night_mode)
{
	const uint8_t segments[] = { LCD_Y7, LCD_Y6, LCD_Y5, LCD_Y4, LCD_Y3, LCD_Y2, LCD_Y1 };
	
	//setSegment(LCD_S3, night_mode); // ZzzZzZzZ
	
	if (speed == 0)
	{
		setSegment(segments[0], LOW);
	} else {	
		setSegment(segments[speed - 1], HIGH);
		setSegment(segments[speed], LOW);
	}
	
}



void CLCD_HVAC::setUserCoolerIntensity(int intensity, bool night_mode)
{
	const uint8_t segments[] = { LCD_Y17, LCD_Y16, LCD_Y15, LCD_Y14, LCD_Y13, LCD_Y12, LCD_Y11 };
	
	setSegment(LCD_S6, HIGH);
	setSegment(LCD_S7, night_mode); // ZzzZzZzZ
	setSegment(LCD_Y19, HIGH);
	
#ifdef MARKETING
	int intensity_t = 0;
	
	// ограничение бризера до 3-х скоростей, парсинг 
	switch (intensity)
	{
		case 1:
		intensity_t = 3;
		break;
		
		case 2:
		intensity_t = 5;
		break;
		
		case 3:
		intensity_t = 7;
		break;
	}
#endif	
	
	for (uint8_t i = 0; i < NUM_OF_ELEMENTS(segments); ++i)
	{
#ifdef MARKETING
	setSegment(segments[i], i < intensity_t ? HIGH : LOW);
#else
	setSegment(segments[i], i < intensity ? HIGH : LOW);
#endif	
	}
}


void CLCD_HVAC::setUserCoolerIntensity_optimized(int intensity, bool night_mode)
{
	const uint8_t segments[] = { LCD_Y17, LCD_Y16, LCD_Y15, LCD_Y14, LCD_Y13, LCD_Y12, LCD_Y11 };
	
	//setSegment(LCD_S7, night_mode); // ZzzZzZzZ
	
	if (intensity == 0)
	{
		setSegment(segments[0], LOW);
		} else {
		setSegment(segments[intensity - 1], HIGH);
		setSegment(segments[intensity], LOW);
	}
	
}


/*
0 = ZERO, 1 = MIDDLE, 2 = LIMIT
*/

void CLCD_HVAC::setUserDamperPos(int damperPos)
{
	
	
	switch (damperPos)
	{
		case 0: // приток
			setSegment(LCD_T8, HIGH);
			setSegment(LCD_T9, LOW);
			setSegment(LCD_T7, HIGH); // fresh air
			break;
		
		case 1: // смешанный
			setSegment(LCD_T8, HIGH);
			setSegment(LCD_T9, HIGH);
			setSegment(LCD_T7, HIGH);
			break;
		
		case 2: // закрыт
			setSegment(LCD_T8, LOW);
			setSegment(LCD_T9, HIGH);
			setSegment(LCD_T7, LOW);
			break;
			
		case 10: // clear
			setSegment(LCD_T8, LOW);
			setSegment(LCD_T9, LOW);
			setSegment(LCD_T7, LOW);
			break;
	}
}



void CLCD_HVAC::setUserTemperature(float temperature, bool night_mode)
{
	uint8_t A, B, C, D;
	
	setSegment(LCD_S8, HIGH);
	setSegment(LCD_S9, night_mode); // ZzzZzZzZ
	setSegment(LCD_S10, HIGH);
	
	int num = (int)(temperature * 10);
	
	if (parse4digitNumber(num, A, B, C, D))
	{		
		setDigit(1, B);
		setDigit(2, C);
		setDigit(3, D);
		
		setSegment(LCD_P1, HIGH);
	}
}

void CLCD_HVAC::setUserHumidity(int humidity, bool night_mode)
{
	const uint8_t segments[] = { LCD_Y10, LCD_Y9, LCD_Y8 };
	
	setSegment(LCD_S4, HIGH);
	setSegment(LCD_S5, night_mode); // ZzzZzZzZ
	setSegment(LCD_Y18, HIGH);
	
	for (uint8_t i = 0; i < NUM_OF_ELEMENTS(segments); ++i)
	{
		setSegment(segments[i], i < humidity ? HIGH : LOW);
	}
}

bool CLCD_HVAC::parse4digitNumber(int N, uint8_t& A, uint8_t& B, uint8_t& C, uint8_t& D)
{
	if (N < 0 || N > 9999)
	{
		return false;
	}
	
	A = N / 1000;
	N = N - A * 1000;
	
	B = N / 100;
	N = N - B * 100;
	
	C = N / 10;
	N = N - C * 10;
	
	D = N;
	
	return true;
}


void CLCD_HVAC::clearUserTemperature(void)
{
	setSegment(LCD_S8, LOW);
	setSegment(LCD_S9, LOW); // ZzzZzZzZ
	setSegment(LCD_S10, LOW);
	
	setDigit(1, 10);
	setDigit(2, 10);
	setDigit(3, 10);
	
	setSegment(LCD_P1, LOW);
}

void CLCD_HVAC::clearUserBlowerSpeed(void)
{
	memset(4, 0x00, 2);
	_displayMemory[4] = 0x00;
	_displayMemory[5] = 0x00;
	
	setSegment(LCD_S2, 0);
	setSegment(LCD_S3, 0); // ZzzZzZzZ
	setChar(CHAR_BREEZER_ERR, 0);
	setSegment(LCD_T8, 0);
	setSegment(LCD_T9, 0);
	setSegment(LCD_T7, 0); // fresh air
	
}

void CLCD_HVAC::clearUserHumidity(void)
{
	memset(6, 0x00, 1);
	_displayMemory[6] = 0x00;
	
	setSegment(LCD_S4, 0);
	setSegment(LCD_S5, 0); // ZzzZzZzZ
	setChar(CHAR_HUMIDIFIER_ERR, 0);
}

void CLCD_HVAC::clearUserCoolerIntensity(void)
{
	memset(8, 0x00, 2);
	_displayMemory[8] = 0x00;
	_displayMemory[9] = 0x00;
	
	setSegment(LCD_S6, 0);
	setSegment(LCD_S7, 0); // ZzzZzZzZ
	setChar(CHAR_COND_ERR, 0);
}

void CLCD_HVAC::clearIcons(void)
{
    setSegment(LCD_S6, LOW); // icon of conditioner
	setSegment(LCD_S2, LOW); // icon of breezer
	setSegment(LCD_S4, LOW); // icon of humidifier
}

