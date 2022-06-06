#include "customRF_hvac.h"
#include "remote/remote.h"
#include <avr/wdt.h>

#include "core/mpu.h"

#if (CUSTOM_RF_PORT == 1)
#define MY_SERIAL Serial1
#else
#define MY_SERIAL Serial
#endif

#define CONNECTION_ATTEMPT_INTERVAL 5000
#define EEPROM_ADDRESS 0
//#define NETWORK_AVAILABILITY_CHECK_INTERVAL 7700
#define NETWORK_AVAILABILITY_CHECK_INTERVAL 45000
#define SHARED_CONNECTION_ID 22374
#define TIMEOUT 500
CRemote _remote;

uint8_t longer_check_network_availability = 2;

enum Commands
{
	CMD_CONNECT,
	CMD_ACCEPT
};

enum UpdateCheck {
	UPDATE_NONE,
	UPDATE_READY,
	UPDATE_ACTUAL,
	UPDATE_TOO_BIG
};

void readCommandReturn(unsigned char* buffer, size_t size)
{
	unsigned long startTime = millis();
	size_t index = 0;
	
	do
	{
		int incomingByte = MY_SERIAL.read();

		if (incomingByte == -1)
			continue;

		buffer[index++] = (unsigned char)incomingByte;

		if (index >= size)
			break;
	}
	while (millis() - startTime < TIMEOUT);
}

CCustomRF_HVAC::CCustomRF_HVAC(unsigned char type)
: _connectionAttempts(0), _status(0),
_allowBinding(false), _isHost(false), _networkAvailable(false), _pinging(false),
_oldConnection_id(SHARED_CONNECTION_ID),
_myFirmwareVersion(0),
_type(type)
{
	_connection_id = 0;
}

void CCustomRF_HVAC::bindDevice(bool allowBinding)
{
	//_allowBinding = allowBinding;
	
	restoreFactorySettings();
	/*
	if (allowBinding)
	{
		//restoreFactorySettings();
		if (_connection_id != SHARED_CONNECTION_ID && _oldConnection_id == SHARED_CONNECTION_ID)
		{
			
			_oldConnection_id = _connection_id;
			_connection_id = SHARED_CONNECTION_ID;
			
			if (!_isHost)
				connect();
		}
	}
	else
	{
		if (_connection_id == SHARED_CONNECTION_ID)
		{
			_connection_id = _oldConnection_id;
			_status = STATUS_READY;
		}
		
		_oldConnection_id = SHARED_CONNECTION_ID;
	}
	*/
}

bool CCustomRF_HVAC::restoreFactorySettings()
{
	setPan_id(0xffff);
	return true;
}

void CCustomRF_HVAC::init()
{
	MY_SERIAL.begin(9600);
}

bool CCustomRF_HVAC::host()
{
	_isHost = true;
	
	for (uint8_t i = 0; ; )
	{
		if (!recv())
			break;
		
		if (i++ > 10)
			return _status == STATUS_READY;
	}
	
	if (_status < STATUS_READY)
	{
		if (_delayTimer.isFinished())
			_delayTimer.start(CONNECTION_ATTEMPT_INTERVAL);
		else
			return false;
	}
	
	switch (_status)
	{
		case STATUS_OFFLINE:
			load();
			_status = STATUS_READY;
			return false;
	}
	
	return true;
}

