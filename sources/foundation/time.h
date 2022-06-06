#ifndef _time_h_
#define _time_h_

#include <stdint.h>

class CTime
{
public:
	enum Day
	{
		SUNDAY    = 1,
		MONDAY    = 2,
		TUESDAY   = 3,
		WEDNESDAY = 4,
		THURSDAY  = 5,
		FRIDAY    = 6,
		SATURDAY  = 7
	};

	/*
	Creates a Time object with a given time.
	
	Arguments:
	yr: year. Range: {2000, ..., 2099}.
	mon: month. Range: {1, ..., 12}.
	date: date (of the month). Range: {1, ..., 31}.
	hr: hour. Range: {0, ..., 23}.
	min: minutes. Range: {0, ..., 59}.
	sec: seconds. Range: {0, ..., 59}.
	day: day of the week. Sunday is 1. Range: {1, ..., 7}.
	*/
	
	CTime(uint16_t yr, uint8_t mon, uint8_t date, uint8_t hr, uint8_t min, uint8_t sec, Day day);

	uint8_t sec;
	uint8_t min;
	uint8_t hr;
	uint8_t date;
	uint8_t mon;
	Day day;
	uint16_t yr;
};

#endif
