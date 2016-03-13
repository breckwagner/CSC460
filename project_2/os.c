#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// Not sure which one we will use so including them both here
#include "queue.h"    // OpenBSD implementation of lists
#include "adlist.h"   // doubly linked list by Salvatore Sanfilippo

#include "os.h"
#include "kernal.h"
#include "common.h"

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

volatile uint32_t ticks = 0;

//
volatile static ProcessDescriptor* running_queue[MINPRIORITY][MAXTHREAD];

// A list of process_descriptor(s)
volatile static ProcessDescriptor* sleep_queue[MAXTHREAD];

//
volatile static uint8_t running_queue_pointer[MAXTHREAD];

//
volatile static uint8_t sleep_queue_pointer;

//
volatile uint8_t * kernel_stack_pointer;

//
volatile uint8_t * current_stack_pointer;


volatile static uint16_t next_process;

volatile static uint16_t KernelActive;

volatile static uint16_t tasks;

static ProcessDescriptor Process[MAXTHREAD];

volatile static ProcessDescriptor* current_process;

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/

static PID Kernel_Create_Task(voidfuncptr f);

void Kernel_Create_Task_At(ProcessDescriptor *process, voidfuncptr function);

static void Dispatch();

static void Next_Kernel_Request();

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

void Kernel_Create_Task_At(ProcessDescriptor *process, voidfuncptr function) {
  uint8_t * stack_pointer;
  stack_pointer = (uint8_t *) &(process->workSpace[WORKSPACE-1]);
  memset(&(process->workSpace), 0, WORKSPACE);

  *(uint8_t *)stack_pointer-- = ((uint16_t)Task_Terminate) & 0xff;
  *(uint8_t *)stack_pointer-- = (((uint16_t)Task_Terminate) >> 8) & 0xff;
  *(uint8_t *)stack_pointer-- = 0x00;

  *(uint8_t *)stack_pointer-- = ((uint16_t)function) & 0xff;
  *(uint8_t *)stack_pointer-- = (((uint16_t)function) >> 8) & 0xff;
  *(uint8_t *)stack_pointer-- = 0x00;

  // Decrement stack pointer for the 32 registers and the EIND
  stack_pointer -= 34;

  process->stack_pointer = stack_pointer;
  process->code = function;
  process->request = NONE;
  process->state = READY;
}

static void schedule_task (ProcessDescriptor * pd) {
  uint8_t flag = disable_global_interrupts();
  // LOOK FOR ROOM IN QUEUE
  for (int i=running_queue_pointer[pd.priority]; running_queue[pd.priority][i] !=NULL;i++);
  running_queue[pd.priority][i] = pd;
  restore_global_interrupts(flag);
}

static PID Kernel_Create_Task(voidfuncptr f) {
   PID id = NULL;
   if (tasks == MAXTHREAD) return 0;

   /* find a DEAD ProcessDescriptor that we can use  */
   for (id = 1; id < MAXTHREAD; id++) {
       if (Process[id-1].state == DEAD) break;
   }

   ++tasks;
   Kernel_Create_Task_At( &(Process[id-1]), f );

   schedule_task (&(Process[id-1]));

   return id;
}

static void Dispatch() {
  /*
  if(listFirst(running_queue) != NULL) {
    list * l = listNodeValue(listFirst(running_queue));
    if (listFirst(l) != NULL) {
      current_process = listNodeValue(listFirst(running_queue));
      current_stack_pointer = current_process->stack_pointer;
      current_process->state = RUNNING;
    }
  }*/


  // THE OLD code
  ///*
  while(Process[next_process].state != READY) {
    next_process = (next_process + 1) % MAXTHREAD;
  }
  current_process = &(Process[next_process]);
  current_stack_pointer = current_process->stack_pointer;
  current_process->state = RUNNING;
  next_process = (next_process + 1) % MAXTHREAD;
  //*/

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
        signal_debug(2, true);
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
 * @see os.h
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
  uint8_t sreg = disable_global_interrupts();

  arg = current_process->argument;

  restore_global_interrupts(sreg);
  return arg;
}

void task_suspend(void) {Task_Suspend(current_process->id);}

void Task_Suspend(PID p) {
  Process[p-1].state = SUSPENDED;
  if (current_process->id==p) Task_Yield();
}

void task_resume(void) {Task_Resume(current_process->id);}

void Task_Resume(PID p) {
  Process[p-1].state = READY;
}

void task_sleep(PID p, TICK t) {
  // TODO
}

void Task_Sleep(TICK t) {
  uint8_t flag = disable_global_interrupts();
  listNode *node;
  if (NULL != sleep_queue) {
    listIter *it = listGetIterator(sleep_queue, AL_START_HEAD);
    while ((node = listNext(it))) {
      if (node == NULL) {
        listAddNodeHead(sleep_queue, current_process);
        break;
      } else if(((ProcessDescriptor *)listNodeValue(node))->expires < current_process->expires) {
        // TODO: for now this is fine but later we should detect overflow
        // and when the time is greater then the size of ticks (uint32_t)
        // we should add a helper task to the queue that will then add
        // current_process to the queue after length max size of uint32_t has
        // passed
        listInsertNode(sleep_queue, node, current_process, true);
      }
    }
    listReleaseIterator(it);
  }
  Task_Suspend(current_process->id);
  restore_global_interrupts(flag);
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
  running_queue = listCreate();
  sleep_queue = listCreate();
  //LIST_INIT(&processes_head);


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
  if (KernelActive) Task_Yield();
}

void init_timer () {
  // initialize Timer1
  uint8_t interrupt_flag = disable_global_interrupts();
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
