/*
 * debug.c
 *
 * Created: 02.06.2019 20:53:16
 *  Author: ThePetrovich
 */ 

#include <stdarg.h>
#include "../kernel.h"
#include "../drivers.h"

extern uint8_t creg0;
extern volatile char tx0_buffer[128];

void sd_puts(char * data);
void sd_flush();

#if KERNEL_DEBUG_MODULE == 1

char levels[5][16] = {
	"",
	"[INFO]",
	"[WARN]",
	"[ERROR]",
	"[FATAL]"
};
#endif

inline void debug_sendMessage(uint8_t level, const char * format, va_list args) 
{
	#if KERNEL_DEBUG_MODULE == 1
		char *buffer = malloc(128);
		if(buffer == NULL) return;
		
		if(level != 0 && !kernel_checkFlag(KFLAG_INIT)){
			#if PROFILING == 0
				sprintf(buffer, "%02d.%02d.%02d %02d:%02d:%02d ", GPS.day, GPS.month, GPS.year, GPS.hour, GPS.minute, GPS.second);
			#else
				sprintf(buffer, "%ld ", (int32_t)kernel_getUptime());
			#endif
			uart0_puts(buffer);
		}
		uart0_puts(levels[level]);
		vsprintf(buffer, format, args);
		uart0_puts(buffer);
		free(buffer);
	#else
		#warning Trying to use disabled debug module, this may spawn dragons
	#endif
}

inline void debug_sendMessage_p(uint8_t level, const char * format, va_list args) 
{
	#if KERNEL_DEBUG_MODULE == 1
		char *buffer = malloc(128);
		if(buffer == NULL) return;
		
		if(level != 0 && !kernel_checkFlag(KFLAG_INIT)){
			#if PROFILING == 0
				sprintf(buffer, "%02d.%02d.%02d %02d:%02d:%02d ", GPS.day, GPS.month, GPS.year, GPS.hour, GPS.minute, GPS.second);
			#else
				sprintf(buffer, "%ld ", (int32_t)kernel_getUptime());
			#endif
			uart0_puts(buffer);
		}
		uart0_puts(levels[level]);
		vsprintf_P(buffer, format, args);
		uart0_puts(buffer);
		free(buffer);
	#else
		#warning Trying to use disabled debug module, this may spawn dragons
	#endif
}

void debug_sendMessageSD(uint8_t level, const char * format, va_list args)
{
	#if KERNEL_DEBUG_MODULE == 1
		char *buffer = malloc(128);
		if(buffer == NULL) return;
		
		if(level != 0 && !kernel_checkFlag(KFLAG_INIT)){
			sprintf(buffer, "%02d.%02d.%02d %02d:%02d:%02d ", GPS.day, GPS.month, GPS.year, GPS.hour, GPS.minute, GPS.second);
			sd_puts(buffer);
		}
		sd_puts(levels[level]);
		vsprintf(buffer, format, args);
		sd_puts(buffer);
		free(buffer);
	#else
		#warning Trying to use disabled debug module, this may spawn dragons
	#endif
}

void debug_sendMessageSD_p(uint8_t level, const char * format, va_list args)
{
	#if KERNEL_DEBUG_MODULE == 1
		char *buffer = malloc(128);
		if(buffer == NULL) return;
		
		if(level != 0 && !kernel_checkFlag(KFLAG_INIT)){
			sprintf(buffer, "%02d.%02d.%02d %02d:%02d:%02d ", GPS.day, GPS.month, GPS.year, GPS.hour, GPS.minute, GPS.second);
			sd_puts(buffer);
		}
		sd_puts(levels[level]);
		vsprintf_P(buffer, format, args);
		sd_puts(buffer);
		free(buffer);
	#else
		#warning Trying to use disabled debug module, this may spawn dragons
	#endif
}

inline void debug_logMessage(uint8_t pgm, uint8_t level, const char * format, ...)
{
	#if KERNEL_DEBUG_MODULE == 1
		va_list args;
		va_start(args, format);
		if(!pgm){
			debug_sendMessage(level, format, args);
			#if KERNEL_SD_MODULE == 1
				debug_sendMessageSD(level, format, args);
			#endif
		}
		else {
			debug_sendMessage_p(level, format, args);
			#if KERNEL_SD_MODULE == 1
				debug_sendMessageSD_p(level, format, args);
			#endif
		}
		va_end(args);
	#else
		#warning Trying to use disabled debug module, this may spawn dragons
	#endif
}
