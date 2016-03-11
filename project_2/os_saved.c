#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

// Not sure which one we will use so including them both here
#include "queue.h"  // OpenBSD implementation of lists
#include "adlist.h" // doubly linked list by Salvatore Sanfilippo

#include "os.h"
#include "kernal.h"
#include "common.h"

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#ifndef F_CPU
#define F_CPU 16000000UL // set the CPU clock
#endif

#define BAUD 9600                           // define baud
#define BAUDRATE ((F_CPU)/(BAUD*16UL)-1)    // set baud rate value for UBRR

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

volatile uint32_t ticks = 0;

// LIST_HEAD(process_descriptor_head, process_descriptor) processes_head;

// This is a queue of queues. Each queue here represents a level of priority
list *running_queue;

// A list of process_descriptor(s)
list *sleep_queue;

volatile uint8_t *kernel_stack_pointer;

volatile uint8_t *current_stack_pointer;

volatile static uint16_t next_process;

volatile static uint16_t KernelActive;

volatile static uint16_t tasks;

static ProcessDescriptor Process[MAXTHREAD];

volatile static ProcessDescriptor *current_process;

volatile static listNode *current_process_node;

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/

static PID kernel_create_task(voidfuncptr f);

void kernel_create_task_at(ProcessDescriptor *process, voidfuncptr function);

static void dispatch();

static void next_kernel_request();

static void _schedule_task(ProcessDescriptor *pd, list * running_queue);

static void schedule_task(ProcessDescriptor *pd);

PRIORITY _task_change_priority (PRIORITY priority);

PRIORITY task_change_priority (PID id, PRIORITY priority);

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/

PRIORITY _task_change_priority (PRIORITY priority) {
  return task_change_priority (current_process->id, priority);
}

PRIORITY task_change_priority (PID id, PRIORITY priority) {
  PRIORITY old_priority = Process[id-1].priority;
  // TODO: remove process from old_priority level in queue

  // Change process priority in ProcessDescriptor
  Process[id-1].priority = priority;
  // Add process back into queue
  schedule_task(&(Process[id - 1]));
  return old_priority;
}

void kernel_create_task_at(ProcessDescriptor *process, voidfuncptr function) {
  uint8_t *stack_pointer;
  stack_pointer = (uint8_t *)&(process->workSpace[WORKSPACE - 1]);
  memset(&(process->workSpace), 0, WORKSPACE);

  // When the task returns (if it returns before calling Task_Terminate),
  // Task_Terminate is called using this
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
  process->id = (Process - process + 1);
}

static void _schedule_task(ProcessDescriptor *pd, list * queue) {
  uint8_t flag = disable_global_interrupts();
  listNode *node;
	listIter *it = listGetIterator(queue, AL_START_HEAD);
	for (;;) {
		node = listNext(it);
		if (node == NULL) {
			// Add priority level to queue
			list *l = listCreate();
			listAddNodeTail(queue, l);
			listAddNodeTail(l, pd);
			break;
		} else {
			//
			listNode * sub_queue_node = listFirst((list *)listNodeValue(node));
			if (sub_queue_node == NULL) {
				// shouldn't happen but if it does remove it
				break;
			}
			ProcessDescriptor * sub_queue_head_element = ((ProcessDescriptor *)listNodeValue(sub_queue_node));
			list * sub_queue = listNodeValue(sub_queue_node);
			if(sub_queue_head_element->priority == pd->priority) {
				listAddNodeTail((list *)listNodeValue(node), pd);
				break;
			} else if (sub_queue_head_element->priority > pd->priority) {
				list *l = listCreate();
				listInsertNode(queue, node, l, false);
				listAddNodeTail(l, pd);
				break;
			}
		}
	}
	listReleaseIterator(it);
  restore_global_interrupts(flag);
}

static void schedule_task(ProcessDescriptor *pd) {
  _schedule_task(pd, running_queue);
}

