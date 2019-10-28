/*
 * kernel.c
 *
 * Created: 11.05.2019 21:12:52
 *  Author: ThePetrovich
 */ 

#include "../kernel.h"
#include "../hal.h"

static volatile uint16_t kflags = 0;
static volatile uint16_t kflags_mirror __attribute__ ((section (".noinit")));
static uint64_t e_time = 0;
extern uint8_t mcucsr_mirror;

static uint8_t callIndex[3] = {0, 0, 0};
static volatile uint8_t taskIndex = 0; //Index of the last task in queue
#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
	static volatile task callQueueP0[MAX_HIGHPRIO_CALL_QUEUE_SIZE];
#endif
#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0
	static volatile task callQueueP1[MAX_NORMPRIO_CALL_QUEUE_SIZE];
#endif
#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
	static volatile task callQueueP2[MAX_LOWPRIO_CALL_QUEUE_SIZE]; //TODO: variable number of priorities (might be hard to implement)
#endif
static volatile struct taskStruct taskQueue[MAX_TASK_QUEUE_SIZE];

static char const PROGMEM _MODULE_SIGNATURE[] = "Kernel";

int idle(){
	hal_nop();
	return 0;
}
void init();

void kernel_setFlag(uint8_t flag, uint8_t value)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t nvalue = !!value;
	kflags ^= (-1 * nvalue ^ kflags) & (1 << flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
} 

uint8_t kernel_checkFlag(uint8_t flag)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t flag_tmp = hal_checkBit_m(kflags, flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return flag_tmp;
}

uint64_t kernel_getUptime()
{
	return e_time;
}

static inline volatile task* kernel_getCallQueuePointer(uint8_t t_priority){
	switch(t_priority){
		case KPRIO_HIGH:
			#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
				return callQueueP0;
			#else
				return NULL;
			#endif
		break;
		case KPRIO_NORM:
			#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0 
				return callQueueP1;
			#else
				return NULL;
			#endif
		break;
		case KPRIO_LOW:
			#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
				return callQueueP2;
			#else
				return NULL;
			#endif
		break;
		default:
			return NULL;
		break;
	}
}

static inline uint8_t kernel_getMaxQueueSize(uint8_t t_priority){
	switch(t_priority){
		case KPRIO_HIGH:
			return MAX_HIGHPRIO_CALL_QUEUE_SIZE;
		break;
		case KPRIO_NORM:
			return MAX_NORMPRIO_CALL_QUEUE_SIZE;
		break;
		case KPRIO_LOW:
			return MAX_LOWPRIO_CALL_QUEUE_SIZE;
		break;
		default:
			return 0;
		break;
	}
}

static inline void kernel_resetTaskByPosition(uint8_t position){
	taskQueue[position].pointer = idle;
	taskQueue[position].delay = 0;
	taskQueue[position].repeatPeriod = 0;
	taskQueue[position].priority = KPRIO_LOW;
	taskQueue[position].state = KSTATE_ACTIVE;
}

inline uint8_t kernel_addCall(task t_ptr, uint8_t t_priority)
{	
	if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
		debug_logMessage(PGM_PUTS, L_INFO, (char *)PSTR("kernel: Added call to queue\r\n"));
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return 0;

	if(callIndex[t_priority] < maxsize){
		callIndex[t_priority]++;
		volatile task* ptr = kernel_getCallQueuePointer(t_priority);
		(ptr)[callIndex[t_priority]] = t_ptr;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		#if KERNEL_UTIL_MODULE == 1
			util_displayError(ERR_QUEUE_OVERFLOW);
		#endif
		kernel_clearCallQueue(0);
		kernel_clearCallQueue(1);
		kernel_clearCallQueue(2);
		kernel_clearTaskQueue();
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return ERR_QUEUE_OVERFLOW;
	}
}