bool CCustomRF_HVAC::autoconnect()
{
	unsigned short pan_id;

	_isHost = false;
	
	for (uint8_t i = 0; ; )
	{
		if (!recv())
			break;
		
		if (i++ > 10)
			return _status == STATUS_READY;
	}
	
	if (_status < STATUS_READY)
	{
		if (_delayTimer.isFinished())
			_delayTimer.start(CONNECTION_ATTEMPT_INTERVAL);
		else
			return false;
	}
	
	switch (_status)
	{
		case STATUS_OFFLINE:
			load();
			_status = STATUS_READY;
			_checkTimer.start(NETWORK_AVAILABILITY_CHECK_INTERVAL);

			break;
		
		case STATUS_SEARCHING:
			pan_id = readPan_id();
			if (pan_id > 0x0000 && pan_id < 0xFFFF)
			{
				if (pan_id == 0xFFFE) // Router error
				{
					searchHost();
				}
				
				delay(100);
				int networkStatus = queryNetworkStatus();
				//wtf.setCO2ppm(networkStatus);
				if (networkStatus == 0x05) // Unauthorized
				{
					//searchHost();
				}
				else if (networkStatus == 0x07 || networkStatus == 0x06) // Router join the network
				{
					connect();
					_delayTimer.start(CONNECTION_ATTEMPT_INTERVAL);
					break;
				}
			}
			break;
		
		case STATUS_CONNECTING:
			if (_connectionAttempts++ < 1)
			{
				sendCommand(CMD_CONNECT, _type);
				_delayTimer.start(CONNECTION_ATTEMPT_INTERVAL);
			}
			else
			{
				searchHost();
			}
			break;
		
		case STATUS_READY:
			if (_buffer.getLength() > 0)
			{
				_networkAvailable = true;
				_checkTimer.start(NETWORK_AVAILABILITY_CHECK_INTERVAL);
				//wtf.setCO2ppm(queryNetworkStatus());
			}
			else if (_checkTimer.isFinished())
			{
				//wtf.setCO2ppm(queryNetworkStatus());
				_networkAvailable = false;
			}
			return true;
	}
	
	return false;
}

void CCustomRF_HVAC::connect()
{
	_status = STATUS_CONNECTING;
	_connectionAttempts = 0;
}

void CCustomRF_HVAC::searchHost()
{
	_status = STATUS_SEARCHING;
	setPan_id(0xFFFF); // pan_id = 0xFFFF = find next available network
}

void CCustomRF_HVAC::load()
{
	
	if (EEPROM.read(EEPROM_ADDRESS) == 0x5a)
	{
		EEPROM.read(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
	}
	else
	{
		_connection_id = _isHost ? 0 : 1;
	}
	
	/*
	//0.67
	_myFirmwareVersion = (uint16_t)VERSION_MAJOR << 8 | (uint16_t)VERSION_MINOR;
	
	if (EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS) == 0x5a)
	{
		uint16_t temp_vers = 0;
		EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&temp_vers, sizeof(temp_vers));
		
		if (temp_vers == _myFirmwareVersion)
		{
			return;
		}
	}
	EEPROM.write(FIRMWARE_VERSION_EEPROM_ADDRESS, 0x5a);
	EEPROM.write(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&_myFirmwareVersion, sizeof(_myFirmwareVersion));
	softReset();
	*/
	
	
	
	
	/*
	if (EEPROM.read(EEPROM_ADDRESS) == 0x5a)
	{
		EEPROM.read(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
	}
	else
	{
		_connection_id = _isHost ? 0 : 1;
	}
	
	
	if (EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS) == 0x5a)
	{
		EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&_myFirmwareVersion, sizeof(_myFirmwareVersion));
	}
	*/
}

void CCustomRF_HVAC::save()
{
	EEPROM.write(EEPROM_ADDRESS, 0x5a);
	EEPROM.write(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
}

unsigned short CCustomRF_HVAC::readPan_id()
{
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	
	MY_SERIAL.write(0x02);
	MY_SERIAL.write(0xA8);
	MY_SERIAL.write(0x79);
	MY_SERIAL.write(0xC3);
	MY_SERIAL.write(0x05);

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[5] << 8 | answer[6];
}

bool CCustomRF_HVAC::setPan_id(unsigned short pan_id)
{
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0 };
	//do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	
	MY_SERIAL.write(0x02);
	MY_SERIAL.write(0xA8);
	MY_SERIAL.write(0x79);
	MY_SERIAL.write(0xC3);
	MY_SERIAL.write(0x04);
	MY_SERIAL.write((unsigned char)(pan_id >> 8));
	MY_SERIAL.write((unsigned char)(pan_id));

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[4] == 0x4F && answer[5] == 0x4B;
}

void CCustomRF_HVAC::SystemResetE18(void)
{
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	MY_SERIAL.write(0x02);
	MY_SERIAL.write(0xA8);
	MY_SERIAL.write(0x79);
	MY_SERIAL.write(0xC3);
	MY_SERIAL.write(0x06); // command for system reset of E18
}

void CCustomRF_HVAC::SetPollRateE18(unsigned char my_poll_rate)
{
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	MY_SERIAL.write(0x02);
	MY_SERIAL.write(0xA8);
	MY_SERIAL.write(0x79);
	MY_SERIAL.write(0xC3);
	MY_SERIAL.write(0x07); // command for set new poll rate
	MY_SERIAL.write(my_poll_rate); 
}

int CCustomRF_HVAC::queryNetworkStatus()
{
	/*
		Query network status: 02 A8 79 C3 01
		Return value: 02 A8 79 C3 01 YY
		YY: 02 06 07 09
		02: Equipment not to join the network
		06: End Device join the network
		07: Router join the network
		09: Coordinator is working
	*/
	
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0 };
	
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	
	MY_SERIAL.write(0x02);
	MY_SERIAL.write(0xA8);
	MY_SERIAL.write(0x79);
	MY_SERIAL.write(0xC3);
	MY_SERIAL.write(0x01);

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[5];
}