static PID kernel_create_task(voidfuncptr f) {
  PID id = NULL;
  if (tasks == MAXTHREAD)
    return 0;

  // find a DEAD ProcessDescriptor that we can use
  for (id = 1; id < MAXTHREAD; id++) {
    if (Process[id - 1].state == DEAD)
      break;
  }

  ++tasks;
  kernel_create_task_at(&(Process[id - 1]), f);

  schedule_task(&(Process[id - 1]));

  return id;
}

static void dispatch() {

  if(listFirst(running_queue) != NULL) {
    list * l = listNodeValue(listFirst(running_queue));

    if (listLength(l) != 0) {
      //current_process = ((ProcessDescriptor *)listNodeValue(listFirst(l)))->id
      signal_debug(5, true);
      /*current_process = listNodeValue(listFirst(running_queue));
      current_stack_pointer = current_process->stack_pointer;
      current_process->state = RUNNING;*/
    }
  }

  // THE OLD code
  ///*
  while (Process[next_process].state != READY) {
    next_process = (next_process + 1) % MAXTHREAD;
  }
  current_process = &(Process[next_process]);
  current_stack_pointer = current_process->stack_pointer;
  current_process->state = RUNNING;
  next_process = (next_process + 1) % MAXTHREAD;
  //*/
}

static void next_kernel_request() {
  goto ENTER;
  for (;;) {
    // Task System calls return here at Exit_Kernel
    Exit_Kernel();

    // save the current_process's stack pointer
    current_process->stack_pointer = current_stack_pointer;

    switch (current_process->request) {

    case CREATE:
      kernel_create_task(current_process->code);
      break;

    case NEXT:
    case NONE: // NONE could be caused by a timer interrupt
      signal_debug(2, true);

      //
      current_process->state = READY;

      // begin process of switching context to next task in queue
      ENTER: dispatch();
      break;

    case TERMINATE:
      // deallocate all resources used by this task
      current_process->state = DEAD;
      --tasks;
      dispatch();
      break;

    default:
      OS_Abort();
      break;
    }
    // Clear request
    current_process->request = NONE;
    // activate newly selected task
    current_stack_pointer = current_process->stack_pointer;
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
    pid = kernel_create_task(f);
  }
  Process[pid - 1].priority = py;
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

void task_suspend(void) { Task_Suspend(current_process->id); }

void Task_Suspend(PID p) {
  Process[p - 1].state = SUSPENDED;
  if (current_process->id == p)
    Task_Yield();
}

void task_resume(void) { Task_Resume(current_process->id); }

void Task_Resume(PID p) { Process[p - 1].state = READY; }

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
      } else if (((ProcessDescriptor *)listNodeValue(node))->expires <
                 current_process->expires) {
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

void Mutex_Lock(MUTEX m) {}

void Mutex_Unlock(MUTEX m) {}

EVENT Event_Init(void) {
  // TODO
  return 0;
}

void Event_Wait(EVENT e) {}

void Event_Signal(EVENT e) {}

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
  // Reminder: Clear the memory for the task on creation.
  for (id = 1; id < MAXTHREAD; id++) {
    memset(&(Process[id - 1]), 0, sizeof(ProcessDescriptor));
    Process[id - 1].state = DEAD;
  }
  running_queue = listCreate();
  sleep_queue = listCreate();
}

/**
 * This function starts the RTOS after creating a few tasks.
 */
void OS_Start() {
  if ((!KernelActive) && (tasks > 0)) {
    // uint8_t flag =
    disable_global_interrupts();
    KernelActive = 1;
    next_kernel_request();
  }
}

ISR(TIMER1_COMPA_vect) {
  ++ticks;
  if (sleep_queue)
    Task_Yield();
}

void init_timer() {
  // initialize Timer1
  uint8_t interrupt_flag = disable_global_interrupts();
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0; // same for TCCR1B

  // set compare match register to desired timer count:
  OCR1A = 625;

  // turn on CTC mode:
  TCCR1B |= (1 << WGM12);

  // Set CS12 bits for 256 prescaler:
  // TCCR1B |= (1 << CS10);
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
