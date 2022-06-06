#include "countdown_timer.h"

#include "timer.h"

CCountdownTimer::CCountdownTimer()
: _timerStart(0), _timerInterval(0)
{
}

bool CCountdownTimer::isFinished()
{
	return millis() - _timerStart >= _timerInterval;
}

void CCountdownTimer::start(unsigned long interval)
{
	_timerStart = millis();
	_timerInterval = interval;
}

void CCountdownTimer::stop()
{
	_timerInterval = 0;
}
