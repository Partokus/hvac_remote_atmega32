#include "buffer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int DATA_BLOCK_SIZE = 50;

CBuffer::CBuffer(size_t length)
: _bits(NULL), _capacity(0), _currentLength(0), _position(0)
{
	if (length > 0)
		reserve(length);
}

CBuffer::~CBuffer()
{
	free(_bits);
}

unsigned char* CBuffer::getBits(size_t offset) const
{
	if (offset >= _currentLength)
		return NULL;

	return _bits + offset;
}

void CBuffer::clear()
{
	_currentLength = _position = 0;
}

size_t CBuffer::seek(int offset, int origin)
{
	switch (origin)
	{
	case 0:
		return seekAbs((size_t)offset);
	case 1:
		return seekAbs(_position + offset);
	case 2:
		return seekAbs(_currentLength + offset);
	}

	return 0;
}

size_t CBuffer::seekAbs(size_t offset)
{
	_position = offset;

	if (_position > _currentLength)
		_position = _currentLength;

	return _position;
}

size_t CBuffer::read(void* buffer, size_t size)
{
	if (_position >= _currentLength)
		return 0;

	size_t bytesRead = size;

	if (_position + bytesRead > _currentLength)
		bytesRead = _currentLength - _position;

	memcpy(buffer, _bits + _position, bytesRead);

	_position += bytesRead;

	return bytesRead;
}

CString CBuffer::readLine()
{
	char buffer[100];
	CString str;

	size_t index = 0;

	for ( ; _position < _currentLength; ++_position)
	{
		if (_bits[_position] == '\r' || _bits[_position] == '\n')
			break;

		buffer[index++] = _bits[_position];

		if (index == sizeof(buffer) - 1)
		{
			buffer[index] = '\0';
			str = str + buffer;
			index = 0;
		}
	}

	if (_position < _currentLength)
		if (_bits[_position] == '\r')
			++_position;

	if (_position < _currentLength)
		if (_bits[_position] == '\n')
			++_position;

	buffer[index] = '\0';
	str = str + buffer;

	return str;
}

CString CBuffer::readString()
{
	char buffer[100];
	CString str;

	size_t index = 0;

	for ( ; _position < _currentLength; ++_position)
	{
		if (_bits[_position] == '\0')
			break;

		buffer[index++] = _bits[_position];

		if (index == sizeof(buffer) - 1)
		{
			buffer[index] = '\0';
			str = str + buffer;
			index = 0;
		}
	}

	if (_position < _currentLength)
		if (_bits[_position] == '\0')
			++_position;

	buffer[index] = '\0';
	str = str + buffer;

	return str;
}

CString CBuffer::readText()
{
	char* buffer = (char*)malloc(_currentLength + 1);

	if (buffer)
	{
		memcpy(buffer, _bits, _currentLength);

		buffer[_currentLength] = '\0';
	}

	return buffer;
}

CString CBuffer::readWord()
{
	char buffer[100];
	CString str;

	for ( ; _position < _currentLength; ++_position)
		if (_bits[_position] >= '!')
			break;

	size_t index = 0;

	for ( ; _position < _currentLength; ++_position)
	{
		if (_bits[_position] < '!')
			break;

		buffer[index++] = _bits[_position];

		if (index == sizeof(buffer) - 1)
		{
			buffer[index] = '\0';
			str = str + buffer;
			index = 0;
		}
	}

	buffer[index] = '\0';
	str = str + buffer;

	return str;
}

void CBuffer::write(const void* buffer, size_t size)
{
	if (!buffer || size == 0)
		return;

	_position = _currentLength;

	reserve(_currentLength + size);

	if (_bits)
	{
		memcpy(_bits + _position, buffer, size);

		_position += size;
	}
}

void CBuffer::writeString(const CString& value)
{
	write(value.c_str(), value.getLength() + 1);
}

void CBuffer::invalidate()
{
	_bits = NULL;
	_capacity = _currentLength = _position = 0;
}

void CBuffer::reserve(size_t newLength)
{
	_currentLength = newLength;
	size_t newMaxLength = DATA_BLOCK_SIZE * ((_currentLength + DATA_BLOCK_SIZE) / DATA_BLOCK_SIZE);

	if (_capacity < newMaxLength)
	{
		_capacity = newMaxLength;
		_bits = (unsigned char*)realloc(_bits, _capacity);

		if (!_bits)
		{
			invalidate();
			return;
		}
	}
}
