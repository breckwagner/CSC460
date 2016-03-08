#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "os.h"
#include "kernal.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

volatile unsigned char * kernel_stack_pointer;

volatile unsigned char * current_stack_pointer;

volatile static unsigned int next_process;

volatile static unsigned int KernelActive;

volatile static unsigned int tasks;

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
uint8_t enable_global_interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("sei"::);
  return sreg;
}

/**
 * Disables inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t disable_global_interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("cli"::);
  return sreg;
}

/**
 * Restores inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t restore_global_interrupts(uint8_t saved_sreg) {
  if (saved_sreg & 0x80) {
    return enable_global_interrupts();
  } else {
    return disable_global_interrupts();
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

  *(unsigned char *)stack_pointer-- = ((unsigned int)Task_Terminate) & 0xff;
  *(unsigned char *)stack_pointer-- = (((unsigned int)Task_Terminate) >> 8) & 0xff;

  //Place return address of function at bottom of stack
  *(unsigned char *)stack_pointer-- = ((unsigned int)function) & 0xff;
  //Store terminate at the bottom of stack to protect against stack underrun.
  *(unsigned char *)stack_pointer-- = (((unsigned int)function) >> 8) & 0xff;
  *(unsigned char *)stack_pointer-- = 0;

  // Decrement stack pointer for the 32 registers and the EIND
  stack_pointer -= 34;

  process->stack_pointer = stack_pointer;
  process->code = function;
  process->request = NONE;
  process->state = READY;
}


/**
 * Create a new task
 */
static PID Kernel_Create_Task(voidfuncptr f) {
   int id = NULL;
   if (tasks == MAXTHREAD) return id;

   /* find a DEAD ProcessDescriptor that we can use  */
   for (id = 0; id < MAXTHREAD; id++) {
       if (Process[id].state == DEAD) break;
   }

   ++tasks;
   Kernel_Create_Task_At( &(Process[id]), f );
   return id;
}

static void Dispatch() {
  while(Process[next_process].state != READY) {
    next_process = (next_process + 1) % MAXTHREAD;
  }
  current_process = &(Process[next_process]);
  current_stack_pointer = current_process->stack_pointer;
  current_process->state = RUNNING;
  next_process = (next_process + 1) % MAXTHREAD;
}

static void Next_Kernel_Request() {
  Dispatch();  /* select a new task to run */

  for (;;) {
    current_process->request = NONE; /* clear its request */

    /* activate this newly selected task */
    current_stack_pointer = current_process->stack_pointer;
    Exit_Kernel();    /* or CSwitch() */

    /* if this task makes a system call, it will return to here! */

    /* save the Cp's stack pointer */
    current_process->stack_pointer = current_stack_pointer;

    switch(current_process->request){
      case CREATE:
        Kernel_Create_Task (current_process->code);
        break;
      case NEXT:
      case NONE:
        /* NONE could be caused by a timer interrupt */
        current_process->state = READY;
        Dispatch();
        break;
      case TERMINATE:
        /* deallocate all resources used by this task */
        current_process->state = DEAD;
        Dispatch();
        break;
      default: break;
    }
  }
}

/*******************************************************************************
 * Function Definitions OS API
 * see os.h
 ******************************************************************************/

void OS_Abort(void) {
  disable_global_interrupts();
  for (int i = 0; i < 42; i++) {
    // BLINK LED 13
  }
  soft_reset();
}

PID Task_Create( void (*f)(void), PRIORITY py, int arg) {
  PID pid;
  if (KernelActive) {
    uint8_t inturupt_flag = disable_global_interrupts();
    current_process->request = CREATE;
    current_process->code = f;
    Enter_Kernel();
    restore_global_interrupts(inturupt_flag);

  } else {
     /* call the RTOS function directly */
     pid = Kernel_Create_Task(f);
  }
  Process[pid].priority = py;
  return pid;
}

void Task_Terminate(void) {
  if (KernelActive) {
    disable_global_interrupts();
    current_process->request = TERMINATE;
    Enter_Kernel();
  }
}

void Task_Yield(void) {
  if (KernelActive) {
    disable_global_interrupts();
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
  disable_global_interrupts();

  arg = current_process->argument;

  // Restore interupt flag
  SREG = sreg;
  return arg;
}

void Task_Suspend(PID p) {

}

void Task_Resume(PID p) {

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

void main_r () {
  for(;;){

  }
}

// OS_Init function
int main(void) {

  int id;
  tasks = 0;
  KernelActive = 0;
  next_process = 0;
 //Reminder: Clear the memory for the task on creation.
  for (id = 0; id < MAXTHREAD; id++) {
     memset(&(Process[id]),0,sizeof(ProcessDescriptor));
     Process[id].state = DEAD;
  }

  // OS START
  if ((!KernelActive) && (tasks > 0)) {
    disable_global_interrupts();
    KernelActive = 1;
    Task_Create(main_r, 0, 0);
    //Next_Kernel_Request();
    // NEVER RETURNS!!!
  }
  return 0;
}
