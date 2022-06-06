#include "framework.h"

#include <math.h>
#include <string.h>

#include "core/mpu.h"

CFramework::CFramework(
	InputState* inputs,
	const uint8_t* inputPins_PGM,
	int numOfInputs,
	const uint8_t* outputs_PGM,
	int numOfOutputs,
	const TimedInput* timedInputs_PGM,
	int numOfTimedInputs,
	CCountdownTimer* timers,
	int numOfTimers
)
: _inputs(inputs), _inputPins_PGM(inputPins_PGM), _numOfInputs(numOfInputs),
_outputs_PGM(outputs_PGM), _numOfOutputs(numOfOutputs),
_timedInputs_PGM(timedInputs_PGM), _numOfTimedInputs(numOfTimedInputs),
_timers(timers), _numOfTimers(numOfTimers)
{
}

void CFramework::init()
{
	::init(); // /core/mpu.h
}

float CFramework::readThermistorTemp(uint8_t pin, float Sr, float R0, float T0, float B, bool pullup)
{
	int t = analogRead(pin);
	
	if (t <= 0 || t >= 1023)
		return INVALID_TEMP;
	
	float tr = 1023.0 / t - 1;
	
	if (pullup)
		tr = Sr / tr; // Serial resistor is connected with Vcc
	else
		tr = Sr * tr; // Serial resistor is connected with GND
	
	float steinhart;
	steinhart = tr / R0; // (R/Ro)
	steinhart = log(steinhart); // ln(R/Ro)
	steinhart /= B; // 1/B * ln(R/Ro)
	steinhart += 1.0 / (T0 + 273.15f); // + (1/To)
	steinhart = 1.0 / steinhart; // invert
	steinhart -= 273.15f;
	
	return steinhart;
}

bool CFramework::load(int address, uint8_t* buffer, size_t size)
{
	if (EEPROM.read(address) != 0x5a)
	{
		memset(buffer, 0, size);
		return false;
	}
	else
	{
		EEPROM.read(address + 1, buffer, size);	
		return true;
	}
}

void CFramework::save(int address, const uint8_t* buffer, size_t size)
{
	EEPROM.write(address, 0x5a);
	EEPROM.write(address + 1, buffer, size);
}

void CFramework::setup()
{
	int i, j;
	
	for (i = 0, j = 0; i < _numOfInputs; ++i, j += 2)
	{
		_inputs[i].pin = pgm_read_byte(_inputPins_PGM + j);
		_inputs[i].mode = pgm_read_byte(_inputPins_PGM + j + 1);
		pinMode(_inputs[i].pin, _inputs[i].mode);
	}
	
	delayMicroseconds(10);
	
	unsigned long timestamp = millis();
	
	for (i = 0; i < _numOfInputs; ++i)
	{
		_inputs[i].state = digitalRead(_inputs[i].pin);
		_inputs[i].changed = false;
		_inputs[i].skipNextEvent = false;
		_inputs[i].timestamp = timestamp;
	}
	
	for (i = 0; i < _numOfOutputs; ++i)
		pinMode(pgm_read_byte(_outputs_PGM + i), OUTPUT);
}

void CFramework::loop()
{
	
	
	if (_delayTimer.isFinished())
	{
		readInputs();
		processInputs();
		processTimedInputs();
		processTimers();
		_delayTimer.start(15);
	}
}

bool CFramework::isButtonDown(int input) const
{
	return _inputs[input].mode == INPUT_PULLUP ? _inputs[input].state == LOW : _inputs[input].state == HIGH;
}

void CFramework::readInputs()
{
	int state;
	
	unsigned long timestamp = millis();
	
	for (int i = 0; i < _numOfInputs; ++i)
	{
		state = digitalRead(_inputs[i].pin);
		
		if (_inputs[i].state != state)
		{
			_inputs[i].state = state;
			_inputs[i].changed = true;
			_inputs[i].timestamp = timestamp;
		}
	}
}

void CFramework::processInputs()
{
	unsigned long timestamp = millis();
	
	for (int i = 0; i < _numOfInputs; ++i)
	{
		if (_inputs[i].changed)
		{
			if (!_inputs[i].skipNextEvent)
				inputEventRun(i, &_inputs[i], timestamp - _inputs[i].timestamp);
			
			_inputs[i].changed = false;
			_inputs[i].skipNextEvent = false;
		}
	}
}

void CFramework::processTimedInputs()
{
	TimedInput timedInput;
	
	unsigned long timestamp = millis();
		
	for (int i = 0; i < _numOfTimedInputs; ++i)
	{
		memcpy_P(&timedInput, _timedInputs_PGM + i, sizeof(timedInput));
		
		if (_inputs[timedInput.input].skipNextEvent)
			continue;
		
		if (!isButtonDown(timedInput.input))
			continue;
		
		if (!(timestamp - _inputs[timedInput.input].timestamp >= timedInput.timer))
			continue;
		
		_inputs[timedInput.input].skipNextEvent = timedInputEventRun(timedInput.input);
	}
}

void CFramework::processTimers()
{
	CCountdownTimer* timer;
	
	unsigned long timestamp = millis();
	
	for (int i = 0; i < _numOfTimers; ++i)
	{
		timer = &_timers[i];
		
		if (timer->_timerInterval == 0)
			continue;
		
		if (!(timestamp - timer->_timerStart >= timer->_timerInterval))
			continue;
		
		if (timerEventRun(i))
			timer->start(timer->_timerInterval);
		else
			timer->_timerInterval = 0;
	}
}
