/*
 * main.c
 *
 * Created: 07.03.2019 17:56:34
 *  Author: ThePetrovich
 */ 

#include "config.h"

void exampleTask(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test.\r\n"), kernelv1_getUptime());
}

void exampleTask1(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test. 1\r\n"), kernelv1_getUptime());
}

void exampleTask2(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test. 2\r\n"), kernelv1_getUptime());
}

int main(void){
	hal_enableInterrupts();
	wdt_enable(WDTO_2S);
	kernelv1_addTask(KTASK_REPEATED, exampleTask, 2000, KSTATE_ACTIVE);	
	kernelv1_addTask(KTASK_REPEATED, exampleTask1, 500, KSTATE_ACTIVE);	
	kernelv1_addTask(KTASK_REPEATED, exampleTask2, 1250, KSTATE_ACTIVE);	
	kernelv1_init();
	
	while(1) {
		kernelv1_taskManager();
	}
}
