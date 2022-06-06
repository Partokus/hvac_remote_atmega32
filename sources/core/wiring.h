#ifndef _wiring_h_
#define _wiring_h_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>

#define NUM_OF_ELEMENTS(x) (sizeof(x) / sizeof(x[0]))

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define NOT_A_PROGRAMMABLE_PIN 0
#define PA 1
#define PB 2
#define PC 3
#define PD 4

extern const uint8_t SCL;
extern const uint8_t SDA;

extern const uint8_t PROGMEM pin_to_port_PGM[];
extern const uint8_t PROGMEM pin_to_bit_mask_PGM[];

#define pinToPort(P) ( pgm_read_byte( pin_to_port_PGM + (P) ) )
#define pinToBitMask(P) ( pgm_read_byte( pin_to_bit_mask_PGM + (P) ) )

extern const uint16_t PROGMEM port_to_mode_PGM[];
extern const uint16_t PROGMEM port_to_input_PGM[];
extern const uint16_t PROGMEM port_to_output_PGM[];

#define portOutputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_output_PGM + (P))) )
#define portInputRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_input_PGM + (P))) )
#define portModeRegister(P) ( (volatile uint8_t *)( pgm_read_word( port_to_mode_PGM + (P))) )

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

void pinMode(uint8_t, uint8_t);
void digitalWrite(uint8_t, uint8_t);
int digitalRead(uint8_t);

#define EXTERNAL 0
#define DEFAULT 1

uint8_t analogPinToChannel(uint8_t);
void analogReference(uint8_t);
int analogRead(uint8_t);

uint8_t digitalPinToInterrupt(uint8_t);

#endif