uint8_t kernel_addTask(uint8_t taskType, task t_ptr, uint16_t t_delay, uint8_t t_priority, uint8_t startupState)
{
	if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
		debug_logMessage(PGM_PUTS, L_INFO, (char *)PSTR("kernel: Added timed task to queue\r\n"));
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	for(int i = 0; i <= taskIndex; i++){
		if(taskQueue[i].pointer == t_ptr){
			taskQueue[i].repeatPeriod = t_delay;
			taskQueue[i].priority = t_priority;
			taskQueue[i].state = startupState;
			if(taskType == KTASK_REPEATED) taskQueue[i].repeatPeriod = t_delay;
			else taskQueue[i].repeatPeriod = 0;
			
			hal_enableInterrupts();
			hal_statusReg = sreg;
			return 0;
		}
	}
	if(taskIndex < MAX_TASK_QUEUE_SIZE){
		taskIndex++;
		taskQueue[taskIndex].pointer = t_ptr;
		taskQueue[taskIndex].delay = t_delay;
		taskQueue[taskIndex].priority = t_priority;
		taskQueue[taskIndex].state = startupState;
		if(taskType == KTASK_REPEATED) taskQueue[taskIndex].repeatPeriod = t_delay;
		else taskQueue[taskIndex].repeatPeriod = 0;
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		#if KERNEL_UTIL_MODULE == 1
			util_displayError(ERR_QUEUE_OVERFLOW);
		#endif
		kernel_clearCallQueue(0);
		kernel_clearCallQueue(1);
		kernel_clearCallQueue(2);
		kernel_clearTaskQueue();
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return ERR_QUEUE_OVERFLOW;
	}
}

inline uint8_t kernel_removeCall(uint8_t t_priority)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return 0;
	volatile task* ptr = kernel_getCallQueuePointer(t_priority);
	if(ptr == NULL) return 0;
	
	if(callIndex[t_priority] != 0){
		callIndex[t_priority]--;
		for(int i = 0; i < maxsize-1; i++){
			(ptr)[i] = (ptr)[i+1];
		}
		(ptr)[maxsize-1] = idle;
	}	
	else {
		(ptr)[0] = idle;
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

inline uint8_t kernel_removeTask(uint8_t position)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	taskIndex--;
	kernel_resetTaskByPosition(position);
	for(int j = position; j < MAX_TASK_QUEUE_SIZE-1; j++){
		taskQueue[j] = taskQueue[j+1];
	}
	kernel_resetTaskByPosition(MAX_TASK_QUEUE_SIZE-1);
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

uint8_t kernel_removeTaskByPtr(task t_pointer)
{
	uint8_t position;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	taskIndex--;
	for(position = 0; position < MAX_TASK_QUEUE_SIZE-1; position++){
		if(t_pointer == taskQueue[position].pointer)
			break;
	}
	
	if(position != MAX_TASK_QUEUE_SIZE-1){
		kernel_resetTaskByPosition(position);
		for(int j = position; j < MAX_TASK_QUEUE_SIZE-1; j++){
			taskQueue[j] = taskQueue[j+1];
		}
		kernel_resetTaskByPosition(MAX_TASK_QUEUE_SIZE-1);
		
		hal_enableInterrupts();
		hal_statusReg = sreg;
		return 0;
	}
	else {
		 hal_enableInterrupts();
		 hal_statusReg = sreg;
		 return 1;
	}
}

void kernel_clearCallQueue(uint8_t t_priority)
{
	if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
		debug_logMessage(PGM_PUTS, L_WARN, (char *)PSTR("kernel: Call queue cleared\r\n"));
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	
	uint8_t maxsize = kernel_getMaxQueueSize(t_priority);
	if(maxsize == 0) return;	
	volatile task* ptr = kernel_getCallQueuePointer(t_priority);
	if(ptr == NULL) return;
	
	for(int i = 0; i < maxsize; i++){
		(ptr)[i] = idle;
	}
	callIndex[t_priority] = 0;
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
}

void kernel_clearTaskQueue()
{
	if(kernel_checkFlag(KFLAG_DEBUG) && VERBOSE)
		debug_logMessage(PGM_PUTS, L_WARN, (char *)PSTR("kernel: Task queue cleared\r\n"));
		
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE; i++){
		kernel_resetTaskByPosition(i);
	}
	taskIndex = 0;
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
}

