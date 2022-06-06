#ifndef _shared_h_
#define _shared_h_

// header
enum
{
	HEADER_BREEZER = 1,
	HEADER_REMOTE = 2,
	HEADER_BREEZER_ERR = 3,
	HEADER_COND = 4,
	HEADER_HUMIDIFIER = 5	
};

// cmd
enum { CMD_TEST, CMD_FORCED, NUM_OF_CMDS };
// damper position
enum { ZERO1, MIDDLE, THE_LIMIT	};
#define CLEAR_DAMPER_POSITION 10

struct BreezerState
{
	uint8_t header;
	uint8_t lastCmd;
	bool powerOn;
	int8_t tempIn;
	float temperature;
	uint8_t blowerSpeed, damperPosition;
};

struct RemoteState
{
	uint8_t header;
	uint8_t cmd;
	bool saveSettings, powerOn, autoOn, nightOn;
	uint8_t userBlowerSpeed, userDamperPosition, userHumidity;
	float userTemperature;
	uint8_t REFmode, REFfanSpeed;
	float REFtempSetp;
};

// errorCode
enum { E_01 = 1, E_02, E_03, E_04, E_05, E_06, E_07, E_08, NUM_OF_ERRORS = E_08 };

struct ErrorState
{
	uint8_t header;
	uint16_t errorFlags;
};

struct CondState
{
	uint8_t header;
	uint8_t lastCmd;
	uint8_t REFfanSpeed;
	float REFtempSupply;
	uint16_t errorFlags;
};

struct HumidifierState
{
	uint8_t header;
	uint8_t lastCmd;
	bool fillWater, powerOn;
	uint8_t humidifierStage;
};

#endif
