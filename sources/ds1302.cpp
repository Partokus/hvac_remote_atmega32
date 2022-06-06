#include "DS1302.h"

#include "core/mpu.h"

namespace
{
	enum Register
	{
		REGISTER_SECOND = 0,
		REGISTER_MINUTE = 1,
		REGISTER_HOUR = 2,
		REGISTER_DATE = 3,
		REGISTER_MONTH = 4,
		REGISTER_DAY = 5,
		REGISTER_YEAR = 6,
		REGISTER_WRITE_PROTECT = 7,

		// The RAM register space follows the clock register space.
		REGISTER_RAM_ADDRESS_0 = 32
	};

	enum Command
	{
		CLOCK_BURST_READ  = 0xBF,
		CLOCK_BURST_WRITE = 0xBE,
		RAM_BURST_READ    = 0xFF,
		RAM_BURST_WRITE   = 0xFE
	};

	// Establishes and terminates a three-wire SPI session.
	class SPISession
	{
	public:
		SPISession(const int ce_pin, const int io_pin, const int sclk_pin)
		: _ce_pin(ce_pin), _io_pin(io_pin), _sclk_pin(sclk_pin)
		{
			digitalWrite(_sclk_pin, LOW);
			digitalWrite(_ce_pin, HIGH);
			delayMicroseconds(4); // tCC
		}
		~SPISession()
		{
			digitalWrite(_ce_pin, LOW);
			delayMicroseconds(4); // tCWH
		}

	private:
		const int _ce_pin;
		const int _io_pin;
		const int _sclk_pin;
	};

	/*
	Returns the decoded decimal value from a binary-coded decimal (BCD) byte.
	Assumes 'bcd' is coded with 4-bits per digit, with the tens place digit in the upper 4 MSBs.
	*/
	uint8_t bcdToDec(const uint8_t bcd)
	{
		return (10 * ((bcd & 0xF0) >> 4) + (bcd & 0x0F));
	}

	// Returns the binary-coded decimal of 'dec'. Inverse of bcdToDec.
	uint8_t decToBcd(const uint8_t dec)
	{
		const uint8_t tens = dec / 10;
		const uint8_t ones = dec % 10;
		return (tens << 4) | ones;
	}

	// Returns the hour in 24-hour format from the hour register value.
	uint8_t hourFromRegisterValue(const uint8_t value)
	{
		uint8_t adj;
		if (value & 128)  // 12-hour mode
			adj = 12 * ((value & 32) >> 5);
		else           // 24-hour mode
			adj = 10 * ((value & (32 + 16)) >> 4);
		return (value & 15) + adj;
	}
} // namespace

CDS1302::CDS1302(const uint8_t ce_pin, const uint8_t io_pin, const uint8_t sclk_pin)
{
	_ce_pin = ce_pin;
	_io_pin = io_pin;
	_sclk_pin = sclk_pin;

	pinMode(ce_pin, OUTPUT);
	pinMode(sclk_pin, OUTPUT);
}

void CDS1302::begin()
{
	const uint8_t reg_list[] = { REGISTER_WRITE_PROTECT /*Remove write protection*/, REGISTER_HOUR, REGISTER_SECOND };
	uint8_t reg_value;
	
	for (size_t i = 0; i < NUM_OF_ELEMENTS(reg_list); ++i)
	{
		reg_value = readRegister(reg_list[i]);
		
		if (reg_value & (1 << 7))
		{
			reg_value &= ~(1 << 7);
			writeRegister(reg_list[i], reg_value);
		}
	}
}

void CDS1302::writeOut(const uint8_t value)
{
	pinMode(_io_pin, OUTPUT);
	/*
	This assumes that shiftOut is 'slow' enough for the DS1302 to read the bits.
	The datasheet specifies that SCLK must be in its high and low states for at least 0.25us at 5V or 1us at 2V.
	Experimentally, a 16MHz Arduino seems to spend ~4us high and ~12us low when shifting.
	*/
	shiftOut(_io_pin, _sclk_pin, LSBFIRST, value);
}

