#include "string.h"

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (defined _MSC_VER && _MSC_VER < 1400)
#define vsnprintf _vsnprintf
#endif

const int STR_BLOCK_SIZE = 30;

CString::CString()
{
	init();
}

CString::CString(const char* strB)
{
	init();
	
	if (strB)
	{
		reserve(strlen(strB));
		
		if (_content)
			strcpy(_content, strB);
	}
}

CString::CString(const CString& value)
{
	init();

	*this = value;
}

CString::CString(int num)
{
	char buffer[12]; // -2147483647

	init();

	itoa(num, buffer, 10);
	
	*this = buffer;
}

CString::~CString()
{
	free(_content);
}

CString CString::operator +(const CString& strB) const
{
	CString strA(*this);
	strA.append(strB._content);
	return strA;
}

CString CString::operator +(const char* strB) const
{
	CString strA(*this);
	strA.append(strB);
	return strA;
}

CString& CString::operator =(const CString& strB)
{
	if (&strB == this) return *this;

	reserve(strB._currentLength);

	if (_content && strB._content)
	{
		strcpy(_content, strB._content);
	}
	
	return *this;
}

CString& CString::operator +=(const CString& strB)
{
	append(strB._content);
	return *this;
}

bool CString::operator ==(const CString& strB) const
{
	if (!_content || !strB._content)
		return false;

	return strcmp(_content, strB._content) == 0;
}

bool CString::operator ==(const char* strB) const
{
	if (!_content)
		return false;

	return strcmp(_content, strB) == 0;
}

char& CString::operator [](size_t index) const
{
	static char dummyWritableChar;

	if (index >= _currentLength)
	{
		dummyWritableChar = 0;
		
		return dummyWritableChar;
	}

	return _content[index];
}

bool CString::compare(const CString& strB) const
{
	return *this == strB;
}

bool CString::compareNoCase(const CString& strB) const
{
	int f, l;

	if (!_content || !strB._content)
		return false;

	unsigned char* dst = (unsigned char*)_content;
	unsigned char* src = (unsigned char*)strB._content;

	do
	{
		f = tolower(*(dst++));
		l = tolower(*(src++));
	}
	while (f && (f == l));

	return f == l;
}

CString CString::extractWord(size_t index) const
{
	size_t i = 0, j = 0, k = 0;

	for (; i < _currentLength; ++i)
	{
		if (isspace((unsigned char)_content[i]) || _content[i] == ',')
			continue;

		for (j = i; j < _currentLength - 1; ++j)
			if (isspace((unsigned char)_content[j + 1]) || _content[j + 1] == ',')
				break;

		if (k == index)
			break;

		i = j + 1;
		++k;
	}

	return substring(i, j);
}

int CString::find(const CString& pattern, size_t beginIndex) const
{
	if (beginIndex >= _currentLength)
		return -1;

	char* p = strstr(_content + beginIndex, pattern._content);

	if (!p)
		return -1;

	return (int)(p - _content);
}

int CString::find(char ch, size_t beginIndex) const
{
	if (beginIndex >= _currentLength)
		return -1;

	char* p = strchr(_content + beginIndex, ch);

	if (!p)
		return -1;

	return (int)(p - _content);
}

CString& CString::format(const char* str, ...)
{
	va_list args;
	char buffer[100];

	va_start(args, str);
	vsnprintf(buffer, sizeof(buffer), str, args);
	va_end(args);

	reserve(strlen(buffer));

	if (_content)
		strcpy(_content, buffer);
	
	return *this;
}

size_t CString::replace(char oldChar, char newChar)
{
	size_t i;

	size_t count = 0;

	for (i = 0; i < _currentLength; ++i)
	{
		if (_content[i] == oldChar)
		{
			_content[i] = newChar;
			++count;
		}
	}

	return count;
}

CString CString::substring(size_t beginIndex, size_t endIndex) const
{
	size_t i, j;
	CString str;

	if (beginIndex < _currentLength && endIndex < _currentLength && beginIndex <= endIndex)
	{
		str.reserve(endIndex - beginIndex + 1);

		if (str._content)
		{
			for (i = beginIndex, j = 0; i <= endIndex; ++i, ++j)
				str._content[j] = _content[i];
			
			str._content[j] = '\0';
		}
	}

	return str;
}

bool CString::toBool() const
{
	if (compareNoCase("true"))
		return true;

	return toInt() != 0;
}

float CString::toFloat() const
{
	if (_content)
		return (float)atof(_content);

	return 0.0f;
}

int CString::toInt() const
{
	if (_content)
		return atoi(_content);

	return 0;
}

void CString::append(const char* strB)
{
	if (!strB)
		return;

	reserve(_currentLength + strlen(strB));

	if (!_content)
		return;

	strcat(_content, strB);
}

void CString::init()
{
	_content = NULL;
	_capacity = _currentLength = 0;
}

void CString::invalidate()
{
	if (_content)
		free(_content);

	_content = NULL;
	_capacity = _currentLength = 0;
}

void CString::reserve(size_t newLength)
{
	_currentLength = newLength;
	size_t newMaxLength = STR_BLOCK_SIZE * ((_currentLength + STR_BLOCK_SIZE) / STR_BLOCK_SIZE);

	if (_capacity < newMaxLength)
	{
		_capacity = newMaxLength;
		_content = (char*)realloc(_content, _capacity);

		if (!_content)
		{
			invalidate();
			return;
		}
	}
}
