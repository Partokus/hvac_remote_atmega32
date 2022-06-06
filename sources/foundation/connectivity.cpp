#include "connectivity.h"
#include "../core/mpu.h"
#include "../customRF_hvac.h"
#include "../remote/remote.h"
#define NUM_OF_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

enum ReceivingState
{
	RECEIVING_NONE,
	RECEIVING_SYNCWORD = RECEIVING_NONE,
	RECEIVING_CONNECTION_ID,
	RECEIVING_LENGTH,
	RECEIVING_PAYLOAD,
	RECEIVING_LRC
};

const char* syncword = "pe";
const unsigned char updateCommandHeader[] = { 0x02, 0xA8, 0x79, 0xC3, 0x40 };

CConnectivity::CConnectivity()
: _bufferSizeLimit(90),
_connection_id(0),
_updateCommandBufferSize(0),
correct_panid(0),
_id(0),
_payloadLength(0), _LRC(0),
_receivedBytes(0),
_receivingState(RECEIVING_NONE)
{
}

CBuffer* CConnectivity::lockBuffer()
{
	if (_buffer.getLength() < 1) return NULL;
	
	_buffer.seekAbs(0);
	
	return &_buffer;
}

void CConnectivity::unlockBuffer()
{
	_buffer.clear();
}

void CConnectivity::send(const unsigned char* buffer, unsigned char size)
{
	size_t i;
	
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	
	for (i = 0; syncword[i]; ++i)
	write(syncword[i]);
	
	write((unsigned char)(_connection_id >> 8));
	write((unsigned char)(_connection_id));
	
	write(size);

	unsigned char LRC = 0;

	for (i = 0; i < size; ++i)
	{
		LRC ^= buffer[i];
		write(buffer[i]);
	}
	
	write(LRC);
}



void CConnectivity::sendRawData(const unsigned char* buffer, unsigned char size)
{
	do_impulse_for_wakeup_E18(); // Посылаем импульс для Е18, чтобы она успела проснуться для приёма данных по UART
	
	for (unsigned char i = 0; i < size; ++i)
	{
		write(buffer[i]);
	}
}

bool CConnectivity::recv()
{
	int incomingByte = read();

	if (incomingByte == -1)
		return false;

	unsigned char bits = (unsigned char)incomingByte;
	proceedUpdateCommand(bits);

	++_receivedBytes;

	switch (_receivingState)
	{
	case RECEIVING_SYNCWORD:
		if (syncword[_receivedBytes - 1] != bits)
			resetFrame();
		else if (syncword[_receivedBytes] == 0)
			proceedFrame();
		break;
		
	case RECEIVING_CONNECTION_ID:
		if (_receivedBytes == 1)
		{
			_id = (unsigned short)bits << 8;
		}
		else
		{
			_id += bits;
			/*
			if (_id != _connection_id) resetFrame();			
			else proceedFrame();
			*/
			my_panid_t = _id;
			//if (_id != _connection_id) correct_panid = false; // panid не верен
			//else correct_panid = true; // panid верен
			
			proceedFrame();
		}
		break;

	case RECEIVING_LENGTH:
		_LRC = 0;
		_payloadLength = bits;
		_receivingBuffer.clear();
		proceedFrame();
		break;

	case RECEIVING_PAYLOAD:
		if (_receivedBytes == 1) // if it's header byte
		{
			if (bits == HEADER_COND) // if it's conditioner's header
			{
				_connection_id = my_panid_t;
				//if (correct_panid == false) resetFrame(); // if panid isn't corrent, so ignore packet
			}
		}
		
		_LRC ^= bits;
		_receivingBuffer.write(&bits, 1);
		if (_receivedBytes >= _payloadLength)
			proceedFrame();
		break;

	case RECEIVING_LRC:
		if (_LRC == bits)
			parseFrame();
		resetFrame();
		break;
	}
	
	return true;
}

void CConnectivity::sendCommand(unsigned char command, unsigned short param)
{
	unsigned char buffer[] = { 'C', 'M', 'D', command, (unsigned char)(param >> 8), (unsigned char)(param) };
	
	send(buffer, sizeof(buffer));
}

bool CConnectivity::parseCommand(unsigned char *bits, size_t length)
{
	// C+M+D+command+param_hi_byte+param_low_byte
	
	if (length == 6)
	{		
		if (bits[0] == 'C' && bits[1] == 'M' && bits[2] == 'D')
		{
			commandEventRun(bits[3], bits[4] << 8 | bits[5]);
			return true;
		}
	}
	
	return false;
}

void CConnectivity::parseFrame()
{
	unsigned char *bits = _receivingBuffer.getBits(0);
	
	if (parseCommand(bits, _payloadLength))
		return;
	
	if (_payloadLength > _bufferSizeLimit)
		return;
	
	if (_buffer.getLength() + _payloadLength > _bufferSizeLimit)
		_buffer.clear();
	
	_buffer.write(bits, _payloadLength);
}

void CConnectivity::proceedFrame()
{
	_receivedBytes = 0;
	++_receivingState;
}

void CConnectivity::resetFrame()
{
	_receivedBytes = 0;
	_receivingState = RECEIVING_NONE;
}

void CConnectivity::proceedUpdateCommand(unsigned char c)
{
	bool isHeader = _updateCommandBufferSize < NUM_OF_ELEMENTS(updateCommandHeader);
	if (isHeader)
	{
		if (updateCommandHeader[_updateCommandBufferSize] != c)
		{
			_updateCommandBufferSize = 0;
			return;
		}
	}
	
	_updateCommandBuffer[_updateCommandBufferSize++] = c;
	
	if (!isHeader)
	{
		if (updateCommandEventRun())
		{
			_updateCommandBufferSize = 0;
		}
		
		if (_updateCommandBufferSize >= NUM_OF_ELEMENTS(_updateCommandBuffer))	
		{
			_updateCommandBufferSize = 0;
		}
	}
}
