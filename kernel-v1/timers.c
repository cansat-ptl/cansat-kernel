/*
 * timers.c
 *
 * Created: 20.07.2019 10:41:13
 *  Author: WorldSkills-2019
 */
#include <kernel-v1/kernel.h>

#if KERNEL_TIMER_MODULE == 1
static volatile struct kv1TimerStruct_t kv1TimerQueue[MAX_TIMER_COUNT];
static volatile uint8_t kv1TimerIndex = 0;

kv1TimerHandle_t kernelv1_setTimer(kv1TimerISR_t t_pointer, uint32_t t_period)
{
	kv1TimerHandle_t dummyHandle = NULL;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	for (int i = 0; i < MAX_TIMER_COUNT; i++) {
		if (kv1TimerQueue[i].tsrPointer == t_pointer || kv1TimerQueue[i].tsrPointer == NULL) {
			kv1TimerQueue[i].tsrPointer = t_pointer;
			kv1TimerQueue[i].period = t_period;
			kv1TimerQueue[i].repeatPeriod = t_period;
			dummyHandle = &kv1TimerQueue[i];
			kv1TimerIndex++;
			break;
		}
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return dummyHandle;
}

void kernelv1_removeTimer(kv1TimerHandle_t handle)
{
	uint8_t exitcode;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if (handle != NULL) handle -> tsrPointer = NULL;

	hal_enableInterrupts();
	hal_statusReg = sreg;
}

inline void kernelv1_timerService()
{
	if (kv1TimerIndex != 0) {
		for(int i = 0; i < MAX_TIMER_COUNT; i++){
			if(kv1TimerQueue[i].tsrPointer == NULL) continue;
			else {
				if(kv1TimerQueue[i].period != 0)
				kv1TimerQueue[i].period--;
				else {
					if(kv1TimerQueue[i].tsrPointer != NULL) (kv1TimerQueue[i].tsrPointer)(); //Additional NULLPTR protection
					kv1TimerQueue[i].period = kv1TimerQueue[i].repeatPeriod;
				}
			}
		}
	}
}
#endif