void CCustomRF_HVAC::responseUpdateCheck()
{
	// 0x02, 0xA8, 0x79, 0xC3, 0x40 AA BB CC DD EE
	
	if (_type != _updateCommandBuffer[5])
		return;
		
	uint16_t updateVersion = _updateCommandBuffer[6] << 8 | _updateCommandBuffer[7];
	
		
	uint8_t updateCheck = UPDATE_READY; // ready to update
	if (_myFirmwareVersion >= updateVersion)
		updateCheck = UPDATE_ACTUAL; // actual firmware
	
	uint8_t response[] = { 0x02, 0xA8, 0x79, 0xC3, 0x41, updateCheck };
	
	sendRawData(response, NUM_OF_ELEMENTS(response));
	
	if (updateCheck == UPDATE_READY)
	{
		_remote.showTextUpdate();
		softReset();
	}
}

void CCustomRF_HVAC::softReset()
{
	wdt_enable(WDTO_15MS);
	while(1);
}

void CCustomRF_HVAC::commandEventRun(unsigned char command, unsigned short param)
{
	if (_isHost)
	{
		switch (command)
		{
			case CMD_CONNECT:
				if (_connection_id == SHARED_CONNECTION_ID)
				{
					unsigned short newConnection_id = makeWord((byte)millis(), (byte)param);
					
					if (newConnection_id == SHARED_CONNECTION_ID)
					{
						++newConnection_id;
					}
					
					sendCommand(CMD_ACCEPT, newConnection_id);
					_connection_id = newConnection_id;
					save();
				}
				else
				{
					sendCommand(CMD_ACCEPT);
				}
				
				_status = STATUS_READY;
				
				break;
		}
	}
	else
	{
		switch (command)
		{
			case CMD_ACCEPT:
				if (_connection_id == SHARED_CONNECTION_ID)
				{
					_connection_id = param;
					save();
				}
				
				_status = STATUS_READY;
				
				break;
		}
	}
	
	_networkAvailable = true;
	_checkTimer.start(NETWORK_AVAILABILITY_CHECK_INTERVAL);

}

int CCustomRF_HVAC::read()
{
	return MY_SERIAL.read();
}

void CCustomRF_HVAC::write(unsigned char c)
{
	MY_SERIAL.write(c);
}

bool CCustomRF_HVAC::updateCommandEventRun()
{
	// 0x02, 0xA8, 0x79, 0xC3, 0x40 AA BB CC DD EE
	
	if (_updateCommandBufferSize == 10)
	{
		if (_updateCommandBuffer[4] == 0x40)
		{
			if (EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS) == 0x5a) // считываем версию
			{
				EEPROM.read(FIRMWARE_VERSION_EEPROM_ADDRESS + 1, (uint8_t*)&_myFirmwareVersion, sizeof(_myFirmwareVersion));
			}
			
			responseUpdateCheck();
		}
		
		return true;
	}
	
	return false;
}

// Подаём импульс (stroke) по ножке TX (AVR), чтобы Е18 успела проснуться до начала передачи данных ей по UART
void do_impulse_for_wakeup_E18()
{
	MY_SERIAL.write(0xAA);
	delay(3); // при delay(1) уже не работает
}