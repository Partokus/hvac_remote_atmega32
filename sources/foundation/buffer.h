#ifndef _buffer_h_
#define _buffer_h_

#include "string.h"

#define READ_VARIABLE(x) read(&x, sizeof(x))
#define WRITE_VARIABLE(x) write(&x, sizeof(x))

class CBuffer
{
public:
	CBuffer(size_t length = 0);
	~CBuffer();

	size_t getLength() const { return _currentLength; }
	size_t getPosition() const { return _position; }
	bool eof() const { return _position >= _currentLength; }
	unsigned char* getBits(size_t offset = 0) const;

	void clear();
	size_t seek(int offset, int origin);
	size_t seekAbs(size_t offset);

	size_t read(void* buffer, size_t size);
	CString readLine();
	CString readString();
	CString readText();
	CString readWord();

	void write(const void* buffer, size_t size);
	void writeString(const CString& string);

protected:
	void invalidate();

	void reserve(size_t newLength);

	unsigned char* _bits;
	size_t _capacity, _currentLength, _position;
};

#endif
