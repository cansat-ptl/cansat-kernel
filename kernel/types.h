/*
 * types.h
 *
 * Created: 12.05.2019 18:36:04
 *  Author: ThePetrovich
 */ 

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

#define CMD_MAX_WORD_SIZE 16

typedef char cmdWord[CMD_MAX_WORD_SIZE];
typedef int (*task)(void);
typedef void (*timerISR)(void);
typedef void (*cmdHandler)();
typedef uint8_t byte;

struct taskStruct {
	task pointer;
	uint16_t delay;
	uint16_t repeatPeriod;
	uint8_t priority;
	uint8_t state;
};

struct timerStruct {
	timerISR tsrPointer;
	uint32_t period;
	uint32_t repeatPeriod;
};

struct commandStruct {
	cmdWord keyword;
	cmdHandler handler;
	uint8_t length;
};

#endif /* TYPES_H_ */