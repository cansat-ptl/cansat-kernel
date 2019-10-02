/*
 * kernel.h
 *
 * Created: 11.05.2019 21:15:24
 *  Author: ThePetrovich
 */ 

#ifndef KERNEL_H_
#define KERNEL_H_

#define KERNEL_VER "0.5.1-bleeding"
#define KERNEL_TIMESTAMP __TIMESTAMP__

#define SDCARD_MOD_VER "0.0.5-rc1"
#define SDCARD_MOD_TIMESTAMP __TIMESTAMP__

#define CMD_MOD_VER "0.0.1-bleeding"
#define CMD_MOD_TIMESTAMP __TIMESTAMP__

#include "types.h"
#include "hal.h"
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
	
#define PRIORITY_HIGH 0
#define PRIORITY_NORM 1
#define PRIORITY_LOW 2
	
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

#define TASK_SINGLERUN 0
#define TASK_REPEATED 1

void init();
int systemInit();
void initTaskManager();

void kernel_setFlag(uint8_t flag, uint8_t value) __attribute__ ((section (".kernel")));
uint8_t kernel_checkFlag(uint8_t flag) __attribute__ ((section (".kernel")));
uint64_t kernel_getUptime() __attribute__ ((section (".kernel")));

uint8_t kernel_addCall(task t_ptr, uint8_t t_priority) __attribute__ ((section (".kernel"))); 
uint8_t kernel_addTask(uint8_t taskType, task t_ptr, uint16_t t_delay, uint8_t t_priority, uint8_t startupState) __attribute__ ((section (".kernel")));
uint8_t kernel_removeCall(uint8_t t_priority) __attribute__ ((section (".kernel")));
uint8_t kernel_removeTask(uint8_t position) __attribute__ ((section (".kernel")));
uint8_t kernel_setTaskState(task t_pointer, uint8_t state) __attribute__ ((section (".kernel")));
void kernel_clearTaskQueue() __attribute__ ((section (".kernel")));
void kernel_clearCallQueue(uint8_t t_priority) __attribute__ ((section (".kernel")));
void kernel_checkMCUCSR() __attribute__ ((section (".kernel")));
uint8_t kernelInit() __attribute__ ((section (".kernel")));

void kernel_stopTimer() __attribute__ ((section (".kernel")));
void kernel_startTimer() __attribute__ ((section (".kernel")));
void kernel_setupTimer() __attribute__ ((section (".kernel")));

#ifdef KERNEL_TIMER_MODULE
	uint8_t kernel_setTimer(timerISR t_pointer, uint32_t t_delay) __attribute__ ((section (".kernel")));
	uint8_t kernel_removeTimer(timerISR t_pointer) __attribute__ ((section (".kernel")));
	void kernel_timerService() __attribute__ ((section (".kernel")));
#endif

#ifdef KERNEL_SD_MODULE
	void sd_puts(char * data) __attribute__ ((section (".kernel")));
	void sd_flush() __attribute__ ((section (".kernel")));
	void sd_readPtr() __attribute__ ((section (".kernel")));
	void sd_init() __attribute__ ((section (".kernel")));
#endif

#ifdef KERNEL_WDT_MODULE
	void wdt_saveMCUCSR(void) __attribute__((naked)) __attribute__((section(".init3")));
	//void wdt_disableWatchdog();
	void wdt_enableWatchdog();
#endif

#ifdef KERNEL_UTIL_MODULE
	#define util_getArrayLength_m(arr) ((int)(sizeof(arr) / sizeof(arr)[0]))
	void util_printVersion() __attribute__ ((section (".kernel")));
	uint8_t util_strCompare(char * a, char * b, uint8_t len) __attribute__ ((section (".kernel")));
	void util_displayError(uint8_t error);
#endif

//#ifdef KERNEL_CLI_MODULE
	#define ERR_EMPTY_STRING 20
	#define ERR_COMMAND_NOT_FOUND 21
	void kernel_initCLI() __attribute__ ((section (".kernel")));
	void kernel_registerCommand(const char * c_keyword, cmdHandler c_ptr) __attribute__ ((section (".kernel")));
//#endif
	

#ifdef KERNEL_DEBUG_MODULE
	#define DBG_MOD_VER "0.5.1-bleeding"
	#define DBG_MOD_TIMESTAMP __TIMESTAMP__
	#define PGM_ON 1
	#define PGM_OFF 0
	#define L_NONE 0
	#define L_INFO 1
	#define L_WARN 2
	#define L_ERROR 3
	#define L_FATAL 4
	void debug_sendMessage(uint8_t level, const char * format, va_list args) __attribute__ ((section (".kernel")));
	void debug_sendMessageSD(uint8_t level, const char * format, va_list args) __attribute__ ((section (".kernel")));
	void debug_sendMessage_p(uint8_t level, const char * format, va_list args) __attribute__ ((section (".kernel")));
	void debug_sendMessageSD_p(uint8_t level, const char * format, va_list args) __attribute__ ((section (".kernel")));
	void debug_logMessage(uint8_t pgm, uint8_t level, const char * format, ...) __attribute__ ((section (".kernel")));
#endif

#endif /* KERNEL_H_ */