uint8_t kernel_setTaskState(task t_pointer, uint8_t state)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
		
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE-1; i++){
		if(taskQueue[i].pointer == t_pointer){
			taskQueue[i].state = state;
			hal_enableInterrupts();
			return 0;
		}
	}
	
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 1;
}

inline static uint8_t kernel_taskManager()
{
	#if MAX_HIGHPRIO_CALL_QUEUE_SIZE != 0
	if((callQueueP0[0] != idle || callQueueP0[1] != idle)){
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		int taskReturnCode = (callQueueP0[0])();
		if(taskReturnCode != 0) kernel_setTaskState(callQueueP0[0], KSTATE_SUSPENDED);
		kernel_removeCall(0);
		
		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
		
		return taskReturnCode;
	}
	#endif
	#if MAX_NORMPRIO_CALL_QUEUE_SIZE != 0
	else if((callQueueP1[0] != idle || callQueueP1[1] != idle)){	
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		int taskReturnCode = (callQueueP1[0])();
		if(taskReturnCode != 0) kernel_setTaskState(callQueueP1[0], KSTATE_SUSPENDED);
		kernel_removeCall(1);
		
		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
		
		return taskReturnCode;
	}
	#endif
	#if MAX_LOWPRIO_CALL_QUEUE_SIZE != 0
	else if(callQueueP2[0] != idle || callQueueP2[1] != idle){		
		#if PROFILING == 1
			uint64_t startTime = kernel_getUptime();
		#endif
		
		int taskReturnCode = (callQueueP2[0])();
		if(taskReturnCode != 0) kernel_setTaskState(callQueueP2[0], KSTATE_SUSPENDED);
		kernel_removeCall(2);

		#if PROFILING == 1
			debug_logMessage(PGM_ON, L_INFO, PSTR("kernel: Task exec time: %u ms\r\n"), (unsigned int)(kernel_getUptime()-startTime));
		#endif
		
		return taskReturnCode;
	}
	#endif
	idle();
	return 0;
}

static uint8_t kernel()
{
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                        [DONE]\r\n"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting task manager"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
	#if KERNEL_CLI_MODULE == 1
		kernel_initCLI();
	#endif
	while(1){
		wdt_reset();
		uint8_t taskReturnCode = kernel_taskManager();
		#if KERNEL_UTIL_MODULE == 1
			if(taskReturnCode != 0) util_displayError(taskReturnCode);
		#endif
		hal_switchBit(&LED_KRN_PORT, LED_KRN);
		hal_enableInterrupts();
	}
	return ERR_KRN_RETURN;
}

void kernel_startTimer()
{
	if(kernel_checkFlag(KFLAG_TIMER_SET)){
		hal_startSystemTimer();
		kernel_setFlag(KFLAG_TIMER_EN, 1);
	}
}

void kernel_stopTimer()
{
	if(kernel_checkFlag(KFLAG_TIMER_SET)){
		hal_stopSystemTimer();
		kernel_setFlag(KFLAG_TIMER_EN, 0);
	}
}

void kernel_setupTimer()
{
	hal_setupSystemTimer();
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                         [DONE]\r\n"));
	kernel_setFlag(KFLAG_TIMER_SET, 1);
}

