/*
 * utils.c
 *
 * Created: 23.06.2019 10:27:23
 *  Author: WorldSkills-2019
 */ 
#include "../kernel.h"
#include "../drivers.h"

void util_printVersion()
{
	#if KERNEL_DEBUG_MODULE == 1
		char msg[128];
		debug_logMessage(PGM_ON, L_INFO, (char *)PSTR("[INIT]Kernel: starting up\r\n"));
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using kernel version %s built %s\r\n"), KERNEL_VER, KERNEL_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using debug module version %s built %s\r\n"), DBG_MOD_VER, DBG_MOD_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using HAL module version %s built %s\r\n"), HAL_MOD_VER, HAL_MOD_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using UART interface driver version %s built %s\r\n"), UARTDRV_VER, UARTDRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using SPI interface driver version %s built %s\r\n"), SPIDRV_VER, SPIDRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using TWI interface driver version %s built %s\r\n"), TWIDRV_VER, TWIDRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using ADXL345 device driver version %s built %s\r\n"), ADXLDRV_VER, ADXLDRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using BMP280 device driver version %s built %s\r\n"), BMPDRV_VER, BMPDRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
		sprintf_P(msg, PSTR("[INIT]Kernel: using DS18B20 device driver version %s built %s\r\n"), DS18DRV_VER, DS18DRV_TIMESTAMP);
		debug_logMessage(PGM_OFF, L_INFO, "%s", msg);
		wdt_reset();
	#else
		#warning Trying to use disabled util module, this may spawn dragons
	#endif
}

uint8_t util_strCompare(char * a, char * b, uint8_t len)
{
	unsigned char i;
	for(i=0;i<len;i++){
		if(a[i]!=b[i]) return 0;
	}
	return 1;
}

void util_displayError(uint8_t error)
{
	if(kernel_checkFlag(KFLAG_DEBUG)){
		switch(error){
			case ERR_QUEUE_OVERFLOW:
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n--------------------------------------------------------------------------------\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] A task/call queue overflow has occurred.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] This is a critical issue, and immediate action is required.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Task manager will be reloaded and reset.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Please, report this to the developer as soon as possible.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Error details: MAX_QUEUE_SIZE >= callIndex/taskIndex\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("--------------------------------------------------------------------------------\r\n\r\n"));
			break;
			case ERR_WDT_RESET:
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n--------------------------------------------------------------------------------\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] The system has been reset by watchdog.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] This is usually caused by software issues or faulty device connections.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Please, report this to the developer as soon as possible.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Error details: MCUCSR.WDRF = 1\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("--------------------------------------------------------------------------------\r\n\r\n"));
			break;
			case ERR_BOD_RESET:
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n--------------------------------------------------------------------------------\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] The system has been reset by brown-out detector.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] This is usually caused by an unstable power supply.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Please, check power supply wire connections and circuitry as soon as possible.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Error details: MCUCSR.BORF = 1\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("--------------------------------------------------------------------------------\r\n\r\n"));
			break;
			case ERR_DEVICE_FAIL:
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n--------------------------------------------------------------------------------\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] A major device failure has been reported by one of the tasks.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] To prevent damage and data corruption, the task has been suspended.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Please, check your device connections and circuitry as soon as possible.\r\n"));
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[FATAL] Error details: taskReturnCode = ERR_DEVICE_FAIL\r\n")); //TODO: implement verbose error info, e.g what device has failed
				debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("--------------------------------------------------------------------------------\r\n\r\n"));
			break;
		}
	}
}