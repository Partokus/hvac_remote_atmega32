#ifndef _lcd_hvac_h_
#define _lcd_hvac_h_

#include <avr/pgmspace.h>

#include "../ht162x.h"

enum
{
	LCD_S10,  // ADDRESS=0
	LCD_NC1,
	LCD_NC2,
	LCD_NC3,
	LCD_NC4,  // ADDRESS=1
	LCD_Z1,
	LCD_Z5,
	LCD_S1,
	
	LCD_S2,   // ADDRESS=2
	LCD_S3,
	LCD_S4,
	LCD_S5,
	LCD_S6,   // ADDRESS=3
	LCD_S7,
	LCD_S8,
	LCD_S9,
	
	LCD_Y0,   // ADDRESS=4
	LCD_Y1,
	LCD_Y2,
	LCD_Y3,
	LCD_Y4,   // ADDRESS=5
	LCD_Y5,
	LCD_Y6,
	LCD_Y7,
	
	LCD_Y18,  // ADDRESS=6
	LCD_Y8,
	LCD_Y9,
	LCD_Y10,
	LCD_Z2,   // ADDRESS=7
	LCD_Z6,
	LCD_Z3,
	LCD_Z7,
	
	LCD_Y19,  // ADDRESS=8
	LCD_Y11,
	LCD_Y12,
	LCD_Y13,
	LCD_Y14,  // ADDRESS=9
	LCD_Y15,
	LCD_Y16,
	LCD_Y17,
	
	LCD_1A,   // ADDRESS=10
	LCD_1B,
	LCD_1F,
	LCD_1G,
	LCD_1C,   // ADDRESS=11
	LCD_1E,
	LCD_1D,
	LCD_NC5,
	
	LCD_2A,   // ADDRESS=12
	LCD_2B,
	LCD_2F,
	LCD_2G,
	LCD_2C,   // ADDRESS=13
	LCD_2E,
	LCD_2D,
	LCD_Z4,
	
	LCD_3A,   // ADDRESS=14
	LCD_3B,
	LCD_3F,
	LCD_3G,
	LCD_3C,   // ADDRESS=15
	LCD_3E,
	LCD_3D,
	LCD_P1,
	
	LCD_P2,   // ADDRESS=16
	LCD_12D,
	LCD_12E,
	LCD_12C,
	LCD_12G,  // ADDRESS=17
	LCD_12F,
	LCD_12B,
	LCD_12A,
	
	LCD_11D,  // ADDRESS=18
	LCD_11E,
	LCD_11C,
	LCD_11G,
	LCD_11F,  // ADDRESS=19
	LCD_11B,
	LCD_11A,
	LCD_Z10,
	
	LCD_10D,  // ADDRESS=20
	LCD_10E,
	LCD_10C,
	LCD_10G,
	LCD_10F,  // ADDRESS=21
	LCD_10B,
	LCD_10A,
	LCD_Z9,
	
	LCD_9D,   // ADDRESS=22
	LCD_9E,
	LCD_9C,
	LCD_9G,
	LCD_9F,   // ADDRESS=23
	LCD_9B,
	LCD_9A,
	LCD_NC6,
	
	LCD_8D,   // ADDRESS=24
	LCD_8E,
	LCD_8C,
	LCD_8G,
	LCD_8F,   // ADDRESS=25
	LCD_8B,
	LCD_8A,
	LCD_S11,
	
	LCD_NC7,  // ADDRESS=26
	LCD_NC8,
	LCD_NC9,
	LCD_NC10,
	LCD_NC11, // ADDRESS=27
	LCD_NC12,
	LCD_NC13,
	LCD_NC14,
	
	LCD_NC15, // ADDRESS=28
	LCD_NC16,
	LCD_NC17,
	LCD_NC18,
	LCD_NC19, // ADDRESS=29
	LCD_NC20,
	LCD_NC21,
	LCD_NC22,
	
	LCD_NC23, // ADDRESS=30
	LCD_NC24,
	LCD_NC25,
	LCD_NC26,
	LCD_NC27, // ADDRESS=31
	LCD_NC28,
	LCD_NC29,
	LCD_NC30,
	
	LCD_7D,   // ADDRESS=32
	LCD_7E,
	LCD_7C,
	LCD_7G,
	LCD_7F,   // ADDRESS=33
	LCD_7B,
	LCD_7A,
	LCD_Z8,
	
	LCD_6D,   // ADDRESS=34
	LCD_6E,
	LCD_6C,
	LCD_6G,
	LCD_6F,   // ADDRESS=35
	LCD_6B,
	LCD_6A,
	LCD_T11,
	
	LCD_5D,   // ADDRESS=36
	LCD_5E,
	LCD_5C,
	LCD_5G,
	LCD_5F,   // ADDRESS=37
	LCD_5B,
	LCD_5A,
	LCD_T10,
	
	LCD_4D,   // ADDRESS=38
	LCD_4E,
	LCD_4C,
	LCD_4G,
	LCD_4F,   // ADDRESS=39
	LCD_4B,
	LCD_4A,
	LCD_T9,
	
	LCD_T8,   // ADDRESS=40
	LCD_T7,
	LCD_T6,
	LCD_T5,
	LCD_T4,   // ADDRESS=41
	LCD_T3,
	LCD_T2,
	LCD_T1,
	
