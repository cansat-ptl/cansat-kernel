/*
 * kernel.h
 *
 * Created: 11.05.2019 21:15:24
 *  Author: ThePetrovich
 */ 

#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL_VER "0.6.0-bleeding"
#define KERNEL_TIMESTAMP __TIMESTAMP__

#define SDCARD_MOD_VER "0.0.5-rc1"
#define SDCARD_MOD_TIMESTAMP __TIMESTAMP__

#define CMD_MOD_VER "0.0.2-bleeding"
#define CMD_MOD_TIMESTAMP __TIMESTAMP__

#include <kernel-v1/types.h>
#include <kernel-v1/hal.h>
#include "kernel_config.h"
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define ERR_QUEUE_OVERFLOW 1
#define ERR_QUEUE_END 2
#define ERR_WDT_RESET 3
#define ERR_BOD_RESET 4
#define ERR_KRN_RETURN 5
#define ERR_DEVICE_FAIL 6
	
#define KPRIO_HIGH 0
#define KPRIO_NORM 1
#define KPRIO_LOW 2
	
#define KFLAG_INIT 0
#define KFLAG_TIMER_SET 1
#define KFLAG_TIMER_EN 2
#define KFLAG_TIMER_ISR 3
#define KFLAG_SD_INIT 4
#define KFLAG_HAL_ERROR 5
#define KFLAG_LOG_SD 13
#define KFLAG_LOG_UART 14
#define KFLAG_DEBUG 15
	
#define KSTATE_ACTIVE 1
#define KSTATE_SUSPENDED 0

#define KTASK_SINGLERUN 0
#define KTASK_REPEATED 1

#define utils_ARRAY_INDEX_FROM_ADDR(base, addr, type) (((uint16_t)(addr)-(uint16_t)(base))/sizeof(type))

void kernelv1_taskManager();
void kernelv1_taskService();

void kernelv1_setFlag(uint8_t flag, uint8_t value);
uint8_t kernelv1_checkFlag(uint8_t flag);
uint64_t kernelv1_getUptime();

uint8_t kernelv1_addCall(kv1Call_t t_ptr);
kv1TaskHandle_t kernelv1_addTask(uint8_t taskType, void* ptr, uint16_t period, uint8_t startupState);
uint8_t kernelv1_removeCall();
void kernelv1_removeTaskByPos(uint8_t position);
uint8_t kernelv1_removeTaskByPtr(void* t_pointer);
uint8_t kernelv1_removeTask(kv1TaskHandle_t handle);
void kernelv1_setTaskState(kv1TaskHandle_t handle, uint8_t newState);
void kernelv1_clearCallQueue();
void kernelv1_clearTaskQueue();
void kernelv1_init();

void kernelv1_startTimer();
void kernelv1_stopTimer();
void kernelv1_setupTimer();

#ifdef KERNEL_TIMER_MODULE
	uint8_t kernel_setTimer(kv1TimerISR_t t_pointer, uint32_t t_delay);
	uint8_t kernel_removeTimer(kv1TimerISR_t t_pointer);
	void kernel_timerService();
#endif

#ifdef KERNEL_SD_MODULE
	void sd_putc(char data);
	void sd_puts(char * data);
	void sd_flush();
	void sd_readPtr();
	void sd_init();
#endif

#ifdef KERNEL_WDT_MODULE
	void wdt_saveMCUCSR(void) __attribute__((naked)) __attribute__((section(".init3")));
	//void wdt_disableWatchdog();
	void wdt_enableWatchdog();
#endif

#ifdef KERNEL_UTIL_MODULE
	#define util_getArrayLength_m(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
	uint8_t util_strCompare(char * a, char * b, uint8_t len);
#endif

//#ifdef KERNEL_CLI_MODULE
	#define ERR_EMPTY_STRING 20
	#define ERR_COMMAND_NOT_FOUND 21
	void cli_init();
	void cli_registerCommand(const char * c_keyword, kv1CmdHandler_t c_ptr);
//#endif


#ifdef KERNEL_DEBUG_MODULE
	#define threads_enterCriticalSection()
	#define threads_exitCriticalSection()
	#define PGM_ON 1
	#define PGM_OFF 0
	#define PGM_PUTS 2
	#define L_NONE 0
	#define L_INFO 1
	#define L_WARN 2
	#define L_ERROR 3
	#define L_FATAL 4
	
	void debug_puts(uint8_t level, const char * message);
	void debug_logMessage(uint8_t pgm, uint8_t level, const char * format, ...);
#endif

#endif /* KERNEL_H_ */