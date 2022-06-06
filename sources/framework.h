#ifndef _framework_h_
#define _framework_h_

#include <inttypes.h>
#include <stddef.h>
#include "core/countdown_timer.h"

#define INVALID_TEMP (-1000)

struct InputState
{
	uint8_t pin, mode;
	int state;
	bool changed, skipNextEvent;
	unsigned long timestamp;
};

struct TimedInput
{
	uint8_t input;
	unsigned long timer;
};

class CFramework
{
public:
	CFramework(
		InputState* inputs,
		const uint8_t* inputPins_PGM,
		int numOfInputs,
		const uint8_t* outputs_PGM,
		int numOfOutputs,
		const TimedInput* timedInputs_PGM,
		int numOfTimedInputs,
		CCountdownTimer* timers,
		int numOfTimers
	);
	
	void init();
	
	float readThermistorTemp(uint8_t pin, float Sr, float R0, float T0, float B, bool pullup);
	
	void startTimer(int timerIdx, unsigned long interval) { _timers[timerIdx].start(interval); }
	void stopTimer(int timerIdx) { _timers[timerIdx].stop(); }
	bool isTimerFinished(int timerIdx) const { return _timers[timerIdx].isFinished(); }
	
	bool load(int address, uint8_t* buffer, size_t size);
	void save(int address, const uint8_t* buffer, size_t size);
	
	virtual void setup();
	virtual void loop();
	
protected:	
	bool isButtonDown(int input) const;
	
	void readInputs();
	void processInputs();
	void processTimedInputs();
	void processTimers();
	
	virtual void inputEventRun(int input, InputState* inputState, unsigned long duration) = 0;
	virtual bool timedInputEventRun(int input) = 0;
	virtual bool timerEventRun(int timerIdx) = 0;
	
	CCountdownTimer _delayTimer;
	InputState* _inputs;
	const uint8_t* _inputPins_PGM;
	int _numOfInputs;
	const uint8_t* _outputs_PGM;
	int _numOfOutputs;
	const TimedInput* _timedInputs_PGM;
	int _numOfTimedInputs;
	CCountdownTimer* _timers;
	int _numOfTimers;
};

#endif