	NUM_OF_SEGMENTS
};

#define ADDRESS(x) ((x) / 4)
#define BITMASK(x) (1 << (3 - (x - ADDRESS(x) * 4)))
#define DISPLAY_MEMORY_SIZE (ADDRESS(NUM_OF_SEGMENTS))

const uint8_t PROGMEM digit_segments_PGM[] = {
	LCD_1A, LCD_1B, LCD_1C, LCD_1D, LCD_1E, LCD_1F, LCD_1G,
	LCD_2A, LCD_2B, LCD_2C, LCD_2D, LCD_2E, LCD_2F, LCD_2G,
	LCD_3A, LCD_3B, LCD_3C, LCD_3D, LCD_3E, LCD_3F, LCD_3G,
	LCD_4A, LCD_4B, LCD_4C, LCD_4D, LCD_4E, LCD_4F, LCD_4G,
	LCD_5A, LCD_5B, LCD_5C, LCD_5D, LCD_5E, LCD_5F, LCD_5G,
	LCD_6A, LCD_6B, LCD_6C, LCD_6D, LCD_6E, LCD_6F, LCD_6G,
	LCD_7A, LCD_7B, LCD_7C, LCD_7D, LCD_7E, LCD_7F, LCD_7G,
	LCD_8A, LCD_8B, LCD_8C, LCD_8D, LCD_8E, LCD_8F, LCD_8G,
	LCD_9A, LCD_9B, LCD_9C, LCD_9D, LCD_9E, LCD_9F, LCD_9G,
	
	LCD_10A, LCD_10B, LCD_10C, LCD_10D, LCD_10E, LCD_10F, LCD_10G,
	LCD_11A, LCD_11B, LCD_11C, LCD_11D, LCD_11E, LCD_11F, LCD_11G,
	LCD_12A, LCD_12B, LCD_12C, LCD_12D, LCD_12E, LCD_12F, LCD_12G,
};

// g,f,e,d,c,b,a

enum
{
	CH_EMPTY = 10,
	CH_MINUS,
	CH_A,
	CH_C,
	CH_E,
	CH_F,
	CH_O,
};

#define LCD_SYMBOL_U 17
#define LCD_SYMBOL_P 18
#define LCD_SYMBOL_D 19

const uint8_t PROGMEM segment_char_map_PGM[] = {
	0b00111111, // 0
	0b00000110, // 1
	0b01011011, // 2
	0b01001111, // 3
	0b01100110, // 4
	0b01101101, // 5
	0b01111101, // 6
	0b00000111, // 7
	0b01111111, // 8
	0b01101111, // 9
	
	0b00000000, // * = 10
	0b01000000, // - = 11
	0b01110111, // A = 12
	0b00111001, // C = 13
	0b01111001, // E = 14
	0b01110001, // F = 15
	0b00111111, // O = 16
	
	0b00111110, // U = 17
	0b01110011, // P = 18
	0b01011110, // d = 19
};

enum
{
	CHAR_HUB,
	CHAR_SEARCH,
	CHAR_NIGHT,
	CHAR_AUTO,
	CHAR_BREEZER_ERR,
	CHAR_HUMIDIFIER_ERR,
	CHAR_COND_ERR,
	CHAR_ERR,
	CHAR_DUCT
};

enum
{
	DIG_0 = 0,
	DIG_1,
	DIG_2,
	DIG_3,
	DIG_4,
	DIG_5,
	DIG_6,
	DIG_7,
	DIG_8,
	DIG_9,
	DIG_SPACE,
	DIG_O,
	DIG_F,
	DIG_PAR,
	DIG_A,
	DIG_C,
	DIG_E,
	DIG_DASH,
	DIG_N,
	DIG_U,
	DIG_P,
	DIG_d,
	DIG_S,
	DIG_R,
};

class CLCD_HVAC : public CHT162x
{
public:
	CLCD_HVAC(uint8_t data, uint8_t wr, uint8_t rd, uint8_t cs);
	
	void clear();
	void turnOn();
	void turnOff();
	void powerOff();
	void setDigit(uint8_t num, uint8_t value);
	void setSegment(uint8_t segment, uint8_t value);
	void setAll();
	
public:	
	void setChar(int8_t _char, bool visible);
	void setDecoration(bool visible);
	
	void setSensorCO2ppm(int CO2ppm);
	void setSensorTemperature(float temperature);
	void setSensorHumidity(int humidity);
	void setFirmwareVersion(int version_t);
	void setUserBlowerSpeed(int speed, bool night_mode);
	void setUserBlowerSpeed_optimized(int speed, bool night_mode);
	void setUserCoolerIntensity(int intensity, bool night_mode);
	void setUserCoolerIntensity_optimized(int intensity, bool night_mode);
	void setUserHumidity(int humidity, bool night_mode);
	void setUserTemperature(float temperature, bool night_mode);
	void setUserDamperPos(int damperPos);
	
	void clearUserTemperature(void);
	void clearUserBlowerSpeed(void);
	void clearUserHumidity(void);
	void clearUserCoolerIntensity(void);
	void clearIcons(void);
private:
	bool parse4digitNumber(int N, uint8_t& A, uint8_t& B, uint8_t& C, uint8_t& D);
	
protected:
	uint8_t _displayMemory[DISPLAY_MEMORY_SIZE];
};

#endif
