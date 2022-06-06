#ifndef _mpu_h_#define _mpu_h_#include "EEPROM.h"#include "hardware_serial.h"#include "timer.h"#include "wiring.h"#define EXTERNAL_INT_0 0
#define EXTERNAL_INT_1 1#define NUM_OF_EXTERNAL_INTERRUPTS 2typedef void (*voidFuncPtr)(void);#define CHANGE 1
#define FALLING 2
#define RISING 3typedef uint8_t byte;#define LSBFIRST 0#define MSBFIRST 1void init(void);void attachInterrupt(uint8_t interruptNum, void (*userFunc)(void), int mode);uint16_t makeWord(byte h, byte l);void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);#endif