uint8_t CDS1302::readIn()
{
	uint8_t input_value = 0;
	uint8_t bit = 0;
	pinMode(_io_pin, INPUT);

	/*
	Bits from the DS1302 are output on the falling edge of the clock cycle.
	This method is called after a previous call to writeOut() or readIn(), which will have already set the clock low.
	*/
	for (int i = 0; i < 8; ++i)
	{
		bit = digitalRead(_io_pin);
		input_value |= (bit << i);  // Bits are read LSB first.

		// See the note in writeOut() about timing. digitalWrite() is slow enough to not require extra delays for tCH and tCL.
		digitalWrite(_sclk_pin, HIGH);
		digitalWrite(_sclk_pin, LOW);
	}

	return input_value;
}

uint8_t CDS1302::readRegister(const uint8_t reg)
{
	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	const uint8_t cmd_byte = (0x81 | (reg << 1));
	writeOut(cmd_byte);
	return readIn();
}

void CDS1302::writeRegister(const uint8_t reg, const uint8_t value)
{
	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	const uint8_t cmd_byte = (0x80 | (reg << 1));
	writeOut(cmd_byte);
	writeOut(value);
}

void CDS1302::writeProtect(const bool enable)
{
	writeRegister(REGISTER_WRITE_PROTECT, (enable << 7));
}

void CDS1302::halt(const bool enable)
{
	uint8_t sec = readRegister(REGISTER_SECOND);
	sec &= ~(1 << 7);
	sec |= (enable << 7);
	writeRegister(REGISTER_SECOND, sec);
}

CTime CDS1302::time()
{
	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	CTime t(2099, 1, 1, 0, 0, 0, CTime::SUNDAY);
	writeOut(CLOCK_BURST_READ);
	t.sec = bcdToDec(readIn() & 0x7F);
	t.min = bcdToDec(readIn());
	t.hr = hourFromRegisterValue(readIn());
	t.date = bcdToDec(readIn());
	t.mon = bcdToDec(readIn());
	t.day = static_cast<CTime::Day>(bcdToDec(readIn()));
	t.yr = 2000 + bcdToDec(readIn());
	return t;
}

void CDS1302::time(const CTime t)
{
	// We want to maintain the Clock Halt flag if it is set.
	const uint8_t ch_value = readRegister(REGISTER_SECOND) & 0x80;

	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	writeOut(CLOCK_BURST_WRITE);
	writeOut(ch_value | decToBcd(t.sec));
	writeOut(decToBcd(t.min));
	writeOut(decToBcd(t.hr));
	writeOut(decToBcd(t.date));
	writeOut(decToBcd(t.mon));
	writeOut(decToBcd(static_cast<uint8_t>(t.day)));
	writeOut(decToBcd(t.yr - 2000));
	// All clock registers *and* the WP register have to be written for the time to be set.
	writeOut(0);  // Write protection register.
}

void CDS1302::writeRam(const uint8_t address, const uint8_t value)
{
	if (address >= kRamSize)
		return;

	writeRegister(REGISTER_RAM_ADDRESS_0 + address, value);
}

uint8_t CDS1302::readRam(const uint8_t address)
{
	if (address >= kRamSize)
		return 0;

	return readRegister(REGISTER_RAM_ADDRESS_0 + address);
}

void CDS1302::writeRamBulk(const uint8_t* const data, int len)
{
	if (len <= 0)
		return;
	
	if (len > kRamSize)
		len = kRamSize;

	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	writeOut(RAM_BURST_WRITE);
	for (int i = 0; i < len; ++i)
		writeOut(data[i]);
}

void CDS1302::readRamBulk(uint8_t* const data, int len)
{
	if (len <= 0)
		return;
	
	if (len > kRamSize)
		len = kRamSize;

	const SPISession s(_ce_pin, _io_pin, _sclk_pin);

	writeOut(RAM_BURST_READ);
	for (int i = 0; i < len; ++i)
		data[i] = readIn();
}
