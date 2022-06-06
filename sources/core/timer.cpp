#include "timer.h"

#include "wiring.h"

// The prescaler is set so that timer0 ticks every 64 clock cycles,
// and the the overflow handler is called every 256 ticks.
#define MICROSECONDS_PER_TIMER0_OVERFLOW (clockCyclesToMicroseconds(64 * 256))

// the whole number of milliseconds per timer0 overflow
#define MILLIS_INC (MICROSECONDS_PER_TIMER0_OVERFLOW / 1000)

// The fractional number of milliseconds per timer0 overflow.
// We shift right by three to fit these numbers into a byte.
// (for the clock speeds we care about - 8 and 16 MHz - this doesn't lose precision.)
#define FRACT_INC ((MICROSECONDS_PER_TIMER0_OVERFLOW % 1000) >> 3)
#define FRACT_MAX (1000 >> 3)

volatile unsigned long timer0_overflow_count = 0;
volatile unsigned long timer0_millis = 0;
static unsigned char timer0_fract = 0;

ISR(TIMER0_OVF_vect)
{
	// copy these to local variables so they can be stored in registers (volatile variables must be read from memory on every access)
	unsigned long m = timer0_millis;
	unsigned char f = timer0_fract;

	m += MILLIS_INC;
	f += FRACT_INC;
	
	if (f >= FRACT_MAX)
	{
		f -= FRACT_MAX;
		m += 1;
	}

	timer0_fract = f;
	timer0_millis = m;
	timer0_overflow_count++;
}

unsigned long millis(void)
{
	uint8_t oldSREG = SREG;

	// disable interrupts while we read timer0_millis or we might get an inconsistent value (e.g. in the middle of a write to timer0_millis)
	cli();
	unsigned long m = timer0_millis;
	SREG = oldSREG;

	return m;
}

unsigned long micros(void)
{
	unsigned long m;
	uint8_t oldSREG = SREG, t;
	
	cli();
	m = timer0_overflow_count;
#if defined(TCNT0)
	t = TCNT0;
#else
	#error TIMER 0 not defined
#endif

#ifdef TIFR
	if ((TIFR & _BV(TOV0)) && (t < 255))
		m++;
#else
	#error TIMER 0 Interrupt Flag Register not defined
#endif

	SREG = oldSREG;
	
	return ((m << 8) + t) * (64 / clockCyclesPerMicrosecond());
}

void delay(unsigned long ms)
{
	uint16_t start = (uint16_t)micros();

	while (ms > 0)
	{
		if (((uint16_t)micros() - start) >= 1000)
		{
			--ms;
			start += 1000;
		}
	}
}

void delayMicroseconds(unsigned int us)
{
	// Calling avrlib's delay_us() function with low values (e.g. 1 or 2 microseconds) gives delays longer than desired.
	
#if F_CPU >= 20000000L
	// 20 MHz clock

	// For a one-microsecond delay, simply wait 2 cycle and return.
	// The overhead of the function call yields a delay of exactly a one microsecond.
	__asm__ __volatile__ (
		"nop" "\n\t"
		"nop"); //just waiting 2 cycle
	
	if (--us == 0)
		return;

	// The following loop takes a 1/5 of a microsecond (4 cycles) per iteration,
	// so execute it five times for each microsecond of delay requested.
	us = (us << 2) + us; // x5 us

	// account for the time taken in the preceeding commands...
	us -= 2;

#elif F_CPU >= 16000000L
	// 16 MHz clock

	// For a one-microsecond delay, simply return.
	// The overhead of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// The following loop takes a quarter of a microsecond (4 cycles) per iteration,
	// so execute it four times for each microsecond of delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands...
	us -= 2;
#else
	// 8 MHz internal clock

	// for a one- or two-microsecond delay, simply return.
	// The overhead of the function calls takes more than two microseconds.
	// Can't just subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;

	// The following loop takes half of a microsecond (4 cycles) per iteration,
	// so execute it twice for each microsecond of delay requested.
	us <<= 1;
    
	// partially compensate for the time taken by the preceeding commands...
	// We can't subtract any more than this or we'd overflow w/ small delays.
	us--;
#endif

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);
}
