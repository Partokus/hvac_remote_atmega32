#include "wiring.h"

void pinMode(uint8_t pin, uint8_t mode)
{
	uint8_t port = pinToPort(pin);

	if (port == NOT_A_PROGRAMMABLE_PIN)
		return;
	
	volatile uint8_t *reg = portModeRegister(port);
	volatile uint8_t *out = portOutputRegister(port);
	
	uint8_t bit = pinToBitMask(pin);

	if (mode == INPUT)
	{ 
		uint8_t oldSREG = SREG;
        cli();
		
		*reg &= ~bit;
		*out &= ~bit;
		
		SREG = oldSREG;
	}
	else if (mode == INPUT_PULLUP)
	{
		uint8_t oldSREG = SREG;
        cli();
		
		*reg &= ~bit;
		*out |= bit;
		
		SREG = oldSREG;
	}
	else
	{
		uint8_t oldSREG = SREG;
        cli();
		
		*reg |= bit;
		
		SREG = oldSREG;
	}
}

void digitalWrite(uint8_t pin, uint8_t val)
{
	uint8_t port = pinToPort(pin);
	
	if (port == NOT_A_PROGRAMMABLE_PIN)
		return;
	
	volatile uint8_t *out = portOutputRegister(port);
	
	uint8_t bit = pinToBitMask(pin);

	uint8_t oldSREG = SREG;
	cli();

	if (val == LOW)
	{
		*out &= ~bit;
	} else
	{
		*out |= bit;
	}

	SREG = oldSREG;
}

int digitalRead(uint8_t pin)
{
	uint8_t port = pinToPort(pin);

	if (port == NOT_A_PROGRAMMABLE_PIN)
		return LOW;
	
	uint8_t bit = pinToBitMask(pin);

	if (*portInputRegister(port) & bit)
		return HIGH;
	
	return LOW;
}

uint8_t analog_reference = DEFAULT;

void analogReference(uint8_t mode)
{
	analog_reference = mode;
}

int analogRead(uint8_t pin)
{
	uint8_t low, high;
	
	pin = analogPinToChannel(pin);
	
	// Set the analog reference (high two bits of ADMUX) and select the channel (low 4 bits).
	// This also sets ADLAR (left-adjust result) to 0 (the default).
#if defined(ADMUX)
	ADMUX = (analog_reference << 6) | (pin & 0x07);
#endif

#if defined(ADCSRA) && defined(ADCL)
	// start the conversion
	sbi(ADCSRA, ADSC);

	// ADSC is cleared when the conversion finishes
	while (bit_is_set(ADCSRA, ADSC));

	// We have to read ADCL first;
	// doing so locks both ADCL and ADCH until ADCH is read.
	// Reading ADCL second would cause the results of each conversion to be discarded,
	// as ADCL and ADCH would be locked when it completed.
	low  = ADCL;
	high = ADCH;
#else
	// we don't have an ADC, return 0
	low  = 0;
	high = 0;
#endif

	// combine the two bytes
	return (high << 8) | low;
}
