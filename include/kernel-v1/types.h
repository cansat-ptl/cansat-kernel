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

typedef char kv1CmdWord_t[CMD_MAX_WORD_SIZE];
typedef struct kv1TaskStruct_t* kv1Call_t;
typedef struct kv1TaskStruct_t* kv1TaskHandle_t;
typedef struct kv1TimerStruct_t* kv1TimerHandle_t;
typedef void (*kv1TimerISR_t)(void);
typedef void (*kv1CmdHandler_t)(void);
typedef uint8_t byte;

struct kv1TaskStruct_t {
	void (*pointer)(void);
	uint16_t delay;
	uint16_t repeatPeriod;
	uint8_t priority;
	uint8_t state;
};

struct kv1TimerStruct_t {
	kv1TimerISR_t tsrPointer;
	uint32_t period;
	uint32_t repeatPeriod;
};

struct kCommandStruct_t {
	kv1CmdWord_t keyword;
	kv1CmdHandler_t handler;
	uint8_t length;
};

#endif /* TYPES_H_ */