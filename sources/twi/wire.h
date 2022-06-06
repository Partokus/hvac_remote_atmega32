#ifndef TwoWire_h
#define TwoWire_h

#include <inttypes.h>
#include <stdlib.h>

#define BUFFER_LENGTH 32

class TwoWire
{
private:
	static uint8_t rxBuffer[];
	static uint8_t rxBufferIndex;
	static uint8_t rxBufferLength;

	static uint8_t txAddress;
	static uint8_t txBuffer[];
	static uint8_t txBufferIndex;
	static uint8_t txBufferLength;

	static uint8_t transmitting;
	static void (*user_onRequest)();
	static void (*user_onReceive)(int);
	static void onRequestService();
	static void onReceiveService(uint8_t*, int);
	
public:
	TwoWire();
	
	void begin();
	void begin(uint8_t);
	void begin(int);
	void beginTransmission(uint8_t);
	void beginTransmission(int);
	uint8_t endTransmission();
	uint8_t endTransmission(uint8_t);
	uint8_t requestFrom(uint8_t, uint8_t);
	uint8_t requestFrom(uint8_t, uint8_t, uint8_t);
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *, size_t);
	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
	void onReceive( void (*)(int) );
	void onRequest( void (*)() );
	
	inline size_t write(unsigned long n) { return write((uint8_t)n); }
	inline size_t write(long n) { return write((uint8_t)n); }
	inline size_t write(unsigned int n) { return write((uint8_t)n); }
	inline size_t write(int n) { return write((uint8_t)n); }
};

extern TwoWire Wire;

#endif
