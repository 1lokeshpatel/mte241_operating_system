/*
 * kernel.c
 *
 *  Created on: Jun. 29, 2023
 *      Author: lokesh
 */

#define RUN_FIRST_THREAD 0x3
#define PRINT_HELLO 0x0
#define PRINT_WORLD 0x17
#define YIELD 0x1
#define OFFSET_INCREMENT 0x200
#define STACK_SIZE 0x4000

#include "kernel.h"
#include "main.h"

extern num_threads;

uint32_t* MSP_INIT_VAL;
uint32_t OFFSET;

struct k_thread osThreads[32];
uint32_t curr_thread;

void SVC_Handler_Main( unsigned int *svc_args ) {
	unsigned int svc_number;
	/*
	* Stack contains:
	* r0, r1, r2, r3, r12, r14, the return address and xPSR
	* First argument (r0) is svc_args[0]
	*/
	svc_number = ( ( char * )svc_args[ 6 ] )[ -2 ] ;
	switch( svc_number )
	{
		case YIELD:
			//Pend an interrupt to do the context switch
			_ICSR |= 1<<28;
			__asm("isb");
			break;

		case PRINT_HELLO:
			printf("Hello \r\n");
			break;

		case PRINT_WORLD: //17 is sort of arbitrarily chosen
			printf("World\r\n");
			break;

		case RUN_FIRST_THREAD:
			__set_PSP((uint32_t)osThreads[curr_thread].sp);
			runFirstThread();
			break;

		default: /* unknown SVC */
			break;
	}
}

uint32_t* allocate_stack(){
	uint32_t* pointer = (uint32_t)MSP_INIT_VAL - (OFFSET_INCREMENT*OFFSET);
	return pointer;
}

void osKernelInitialize(){
	MSP_INIT_VAL = *(uint32_t**)0x0;
	OFFSET = 1;
	curr_thread = 0;
	//set the priority of PendSV to almost the weakest
	SHPR3 |= 0xFE << 16; //shift the constant 0xFE 16 bits to set PendSV priority
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC higher than PendSV
}

void osKernelStart(void){
	__asm("SVC #3");
}

bool osCreateThread(void* thread_function, void* args){
	if(OFFSET*OFFSET_INCREMENT >= STACK_SIZE){
		return false;
	}

	osThreads[OFFSET-1].sp = allocate_stack();
	osThreads[OFFSET-1].thread_function = thread_function;
	osThreads[OFFSET-1].runtime = 5;
	osThreads[OFFSET-1].timeslice = 5;

	*(--osThreads[OFFSET-1].sp) = 1<<24;
	*(--osThreads[OFFSET-1].sp) = osThreads[OFFSET-1].thread_function;
	for(int i = 0; i < 5; i++){
	  *(--osThreads[OFFSET-1].sp) = 0xA;
	}
	*(--osThreads[OFFSET-1].sp) = args;
	for(int i = 0; i < 8; i++){
		  *(--osThreads[OFFSET-1].sp) = 0xA;
		}
	OFFSET++;

	return true;
}

bool osThreadCreateWithDeadline(void* thread_function, void* args, uint32_t deadline){
	if(OFFSET*OFFSET_INCREMENT >= STACK_SIZE){
		return false;
	}

	osThreads[OFFSET-1].sp = allocate_stack();
	osThreads[OFFSET-1].thread_function = thread_function;
	osThreads[OFFSET-1].runtime = deadline;
	osThreads[OFFSET-1].timeslice = deadline;

	*(--osThreads[OFFSET-1].sp) = 1<<24;
	*(--osThreads[OFFSET-1].sp) = osThreads[OFFSET-1].thread_function;
	for(int i = 0; i < 5; i++){
	  *(--osThreads[OFFSET-1].sp) = 0xA;
	}
	*(--osThreads[OFFSET-1].sp) = args;
	for(int i = 0; i < 8; i++){
		  *(--osThreads[OFFSET-1].sp) = 0xA;
		}
	OFFSET++;

	return true;
}

void osSched(){
	osThreads[curr_thread].sp = (uint32_t*)(__get_PSP() - 8*4);
	curr_thread = (++curr_thread)%num_threads;
	__set_PSP((uint32_t)osThreads[curr_thread].sp);
	return;
}

void osYield(void){
	osThreads[curr_thread].runtime = osThreads[curr_thread].timeslice;
	__asm("SVC #1");
}

void run_thread(){
	uint32_t* PSP = allocate_stack();
	__set_PSP(*PSP);
	stackptr = PSP;
	*(--stackptr) = 1<<24;
	*(--stackptr) = (uint32_t)print_continuously;
	for(int i = 0; i < 14; i++){
	  *(--stackptr) = 0xA;
	}
}



