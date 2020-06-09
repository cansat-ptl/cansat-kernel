/*
 * kernel.c
 *
 * Created: 11.05.2019 21:12:52
 *  Author: ThePetrovich
 */

#include <kernel-v1/kernel.h>
#include <kernel-v1/hal.h>

static volatile uint16_t kv1flags = 0;
static volatile uint16_t kv1flags_mirror __attribute__ ((section (".noinit")));
static uint64_t e_time = 0;
extern uint8_t mcucsr_mirror;

static uint8_t kv1CallIndex = 0;
static volatile uint8_t kv1TaskIndex = 0; //Index of the last task in queue
static volatile kv1Call_t kv1CallQueue[MAX_CALL_QUEUE_SIZE]; //TODO: replace with #define
static volatile struct kv1TaskStruct_t kv1TaskList[MAX_TASK_QUEUE_SIZE]; //TODO: replace with #define
static struct kv1TaskStruct_t kv1IdleTask;

void kv1Idle()
{
	hal_nop();
}

void init();

void kernelv1_setFlag(uint8_t flag, uint8_t value)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t nvalue = !!value;
	kv1flags ^= (-1 * nvalue ^ kv1flags) & (1 << flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
}

uint8_t kernelv1_checkFlag(uint8_t flag)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();
	uint8_t flag_tmp = hal_checkBit_m(kv1flags, flag);
	hal_enableInterrupts();
	hal_statusReg = sreg;
	return flag_tmp;
}

uint64_t kernelv1_getUptime()
{
	return e_time;
}

static void kernelv1_resetTaskByPosition(uint8_t position)
{
	kv1TaskList[position].pointer = kv1Idle;
	kv1TaskList[position].delay = 0;
	kv1TaskList[position].repeatPeriod = 0;
	kv1TaskList[position].priority = KPRIO_LOW;
	kv1TaskList[position].state = KSTATE_ACTIVE;
}

static inline void kernelv1_addCall_i(kv1Call_t t_ptr)
{
	if (kv1CallIndex < MAX_CALL_QUEUE_SIZE) {
		kv1CallQueue[kv1CallIndex] = t_ptr;
		kv1CallIndex++;
	}
}

uint8_t kernelv1_addCall(kv1Call_t t_ptr)
{
	uint8_t exitcode = 1;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if (kv1CallIndex < MAX_CALL_QUEUE_SIZE) {
		kv1CallQueue[kv1CallIndex] = t_ptr;
		kv1CallIndex++;
		exitcode = 0;
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return exitcode;
}

kv1TaskHandle_t kernelv1_addTask(uint8_t taskType, void* ptr, uint16_t period, uint8_t startupState)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if(period == 0) period = 1;

	struct kv1TaskStruct_t dummyTask;
	kv1TaskHandle_t handle = NULL;

	dummyTask.pointer = ptr;
	dummyTask.delay = period - 1;
	dummyTask.state = startupState;
	if(taskType == KTASK_REPEATED) dummyTask.repeatPeriod = period - 1;

	for (int i = 0; i < MAX_TASK_QUEUE_SIZE; i++) {
		if (ptr == kv1TaskList[i].pointer || kv1TaskList[i].pointer == kv1Idle || kv1TaskList[i].pointer == NULL) {
			kv1TaskList[i] = dummyTask;
			handle = &kv1TaskList[i];
			break;
		}
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return handle;
}

uint8_t kernelv1_removeCall() //TODO: cyclic buffer
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if(kv1CallIndex > 0){
		kv1CallIndex--;
		for(int i = 0; i < MAX_CALL_QUEUE_SIZE-1; i++){
			kv1CallQueue[i] = kv1CallQueue[i+1];
		}
		kv1CallQueue[MAX_CALL_QUEUE_SIZE-1] = &kv1IdleTask;
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return 0;
}

void kernelv1_removeTaskByPos(uint8_t position) //TODO: cyclic buffer
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	kv1TaskIndex--;
	kernelv1_resetTaskByPosition(position);
	for (int j = position; j < MAX_TASK_QUEUE_SIZE-1; j++) {
		kv1TaskList[j] = kv1TaskList[j+1];
	}
	kernelv1_resetTaskByPosition(MAX_TASK_QUEUE_SIZE-1);

	hal_enableInterrupts();
	hal_statusReg = sreg;
}

