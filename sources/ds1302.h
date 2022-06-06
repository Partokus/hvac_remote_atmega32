#ifndef DS1302_H_
#define DS1302_H_

#include "foundation/time.h"

/*
An interface to the Dallas Semiconductor DS1302 Real Time Clock (RTC) chip.
*/

class CDS1302
{
public:
	// Size of the DS1302's RAM storage, in bytes.
	static const int kRamSize = 31;
	
	CDS1302(uint8_t ce_pin, uint8_t io_pin, uint8_t sclk_pin);
	
	/*
	Disables write protection if enabled.
	Sets 24-hour mode if running in 12-hour mode.
	Clears Clock Halt flag if set.
	*/
	void begin();

	/*
	Enables or disables write protection on the chip.
	While write protection is enabled, all attempts to write to the chip (e.g., setting the time) will have no effect.
	The DS1302 datasheet does not define the initial state of write protection,
	so this method should be called at least once when initializing a device for the first time.
	*/
	void writeProtect(bool enable);

	/*
	Sets or clears Clock Halt flag on the chip.
	Enabling the Clock Halt flag disables the DS1302's clock oscillator and places it into a low-power standby mode.
	While in this mode, the time does not progress.
	The time can still be read from the chip while it is halted, however.
	
	The DS1302 datasheet does not define the initial state of the Clock Halt flag,
	so this method should be called at least once when initializing a device for the first time.
	*/
	void halt(bool value);

	// Returns the current time and date in a CTime object.
	CTime time();

	// Sets the time and date to the instant specified in a given CTime object.
	void time(CTime t);

	/*
	Writes a byte to RAM.
	The DS1302 has 31 bytes (kRamSize) of static RAM that can store arbitrary data as long as the device has power.
	Writes to invalid addresses have no effect.
	*/
	void writeRam(uint8_t address, uint8_t value);

	// Reads a byte from RAM.
	// Reads of invalid addresses return 0.
	uint8_t readRam(uint8_t address);

	// Writes 'len' bytes into RAM from '*data', starting at RAM address 0.
	void writeRamBulk(const uint8_t* data, int len);

	// Reads 'len' bytes from RAM into '*data', starting at RAM address 0.
	void readRamBulk(uint8_t* data, int len);

	// Reads register byte value.
	uint8_t readRegister(uint8_t reg);

	// Writes byte into register.
	void writeRegister(uint8_t reg, uint8_t value);

private:
	uint8_t _ce_pin;
	uint8_t _io_pin;
	uint8_t _sclk_pin;

	// Shifts out a value to the IO pin.
	// Side effects: sets _io_pin as OUTPUT.
	void writeOut(uint8_t value);

	// Reads in a byte from the IO pin.
	// Side effects: sets _io_pin to INPUT.
	uint8_t readIn();
};

#endif
