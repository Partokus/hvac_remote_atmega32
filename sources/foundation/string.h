#ifndef _string_h_
#define _string_h_

#include <stddef.h>

class CString
{
public:
	CString();
	CString(const char* strB);
	CString(const CString& strB);
	CString(int num);
	~CString();

	operator const char* () const { return _content; }
	const char* c_str() const { return _content; }
	size_t getLength() const { return _currentLength; }

	CString   operator +(const CString& strB) const;
	CString   operator +(const char* strB) const;
	CString&  operator =(const CString& strB);
	CString&  operator +=(const CString& strB);
	bool      operator ==(const CString& strB) const;
	bool      operator ==(const char* strB) const;
	char&     operator [](size_t index) const;

	bool      compare(const CString& strB) const;
	bool      compareNoCase(const CString& strB) const;
	CString   extractWord(size_t index) const;
	int       find(const CString& pattern, size_t beginIndex) const;
	int       find(char ch, size_t beginIndex) const;
	CString&  format(const char* str, ...);
	size_t    replace(char oldChar, char newChar);
	CString   substring(size_t beginIndex, size_t endIndex) const;
	bool      toBool() const;
	float     toFloat() const;
	int       toInt() const;

protected:
	void append(const char* strB);

	void init();

	void invalidate();
	
	void reserve(size_t newLength);

	size_t _capacity, _currentLength;

	char* _content;
};

#endif
