#include "time.h"

CTime::CTime(const uint16_t yr, const uint8_t mon, const uint8_t date,
const uint8_t hr, const uint8_t min, const uint8_t sec, const Day day)
{
	this->yr   = yr;
	this->mon  = mon;
	this->date = date;
	this->hr   = hr;
	this->min  = min;
	this->sec  = sec;
	this->day  = day;
}
