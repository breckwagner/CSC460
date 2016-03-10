#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "queue.h"
#include "os.h"
#include "kernal.h"
#include "common.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

volatile uint16_t ticks = 0;

LIST_HEAD(process_descriptor_head, process_descriptor) processes_head;

volatile uint8_t * kernel_stack_pointer;

volatile uint8_t * current_stack_pointer;

volatile static unsigned int next_process;

volatile static unsigned int KernelActive;

volatile static unsigned int tasks;

static ProcessDescriptor Process[MAXTHREAD];

volatile static ProcessDescriptor* current_process;

/*******************************************************************************
 * Function Definitions (utility)
 ******************************************************************************/

/**
 * When creating a new task, it is important to initialize its stack just like
 * it has called "Enter_Kernel()"; so that when we switch to it later, we
 * can just restore its execution context on its stack.
 * (See file "cswitch.S" for details.)
 */
void Kernel_Create_Task_At(ProcessDescriptor *process, voidfuncptr function){
  uint8_t * stack_pointer;
  stack_pointer = (uint8_t *) &(process->workSpace[WORKSPACE-1]);
  memset(&(process->workSpace), 0, WORKSPACE);

  *(uint8_t *)stack_pointer-- = ((uint16_t)Task_Terminate) & 0xff;
  *(uint8_t *)stack_pointer-- = (((uint16_t)Task_Terminate) >> 8) & 0xff;
  *(uint8_t *)stack_pointer-- = 0x00;

  //Place return address of function at bottom of stack
  *(uint8_t *)stack_pointer-- = ((uint16_t)function) & 0xff;
  //Store terminate at the bottom of stack to protect against stack underrun.
  *(uint8_t *)stack_pointer-- = (((uint16_t)function) >> 8) & 0xff;
  *(uint8_t *)stack_pointer-- = 0x00;

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
   PID id = NULL;
   if (tasks == MAXTHREAD) return 0;

   /* find a DEAD ProcessDescriptor that we can use  */
   for (id = 1; id < MAXTHREAD; id++) {
       if (Process[id-1].state == DEAD) break;
   }

   ++tasks;
   Kernel_Create_Task_At( &(Process[id-1]), f );
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

    switch (current_process->request) {
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

PID Task_Create(void (*f)(void), PRIORITY py, int arg) {
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
  Process[pid-1].priority = py;
  return pid;
}

void Task_Terminate(void) {
  if (KernelActive) {
    uint8_t flag = disable_global_interrupts();
    current_process->request = TERMINATE;
    Enter_Kernel();
    restore_global_interrupts(flag);
  }
}

void Task_Yield(void) {
  if (KernelActive) {
    uint8_t flag = disable_global_interrupts();
    current_process->request = NEXT;
    Enter_Kernel();
    restore_global_interrupts(flag);
  }
}

/**
 * @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void) {
  int arg;
  uint8_t sreg;
  // Save interupt flag
  sreg = disable_global_interrupts();

  arg = current_process->argument;

  restore_global_interrupts(sreg);
  return arg;
}

void Task_Suspend(PID p) {
  Process[p-1].state = SUSPENDED;

  //if (&current_process==&Process+p) {
  //  Task_Yield();
  //}
}

void Task_Resume(PID p) {
  Process[p-1].state = READY;
}

void Task_Sleep(TICK t) {
  //Task_Create(helper_function, 0, current_process-Process)
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




/*******************************************************************************
 * OS API END
 ******************************************************************************/

/**
 * This function initializes the RTOS and must be called before any other
 * system calls.
 */
void OS_Init() {
  int id;
  tasks = 0;
  KernelActive = 0;
  next_process = 0;
  //Reminder: Clear the memory for the task on creation.
  for (id = 1; id < MAXTHREAD; id++) {
    memset(&(Process[id-1]), 0, sizeof(ProcessDescriptor));
    Process[id-1].state = DEAD;
  }

  LIST_INIT(&processes_head);


}


/**
 * This function starts the RTOS after creating a few tasks.
 */
void OS_Start() {
  if ((!KernelActive) && (tasks > 0)) {
    //uint8_t flag =
    disable_global_interrupts();
    KernelActive = 1;
    Next_Kernel_Request();
  }
}

ISR(TIMER1_COMPA_vect) {
  ++ticks;
  Task_Yield();
}

void init_timer () {
  // initialize Timer1
  uint8_t interrupt_flag = disable_global_interrupts();          // disable global interrupts
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B

  // set compare match register to desired timer count:
  OCR1A = 625;

  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);

  // Set CS12 bits for 256 prescaler:
  //TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);

  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);

  // enable global interrupts:
  restore_global_interrupts(interrupt_flag);
}

// OS_Init function
void kernal_init(void) {
  DDRB = 0xFF;
  DDRL = 0xFF;

  OS_Init();

  // Add main to task queue to be defined later
  Task_Create(main, 0, 0);
  init_timer();
  OS_Start();
}
