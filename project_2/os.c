#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "os.h"


typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

#define WORKSPACE     256

#define MAXPROCESS   4

volatile unsigned char *KernelSp;

volatile unsigned char *CurrentSp;

volatile static unsigned int NextP;

volatile static unsigned int KernelActive;

volatile static unsigned int Tasks;

extern void CSwitch();

extern void Exit_Kernel();

extern void Enter_Kernel();

int main();

typedef enum process_states {
  DEAD = 0,
  READY,
  RUNNING
} PROCESS_STATES;

typedef enum kernel_request_type {
  NONE = 0,
  CREATE,
  NEXT,
  TERMINATE
} KERNEL_REQUEST_TYPE;

typedef struct ProcessDescriptor {
  unsigned char *sp;
  unsigned char workSpace[WORKSPACE];
  PROCESS_STATES state;
  voidfuncptr code;
  KERNEL_REQUEST_TYPE request;
} PD;

static PD Process[MAXPROCESS];

volatile static PD* Cp;

/*******************************************************************************
 * Function Definitions (utility)
 ******************************************************************************/

 void Disable_Interrupts() {
   asm volatile ("cli"::);
 }

 void Enable_Interrupts() {
   asm volatile ("sei"::);
 }

/*******************************************************************************
 * Function Definitions (os.h)
 ******************************************************************************/

void OS_Abort(void) {

}

PID  Task_Create( void (*f)(void), PRIORITY py, int arg) {
  // TODO
  return 0;
}

void Task_Terminate(void) {

}

void Task_Yield(void) {
  if (KernelActive) {
    Disable_Interrupts();
    Cp->request = NEXT;
    Enter_Kernel();
  }
}

int  Task_GetArg(void) {
  // TODO
  return 0;
}
void Task_Suspend( PID p ) {

}
void Task_Resume( PID p ) {

}

void Task_Sleep(TICK t) {

}

MUTEX Mutex_Init(void) {
  // TODO
  return 0;
}
void Mutex_Lock(MUTEX m) {

}
void Mutex_Unlock(MUTEX m) {

}

EVENT Event_Init(void) {
  // TODO
  return 0;
}
void Event_Wait(EVENT e) {

}
void Event_Signal(EVENT e) {

}


int main() {return 0;}