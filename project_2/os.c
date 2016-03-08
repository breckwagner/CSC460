#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

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

typedef struct process_descriptor {
  unsigned char *stack_pointer;
  unsigned char workSpace[WORKSPACE];
  PROCESS_STATES state;
  voidfuncptr code;
  KERNEL_REQUEST_TYPE request;
  PRIORITY priority;
} ProcessDescriptor;

static ProcessDescriptor Process[MAXTHREAD];

volatile static ProcessDescriptor* current_process;

/*******************************************************************************
 * Function Definitions (utility)
 ******************************************************************************/

#define soft_reset() do{wdt_enable(WDTO_15MS);for(;;){}}while(0) // TODO update

/**
 * Enables inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t Enable_Interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("sei"::);
  return sreg;
}

/**
 * Disables inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t Disable_Interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("cli"::);
  return sreg;
}

/**
 * Restores inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t Restore_Interrupts(uint8_t saved_sreg) {
  if (saved_sreg & 0x80) {
    return Enable_Interrupts();
  } else {
    return Disable_Interrupts();
  }
}

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At(ProcessDescriptor *process, voidfuncptr function){
  uint8_t * stack_pointer;
  stack_pointer = (unsigned char *) &(process->workSpace[WORKSPACE-1]);
  memset(&(process->workSpace), 0, WORKSPACE);

  //Notice that we are placing the address (16-bit) of the functions
  //onto the stack in reverse byte order (least significant first, followed
  //by most significant).  This is because the "return" assembly instructions
  //(rtn and rti) pop addresses off in BIG ENDIAN (most sig. first, least sig.
  //second), even though the AT90 is LITTLE ENDIAN machine.

  *(unsigned char *)stack_pointer-- = ((unsigned int)Task_Terminate) & 0xff;
  *(unsigned char *)stack_pointer-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;

  //Place return address of function at bottom of stack
  *(unsigned char *)stack_pointer-- = ((unsigned int)f) & 0xff;
  //Store terminate at the bottom of stack to protect against stack underrun.
  *(unsigned char *)stack_pointer-- = (((unsigned int)f) >> 8) & 0xff;
  *(unsigned char *)stack_pointer-- = 0;

  // Decrement stack pointer for the 32 registers and the EIND
  stack_pointer -= 34;

  process->stack_pointer = stack_pointer;
  process->code = function;
  process->request = NONE;
  process->state = READY;
}


/**
  *  Create a new task
  */
static PID Kernel_Create_Task(voidfuncptr f) {
   int id = NULL;
   if (Tasks == MAXTHREAD) return id;

   /* find a DEAD ProcessDescriptor that we can use  */
   for (id = 0; id < MAXTHREAD; id++) {
       if (Process[id].state == DEAD) break;
   }

   ++Tasks;
   Kernel_Create_Task_At( &(Process[id]), f );
   return id;
}

/*******************************************************************************
 * Function Definitions (os.h)
 ******************************************************************************/

void OS_Abort(void) {
  Disable_Interrupts();
  for (int i = 0; i < 42; i++) {
    // BLINK LED 13
  }
  soft_reset();
}

PID Task_Create( void (*f)(void), PRIORITY py, int arg) {
  PID pid;
  if (KernelActive) {
    uint8_t inturupt_flag = Disable_Interrupt();
    current_process->request = CREATE;
    current_process->code = f;
    Enter_Kernel();
    Restore_Interrupts(inturupt_flag)

  } else {
     /* call the RTOS function directly */
     pid = Kernel_Create_Task(f);
  }
  Process[pid].priority = py;
  return pid;
}

void Task_Terminate(void) {
  if (KernelActive) {
    Disable_Interrupts();
    current_process->request = TERMINATE;
    Enter_Kernel();
  }
}

void Task_Yield(void) {
  if (KernelActive) {
    Disable_Interrupts();
    current_process->request = NEXT;
    Enter_Kernel();
  }
}

/**
 * @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void) {
  int arg;
  uint8_t sreg;

  // Save interupt flag
  sreg = SREG;
  Disable_Interrupt();

  arg = cur_task->arg;

  // Restore interupt flag
  SREG = sreg;
  return arg;
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
     memset(&(Process[x]),0,sizeof(ProcessDescriptor));
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
