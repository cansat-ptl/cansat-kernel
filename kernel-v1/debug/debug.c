/*
 * debug.c
 *
 * Created: 02.06.2019 20:53:16
 *  Author: ThePetrovich
 */

#include <kernel-v1/kernel.h>
#include <kernel-v1/hal.h>
#include <stdio.h>

#define DBG_MOD_VER "0.7.0-bleeding"
#define DBG_MOD_TIMESTAMP __TIMESTAMP__

const char log_nolevel[] PROGMEM = "";
const char log_levelinfo[] PROGMEM = "[INFO] ";
const char log_levelwarn[] PROGMEM = "[WARN] ";
const char log_levelerr[] PROGMEM = "[ERROR] ";
const char log_levelfatal[] PROGMEM = "[FATAL] ";


const char * const levels[] PROGMEM = {
	log_nolevel,
	log_levelinfo,
	log_levelwarn,
	log_levelerr,
	log_levelfatal
};

const char log_stageInit[] PROGMEM = "<INIT>";
const char log_stageRun[] PROGMEM = "<EXEC>";
const char log_stageHalt[] PROGMEM = "<HALT>";
const char log_stageErr[] PROGMEM = "<ERROR>";


const char * const stages[] PROGMEM = {
	log_stageInit,
	log_stageRun,
	log_stageHalt,
	log_stageErr,
};

void debug_sendMessage(uint8_t level, const char * format, va_list args);
void debug_sendMessage_p(uint8_t level, const char * format, va_list args);

void debug_printLevel(uint8_t level)
{
	char * levelptr = (char*)hal_READ_WORD_FROM_FLASH(&(levels[level]));

	while(hal_READ_BYTE_FROM_FLASH(levelptr) != 0x00)
		hal_UART_PUTC(hal_READ_BYTE_FROM_FLASH(levelptr++));
}

/*
void debug_printStage()
{
	uint8_t stage = kernel_getSystemStatus();
	char * stageptr = (char*)hal_READ_WORD_FROM_FLASH(&(stages[stage]));

	while(hal_READ_BYTE_FROM_FLASH(stageptr) != 0x00)
		hal_UART_PUTC(hal_READ_BYTE_FROM_FLASH(stageptr++));
}
*/

inline void debug_sendMessage(uint8_t level, const char * format, va_list args)
{
	if (level != 0) {
		#if CFG_PROFILING == 0
			//time_updateSystemTime();
			//printf_P(PSTR("[%02d:%02d:%02d.%03d]"), time_getHours(), time_getMinutes(), time_getSeconds(), time_getMilliseconds());
		#else
			printf_P(PSTR("[%8ld]"), (int32_t)kernel_getUptime());
		#endif
		//debug_printStage();
		debug_printLevel(level);
	}

	vfprintf(stdout, format, args);
}

inline void debug_sendMessage_p(uint8_t level, const char * format, va_list args)
{
	if (level != 0) {
		#if CFG_PROFILING == 0
			//time_updateSystemTime();
			//printf_P(PSTR("[%02d:%02d:%02d.%03d]"), time_getHours(), time_getMinutes(), time_getSeconds(), time_getMilliseconds());
		#else
			printf_P(PSTR("[%8ld]"), (int32_t)kernel_getUptime());
		#endif
		//debug_printStage();
		debug_printLevel(level);
	}

	vfprintf_P(stdout, format, args);
}

void debug_puts(uint8_t level, const char * format)
{
	threads_enterCriticalSection();

	if (level != 0) {

		#if CFG_PROFILING == 0
			//time_updateSystemTime();
			//printf_P(PSTR("[%02d:%02d:%02d.%03d]"), time_getHours(), time_getMinutes(), time_getSeconds(), time_getMilliseconds());
		#else
			printf_P(PSTR("[%8ld]"), (int32_t)kernel_getUptime());
		#endif

		//debug_printStage();
		debug_printLevel(level);
	}

	while(hal_READ_BYTE_FROM_FLASH(format) != 0x00)
		hal_UART_PUTC(hal_READ_BYTE_FROM_FLASH(format++));

	threads_exitCriticalSection();
}

void debug_logMessage(uint8_t pgm, uint8_t level, const char * format, ...)
{
	threads_enterCriticalSection();
	va_list args;

	va_start(args, format);
	switch(pgm){
		case 0:
			debug_sendMessage(level, format, args);
		break;
		case 1:
			debug_sendMessage_p(level, format, args);
		break;
		case 2:
			debug_puts(level, format);
		break;
	}
	va_end(args);
	threads_exitCriticalSection();
}
