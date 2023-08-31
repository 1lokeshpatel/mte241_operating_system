/*
 * kernel.h
 *
 *  Created on: Jun. 29, 2023
 *      Author: lokesh
 */

#ifndef INC_KERNEL_H_
#define INC_KERNEL_H_

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 // PendSV is bits 23-16
#define _ICSR *(uint32_t*)0xE000ED04 //This lets us trigger PendSV

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

extern void print_continuously();
extern void runFirstThread(void);
extern uint32_t* stackptr;
extern uint32_t PSP;

typedef struct k_thread{
	uint32_t* sp; //stack pointer
	void (*thread_function)(void*); //function pointer
	uint32_t timeslice;
	uint32_t runtime;
}thread;

typedef struct{
	bool value;
	tcb_queue_t waiting;
}mutex;

void SVC_Handler_Main( unsigned int *svc_args );
uint32_t* allocate_stack();
void run_thread();
bool osCreateThread(void* thread_function, void* args);
bool osThreadCreateWithDeadline(void* thread_function, void* args, uint32_t deadline);
void osKernelInitialize();
void osKernelStart(void);
void osSched();
void osYield(void);

#endif /* INC_KERNEL_H_ */
