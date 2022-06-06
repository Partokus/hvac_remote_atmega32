#ifndef _connectivity_h_
#define _connectivity_h_

#include "buffer.h"


static unsigned short my_panid_t;
//static unsigned short my_panid;

class CConnectivity
{
public:
	CConnectivity();
	
	size_t getBufferSizeLimit() { return _bufferSizeLimit; }
	void setBufferSizeLimit(size_t bufferSizeLimit) { _bufferSizeLimit = bufferSizeLimit; }
	
	CBuffer* lockBuffer();
	void unlockBuffer();
	
	void send(const unsigned char* buffer, unsigned char size);
	void sendRawData(const unsigned char* buffer, unsigned char size);
	
	virtual void init() = 0;
	virtual bool host() = 0;
	virtual bool autoconnect() = 0;
	
protected:
	bool recv();
	void sendCommand(unsigned char command, unsigned short param = 0);
	
	virtual void commandEventRun(unsigned char command, unsigned short param) = 0;
	virtual int read() = 0;
	virtual void write(unsigned char c) = 0;
	virtual bool updateCommandEventRun() = 0;
	
	CBuffer _buffer;
	size_t _bufferSizeLimit;
	unsigned short _connection_id;
	size_t _updateCommandBufferSize;
	unsigned char _updateCommandBuffer[10];
	bool correct_panid; // lepeshkin

private:
	void proceedUpdateCommand(unsigned char c);
	
	bool parseCommand(unsigned char *bits, size_t length);
	void parseFrame();
	void proceedFrame();
	void resetFrame();

	unsigned short _id;
	unsigned char _payloadLength, _LRC;
	size_t _receivedBytes;
	CBuffer _receivingBuffer;
	int _receivingState;
};

#endif
