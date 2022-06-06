#ifndef _hardware_serial_h_
#define _hardware_serial_h_

#include <avr/io.h>

#include <inttypes.h>
#include <stddef.h>

struct ring_buffer;

class CHardwareSerial
{
public:
	CHardwareSerial(ring_buffer* rx_buffer, ring_buffer* tx_buffer,
      volatile uint8_t* ubrrh, volatile uint8_t* ubrrl,
      volatile uint8_t* ucsra, volatile uint8_t* ucsrb,
      volatile uint8_t* ucsrc, volatile uint8_t* udr,
      uint8_t rxen, uint8_t txen, uint8_t rxcie, uint8_t udrie, uint8_t u2x);
	
	int available();
	
	void begin(unsigned long);
	
	int read();
	
	size_t write(uint8_t);
	
 protected:
    ring_buffer* _rx_buffer;
    ring_buffer* _tx_buffer;
    volatile uint8_t* _ubrrh;
    volatile uint8_t* _ubrrl;
    volatile uint8_t* _ucsra;
    volatile uint8_t* _ucsrb;
    volatile uint8_t* _ucsrc;
    volatile uint8_t* _udr;
    uint8_t _rxen;
    uint8_t _txen;
    uint8_t _rxcie;
    uint8_t _udrie;
    uint8_t _u2x;
    bool transmitting;
};

#if defined(UBRRH) || defined(UBRR0H)
extern CHardwareSerial Serial;
#endif

#endif
