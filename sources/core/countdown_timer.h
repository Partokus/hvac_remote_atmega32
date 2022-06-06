#ifndef _countdown_timer_h_
#define _countdown_timer_h_

class CCountdownTimer
{
public:
	CCountdownTimer();
	
	bool isFinished();
	void start(unsigned long interval);
	void stop();

protected:
	unsigned long _timerStart, _timerInterval;
		
	friend class CFramework;
};

#endif
