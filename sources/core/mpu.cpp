#include "mpu.h"
#include "hardware_serial.h"
static volatile voidFuncPtr intFunc[NUM_OF_EXTERNAL_INTERRUPTS];

void init(void)
{
	// switch interrupts on
	sei();
	
	// set timer 0 prescale factor to 64
#if defined(TCCR0) && defined(CS01) && defined(CS00)
	// this combination is for the atmega323
	sbi(TCCR0, CS01);
	sbi(TCCR0, CS00);
#else
	#error Timer 0 prescale factor 64 not set correctly
#endif

	// enable timer 0 overflow interrupt
#if defined(TIMSK) && defined(TOIE0)
	sbi(TIMSK, TOIE0);
#else
	#error Timer 0 overflow interrupt not set correctly
#endif

#if defined(ADCSRA)
	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// This will not work properly for other clock speeds, and this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);
	sbi(ADCSRA, ADPS0);

	// enable a2d conversions
	sbi(ADCSRA, ADEN);
#endif


}

void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode)
{
	if (interruptNum >= NUM_OF_EXTERNAL_INTERRUPTS)
		return;
		
	intFunc[interruptNum] = userFunc;
	
	switch (interruptNum)
	{
		case EXTERNAL_INT_0:
			#if defined(MCUCR) && defined(ISC00) && defined(GICR)
			MCUCR = (MCUCR & ~((1 << ISC00) | (1 << ISC01))) | (mode << ISC00);
			GICR |= (1 << INT0);
			
			#else
			#error attachInterrupt not finished for this CPU (case EXTERNAL_INT_0)
			
			#endif
			
			break;
			
		case EXTERNAL_INT_1:
			#if defined(MCUCR) && defined(ISC10) && defined(ISC11) && defined(GICR)
			MCUCR = (MCUCR & ~((1 << ISC10) | (1 << ISC11))) | (mode << ISC10);
			GICR |= (1 << INT1);
			
			#else
			#error attachInterrupt not finished for this CPU (case EXTERNAL_INT_1)
			
			#endif
			
			break;
	}
}

uint16_t makeWord(byte h, byte l)
{
	return (h << 8) | l;
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
	uint8_t i;

	for (i = 0; i < 8; ++i) 
	{
		if (bitOrder == LSBFIRST)
			digitalWrite(dataPin, !!(val & (1 << i)));
		else	
			digitalWrite(dataPin, !!(val & (1 << (7 - i))));
			
		digitalWrite(clockPin, HIGH);
		digitalWrite(clockPin, LOW);		
	}
}

ISR(INT0_vect)
{
	if(intFunc[EXTERNAL_INT_0])
		intFunc[EXTERNAL_INT_0]();
}

ISR(INT1_vect)
{
	if(intFunc[EXTERNAL_INT_1])
		intFunc[EXTERNAL_INT_1]();
}


