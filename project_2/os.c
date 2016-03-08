#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "os.h"


typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

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

static PD Process[MAXTHREAD];

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
  if (KernelActive) {
    Disable_Interrupts();
    Cp->request = CREATE;
    Cp->code = f;
    Enter_Kernel();
  } else {
    // START OF Kernel_Create_Task( voidfuncptr f )
    int x;
    if (Tasks == MAXTHREAD) return NULL; // TOO MANY TAKS, FAIL
    for (x = 0; x < MAXTHREAD; x++) {
      if (Process[x].state == DEAD) break;
    }
    ++Tasks;

    PD *p = &(Process[x]);

    // START OF Kernel_Create_Task_At( &(Process[x]), f );
    unsigned char *sp;
    sp = (unsigned char *) &(p->workSpace[WORKSPACE-1]);
    memset(&(p->workSpace), 0, WORKSPACE);
    //Store terminate at the bottom of stack to protect against stack underrun.
    *(unsigned char *)sp-- = ((unsigned int)Task_Terminate) & 0xff;
    *(unsigned char *)sp-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;
    *(unsigned char *)sp-- = 0;

    //Place return address of function at bottom of stack
    *(unsigned char *)sp-- = ((unsigned int)f) & 0xff;
    *(unsigned char *)sp-- = (((unsigned int)f) >> 8) & 0xff;
    *(unsigned char *)sp-- = 0;

    sp = sp - 34;
    p->sp = sp;		/* stack pointer into the "workSpace" */
    p->code = f;		/* function to be executed as a task */
    p->request = NONE;
    p->state = READY;
  }
  return 0; // TODO
}

void Task_Terminate(void) {
  if (KernelActive) {
    Disable_Interrupts();
    Cp -> request = TERMINATE;
    Enter_Kernel();
  }
}

void Task_Yield(void) {
  if (KernelActive) {
    Disable_Interrupts();
    Cp->request = NEXT;
    Enter_Kernel();
  }
}

int Task_GetArg(void) {
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

void main_r () {return;}

// OS_Init function
int main() {
  /*
  int x;
  Tasks = 0;
  KernelActive = 0;
  NextP = 0;
 //Reminder: Clear the memory for the task on creation.
  for (x = 0; x < MAXPROCESS; x++) {
     memset(&(Process[x]),0,sizeof(PD));
     Process[x].state = DEAD;
  }

  // OS START
  if ((!KernelActive) && (Tasks > 0)) {
    Disable_Interrupt();
    KernelActive = 1;
    Task_Create(main_r, 0, NULL);
    Next_Kernel_Request();
    // NEVER RETURNS!!!
  }*/
  return 0;
}
