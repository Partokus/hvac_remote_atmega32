#include "rl_cc2530_pa2.h"

#include "core/mpu.h"

#define EEPROM_ADDRESS 0
#define LOOKING_FOR_NETWORK_TIME 10000
#define NETWORK_AVAILABILITY_CHECK_INTERVAL 6000
#define SHARED_CONNECTION_ID 22371
#define TIMEOUT 1000

enum Commands
{
	CMD_CONNECT,
	CMD_PING,
	CMD_PONG
};

void readCommandReturn(unsigned char* buffer, size_t size)
{
	int incomingByte;

	unsigned long startTime = millis();

	size_t index = 0;
	
	do
	{
		incomingByte = Serial.read();

		if (incomingByte == -1)
			continue;

		buffer[index++] = (unsigned char)incomingByte;

		if (index >= size)
			break;
	}
	while (millis() - startTime < TIMEOUT);
}

CRL_CC2530_PA2::CRL_CC2530_PA2()
: _connectionAttempts(0), _status(0),
_allowBinding(false), _isHost(false), _networkAvailable(false), _pinging(false),
_oldConnection_id(SHARED_CONNECTION_ID)
{
	_connection_id = 0;
}

void CRL_CC2530_PA2::bindDevice(bool allowBinding)
{
	_allowBinding = allowBinding;
	
	if (allowBinding)
	{
		if (_connection_id != SHARED_CONNECTION_ID && _oldConnection_id == SHARED_CONNECTION_ID)
		{
			restoreFactorySettings();
			
			_oldConnection_id = _connection_id;
			_connection_id = SHARED_CONNECTION_ID;
			
			if (!_isHost)
				searchHost();
		}
	}
	else
	{
		if (_connection_id == SHARED_CONNECTION_ID)
			_connection_id = _oldConnection_id;
		
		_oldConnection_id = SHARED_CONNECTION_ID;
	}
}

bool CRL_CC2530_PA2::restoreFactorySettings()
{
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0, 0 };
	
	Serial.write(0x02);
	Serial.write(0xA8);
	Serial.write(0x79);
	Serial.write(0xC3);
	Serial.write(0x03);
	
	delay(2000);

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));
	
	return answer[5] == 0x4F && answer[6] == 0x4B;
}

void CRL_CC2530_PA2::init()
{
	Serial.begin(9600);
}

bool CRL_CC2530_PA2::host()
{
	_isHost = true;
	
	uint8_t i = 0;
	
	while (true)
	{
		if (!recv())
			break;
		
		if (i++ > 10)
			return _status == STATUS_READY;
	}
	
	if (_status < STATUS_READY)
	{
		if (_delayTimer.isFinished())
			_delayTimer.start(TIMEOUT);
		else
			return false;
	}
	
	switch (_status)
	{
		case STATUS_OFFLINE:
			if (readPan_id() != 0)
			{
				_status = STATUS_DISCONNECTED;
				load();
			}
			return false;
	}
	
	return true;
}

bool CRL_CC2530_PA2::autoconnect()
{
	unsigned short pan_id;

	_isHost = false;
	
	uint8_t i = 0;
	
	while (true)
	{
		if (!recv())
			break;
		
		if (i++ > 10)
			return _status == STATUS_READY;
	}
		
	if (_status < STATUS_READY)
	{
		if (_delayTimer.isFinished())
			_delayTimer.start(TIMEOUT);
		else
			return false;
	}
	
	switch (_status)
	{
		case STATUS_OFFLINE:
			if (readPan_id() != 0)
			{
				_status = STATUS_SEARCHING;
				load();
				_checkTimer.start(LOOKING_FOR_NETWORK_TIME);
			}
			break;
		
		case STATUS_SEARCHING:
			pan_id = readPan_id();
			if (pan_id >= 0x0001 && pan_id <= 0xFF00 && queryNetworkStatus() == 0x07) // Router join the network
				connect();
			else if (_checkTimer.isFinished())
				searchHost();
			break;
		
		case STATUS_CONNECTING:
			if (_connectionAttempts++ < 1)
			{
				sendCommand(CMD_CONNECT, millis());
				_delayTimer.start(2000);
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
			}
			else if (_checkTimer.isFinished())
			{
				if (_pinging)
				{
					_networkAvailable = false;
					_pinging = false;
					searchHost();
				}
				else if (_networkAvailable)
				{
					sendCommand(CMD_PING);
					_pinging = true;
					_checkTimer.start(TIMEOUT);
				}
			}
			return true;
	}
	
	return false;
}

void CRL_CC2530_PA2::connect()
{
	_status = STATUS_CONNECTING;
	_connectionAttempts = 0;
}

void CRL_CC2530_PA2::searchHost()
{
	_status = STATUS_SEARCHING;
	setPan_id(0xFFFF); // pan_id = 0xFFFF = find next available network
	_checkTimer.start(LOOKING_FOR_NETWORK_TIME);
}

void CRL_CC2530_PA2::load()
{
	if (EEPROM.read(EEPROM_ADDRESS) != 0x5a)
	{
		_connection_id = _isHost ? 0 : 1;
		return;
	}
	
	EEPROM.read(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
}

void CRL_CC2530_PA2::save()
{
	EEPROM.write(EEPROM_ADDRESS, 0x5a);
	EEPROM.write(EEPROM_ADDRESS + 1, (uint8_t*)&_connection_id, sizeof(_connection_id));
}

unsigned short CRL_CC2530_PA2::readPan_id()
{
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

	Serial.write(0x02);
	Serial.write(0xA8);
	Serial.write(0x79);
	Serial.write(0xC3);
	Serial.write(0x05);

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[5] << 8 | answer[6];
}

bool CRL_CC2530_PA2::setPan_id(unsigned short pan_id)
{
	unsigned char answer[] = { 0, 0, 0, 0, 0, 0 };

	Serial.write(0x02);
	Serial.write(0xA8);
	Serial.write(0x79);
	Serial.write(0xC3);
	Serial.write(0x04);
	Serial.write((unsigned char)(pan_id >> 8));
	Serial.write((unsigned char)(pan_id));

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[4] == 0x4F && answer[5] == 0x4B;
}

int CRL_CC2530_PA2::queryNetworkStatus()
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
	
	Serial.write(0x02);
	Serial.write(0xA8);
	Serial.write(0x79);
	Serial.write(0xC3);
	Serial.write(0x01);

	readCommandReturn(answer, NUM_OF_ELEMENTS(answer));

	return answer[5];
}

void CRL_CC2530_PA2::commandEventRun(unsigned char command, unsigned short param)
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
						++newConnection_id;
					
					sendCommand(CMD_CONNECT, newConnection_id);
					_connection_id = newConnection_id;
					save();
				}
				else
				{
					sendCommand(CMD_CONNECT);
				}
				
				_status = STATUS_READY;
				
				break;
		
			case CMD_PING:
				sendCommand(CMD_PONG);
				break;
		}
	}
	else
	{
		switch (command)
		{
			case CMD_CONNECT:
				if (_connection_id == SHARED_CONNECTION_ID)
				{
					_connection_id = param;
					save();
				}
				
				_status = STATUS_READY;
				
				break;
				
			case CMD_PONG:
				_pinging = false;
				break;
		}
	}
	
	_networkAvailable = true;
	_checkTimer.start(NETWORK_AVAILABILITY_CHECK_INTERVAL);
}

int CRL_CC2530_PA2::read()
{
	return Serial.read();
}

void CRL_CC2530_PA2::write(unsigned char c)
{
	Serial.write(c);
}
