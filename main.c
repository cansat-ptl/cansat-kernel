/*
 * main.c
 *
 * Created: 07.03.2019 17:56:34
 *  Author: ThePetrovich
 */ 

#include "config.h"

void exampleTask(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test.\r\n"), kernel_getUptime());
}

void exampleTask1(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test. 1\r\n"), kernel_getUptime());
}

void exampleTask2(){
	debug_logMessage(PGM_ON, L_NONE, PSTR("%d This is a test. 2\r\n"), kernel_getUptime());
}

int main(void){
	hal_enableInterrupts();
	wdt_enable(WDTO_2S);
	kernel_addTask(KTASK_REPEATED, exampleTask, 2000, KPRIO_HIGH, KSTATE_ACTIVE);	
	kernel_addTask(KTASK_REPEATED, exampleTask1, 500, KPRIO_HIGH, KSTATE_ACTIVE);	
	kernel_addTask(KTASK_REPEATED, exampleTask2, 1250, KPRIO_HIGH, KSTATE_ACTIVE);	
	kernelInit();
}