uint8_t kernelv1_removeTaskByPtr(void* t_pointer)
{
	uint8_t position, exitcode = 1;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	for (position = 0; position < MAX_TASK_QUEUE_SIZE; position++) {
		if (t_pointer == kv1TaskList[position].pointer)
			break;
	}

	if (position != MAX_TASK_QUEUE_SIZE-1) {
		kernelv1_removeTaskByPos(position);
		exitcode = 0;
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return exitcode;
}

uint8_t kernelv1_removeTask(kv1TaskHandle_t handle)
{
	uint8_t position, exitcode = 1;
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if (handle != NULL) {
		position = utils_ARRAY_INDEX_FROM_ADDR(kv1TaskList, handle, struct kv1TaskStruct_t);
		kernelv1_removeTaskByPos(position);
	}

	hal_enableInterrupts();
	hal_statusReg = sreg;
	return exitcode;
}

void kernelv1_clearCallQueue()
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	for (int i = 0; i < MAX_CALL_QUEUE_SIZE; i++) {
		kv1CallQueue[i] = &kv1IdleTask;
	}
	kv1CallIndex = 0;

	hal_enableInterrupts();
	hal_statusReg = sreg;
}

void kernelv1_clearTaskQueue()
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	for (int i = 0; i < MAX_TASK_QUEUE_SIZE; i++) {
		kernelv1_resetTaskByPosition(i);
	}
	kv1TaskIndex = 0;

	hal_enableInterrupts();
	hal_statusReg = sreg;
}

void kernelv1_setTaskState(kv1TaskHandle_t handle, uint8_t newState)
{
	uint8_t sreg = hal_statusReg;
	hal_disableInterrupts();

	if (handle != NULL) handle -> state = newState;

	hal_enableInterrupts();
	hal_statusReg = sreg;
}

void kernelv1_taskManager()
{
	if (kv1CallIndex != 0) {
		(*(kv1CallQueue[0] -> pointer))();
		if (kv1CallQueue[0] -> state == 254) kernelv1_removeTask(kv1CallQueue[0]);
		kernelv1_removeCall();
	}
}

void kernelv1_startTimer()
{
	if (kernelv1_checkFlag(KFLAG_TIMER_SET)) {
		hal_startSystemTimer();
		kernelv1_setFlag(KFLAG_TIMER_EN, 1);
	}
}

void kernelv1_stopTimer()
{
	if (kernelv1_checkFlag(KFLAG_TIMER_SET)) {
		hal_stopSystemTimer();
		kernelv1_setFlag(KFLAG_TIMER_EN, 0);
	}
}

void kernelv1_setupTimer()
{
	hal_setupSystemTimer();
	kernelv1_setFlag(KFLAG_TIMER_SET, 1);
}

void kernelv1_init()
{
	kernelv1_setFlag(KFLAG_INIT, 1);
	kernelv1_setFlag(KFLAG_TIMER_SET, 0);
	kernelv1_setFlag(KFLAG_TIMER_EN, 0);
	
	kv1IdleTask.pointer = kv1Idle;
	
	#if LOGGING == 1
		debug_logMessage(PGM_ON, L_NONE, (char *)PSTR("\r\n[INIT]kernel: Starting up CanSat kernel v%s\r\n\r\n"), KERNEL_VER);
	#endif

	wdt_reset();

	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up call queue"));
	#endif

	kernelv1_clearCallQueue();

	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("[INIT]kernel: Setting up task queue"));
	#endif

	kernelv1_clearTaskQueue();

	#if LOGGING == 1
		debug_logMessage(PGM_PUTS, L_NONE, (char *)PSTR("                  [DONE]\r\n"));
	#endif 
	
	kernelv1_setupTimer();
	kernelv1_startTimer();

	kernelv1_setFlag(KFLAG_TIMER_EN, 1);
}

void kernelv1_taskService()
{
	for(int i = 0; i < MAX_TASK_QUEUE_SIZE; i++){
		if (kv1TaskList[i].pointer == kv1Idle || kv1TaskList[i].pointer == NULL) continue;
		else {
			if(kv1TaskList[i].delay != 0 && kv1TaskList[i].state == KSTATE_ACTIVE)
				kv1TaskList[i].delay--;
			else {
				if(kv1TaskList[i].state == KSTATE_ACTIVE){
					kernelv1_addCall_i(&kv1TaskList[i]);
					if(kv1TaskList[i].repeatPeriod == 0)
						kv1TaskList[i].state = 255;
					else
						kv1TaskList[i].delay = kv1TaskList[i].repeatPeriod;
				}
			}
		}
	}
	e_time++;
}

#if CFG_USE_EXTERNAL_ISR == 0
ISR(HAL_TIMER_INTERRUPT_vect)
{
	kernelv1_taskService();
	#ifndef USE_EXTERNAL_TIMER_ISR
		#if KERNEL_TIMER_MODULE == 1
			kernelv1_timerService();
		#endif
	#endif
}
#endif