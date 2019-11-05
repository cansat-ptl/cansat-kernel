/*
 * timers.c
 *
 * Created: 20.07.2019 10:41:13
 *  Author: WorldSkills-2019
 */ 
#include "../kernel.h"

#if KERNEL_TIMER_MODULE == 1
static volatile struct kTimerStruct_t timerQueue[MAX_TIMER_COUNT];
static volatile uint8_t timerIndex = 0;

uint8_t kernel_setTimer(kTimerISR t_pointer, uint32_t t_delay)
{
	if(!kernel_checkFlag(KFLAG_TIMER_EN)) return 2;
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	for(int i = 0; i <= timerIndex; i++){
		if(timerQueue[i].tsrPointer == t_pointer){
			timerQueue[i].period = t_delay;
			timerQueue[i].repeatPeriod = t_delay;
			
			hal_enableInterrupts();
			hal_statusReg = sreg;
			return 0;
		}
	}
	if(timerIndex < MAX_TIMER_COUNT){
		timerIndex++;
		timerQueue[timerIndex].tsrPointer = t_pointer;
		timerQueue[timerIndex].period = t_delay;
		timerQueue[timerIndex].repeatPeriod = t_delay;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return MAX_TIMER_COUNT;
	}
}

uint8_t kernel_removeTimer(kTimerISR t_pointer)
{
	if(!kernel_checkFlag(KFLAG_TIMER_EN)) return 2;
	
	uint8_t position;
	
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	timerIndex--;
	for(position = 0; position < MAX_TIMER_COUNT-1; position++){
		if(t_pointer == timerQueue[position].tsrPointer)
		break;
	}
	
	if(position != MAX_TIMER_COUNT-1){
		timerQueue[position].tsrPointer = NULL;
		timerQueue[position].period = 0;
		for(int j = position; j < MAX_TIMER_COUNT-1; j++){
			timerQueue[j] = timerQueue[j+1];
		}
		timerQueue[MAX_TIMER_COUNT-1].tsrPointer = NULL;
		timerQueue[MAX_TIMER_COUNT-1].period = 0;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

inline void kernel_timerService()
{
	for(int i = 0; i < MAX_TIMER_COUNT; i++){
		if(timerQueue[i].tsrPointer == NULL) continue;
		else {
			if(timerQueue[i].period != 0)
				timerQueue[i].period--;
			else {
				if(timerQueue[i].tsrPointer != NULL) (timerQueue[i].tsrPointer)(); //Additional NULLPTR protection
				timerQueue[i].period = timerQueue[i].repeatPeriod;
			}
		}
	}
}
#endif