uint8_t kernelInit()
{
	init();
	kernel_setFlag(KFLAG_INIT, 1);
	kernel_setFlag(KFLAG_TIMER_SET, 0);
	kernel_setFlag(KFLAG_TIMER_EN, 0);
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n[INIT]kernel: Starting up CanSat kernel v%s\r\n\r\n"), KERNEL_VER);
	
	wdt_reset();
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_HIGH queue"));
	kernel_clearCallQueue(0);
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("         [DONE]\r\n"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_NORM queue"));
	kernel_clearCallQueue(1);
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("         [DONE]\r\n"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up PRIORITY_LOW queue"));
	kernel_clearCallQueue(2);
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("          [DONE]\r\n"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up task queue"));
	kernel_clearTaskQueue();
	wdt_reset();
	
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("\r\n----------------------Memory usage----------------------\r\n"));
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: HIGHPRIO queue size:             %d\r\n"), MAX_HIGHPRIO_CALL_QUEUE_SIZE);
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: NORMPRIO queue size:             %d\r\n"), MAX_NORMPRIO_CALL_QUEUE_SIZE);
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: LOWPRIO queue size:              %d\r\n"), MAX_LOWPRIO_CALL_QUEUE_SIZE);
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Task queue size:                 %d\r\n"), MAX_TASK_QUEUE_SIZE);
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: HIGHPRIO queue memory usage:     %d bytes\r\n"), MAX_HIGHPRIO_CALL_QUEUE_SIZE * sizeof(void*));
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: NORMPRIO queue memory usage:     %d bytes\r\n"), MAX_NORMPRIO_CALL_QUEUE_SIZE * sizeof(void*));
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: LOWPRIO queue memory usage:      %d bytes\r\n"), MAX_LOWPRIO_CALL_QUEUE_SIZE * sizeof(void*));
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Task queue memory usage:         %d bytes\r\n"), MAX_TASK_QUEUE_SIZE * sizeof(taskQueue[0]));
	debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("[INIT]kernel: Total Task Manager memory usage: %d bytes\r\n"), MAX_TASK_QUEUE_SIZE * sizeof(taskQueue[0]) +\
																												  MAX_HIGHPRIO_CALL_QUEUE_SIZE * sizeof(void*) +\
																												  MAX_NORMPRIO_CALL_QUEUE_SIZE * sizeof(void*) +\
																												  MAX_LOWPRIO_CALL_QUEUE_SIZE * sizeof(void*));
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("--------------------------------------------------------\r\n\r\n"));
	wdt_reset();
	
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting timer"));
	kernel_setupTimer();
	kernel_startTimer();
	kernel_setFlag(KFLAG_TIMER_EN, 1);
	debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Starting kernel"));
	kernel();
	
	return 0;
}

void kernel_checkMCUCSR()
{
	if(hal_checkBit_m(mcucsr_mirror, WDRF)){
		#if KERNEL_UTIL_MODULE == 1
			util_displayError(ERR_WDT_RESET);
		#endif
		hal_setBit_m(kflags, WDRF);
		return;
	}
	if(hal_checkBit_m(mcucsr_mirror, BORF)){
		#if KERNEL_UTIL_MODULE == 1
			util_displayError(ERR_BOD_RESET);
		#endif
		hal_setBit_m(kflags, BORF);
	}
	return;
}


inline static void kernel_taskService()
{
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE; i++){
		if(taskQueue[i].pointer == idle) continue;
		else {
			if(taskQueue[i].delay != 0 && taskQueue[i].state == KSTATE_ACTIVE)
				taskQueue[i].delay--;
			else {
				if(taskQueue[i].state == KSTATE_ACTIVE && taskQueue[i].pointer != NULL){
					kernel_addCall(taskQueue[i].pointer, taskQueue[i].priority);
					if(taskQueue[i].repeatPeriod == 0) 
						kernel_removeTask(i);
					else 
						taskQueue[i].delay = taskQueue[i].repeatPeriod;
				}
			}
		}
	}
	e_time++;
}

ISR(HAL_TIMER_INTERRUPT_vect)
{
	hal_setBit_m(kflags, KFLAG_TIMER_ISR);
	hal_disableInterrupts();
	
	kernel_taskService();
	#ifndef USE_EXTERNAL_TIMER_ISR
		#if KERNEL_TIMER_MODULE == 1
			kernel_timerService();
		#endif
	#endif
	
	hal_enableInterrupts();
	hal_clearBit_m(kflags, KFLAG_TIMER_ISR);
}
