#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>

#include "os.h"
#include "kernel.h"
#include "common.h"

/*******************************************************************************
 * MACROS
 ******************************************************************************/

#ifndef F_CPU
#define F_CPU 16000000UL // set the CPU clock
#endif

#define BAUD 9600                              // define baud
#define BAUDRATE ((F_CPU) / (BAUD * 16UL) - 1) // set baud rate value for UBRR

/*******************************************************************************
 * Global Variables
 ******************************************************************************/

volatile uint32_t ticks = 0;

volatile static ProcessDescriptor* running_queue[MINPRIORITY+1];

volatile static ProcessDescriptor * sleep_queue;

volatile uint8_t * kernel_stack_pointer;

volatile uint8_t * current_stack_pointer;

volatile static uint16_t KernelActive;

volatile static uint16_t tasks;

volatile static ProcessDescriptor Process[MAXTHREAD];

volatile static ProcessDescriptor *current_process;

volatile static MutexLock Lock[MAXMUTEX];

volatile static MutexLock *current_lock;

volatile static MutexLock *next_lock;

volatile static uint16_t num_mutexes;

/*******************************************************************************
 * Function Declarations
 ******************************************************************************/

static PID kernel_create_task(voidfuncptr f);

void kernel_create_task_at(volatile ProcessDescriptor *process, voidfuncptr function);

void static enqueue(volatile ProcessDescriptor * pd);

void static dequeue(volatile ProcessDescriptor * pd);

static void dispatch();

static void next_kernel_request();

static void schedule_task(volatile ProcessDescriptor *pd);

PRIORITY _task_change_priority(PRIORITY priority);

PRIORITY task_change_priority(PID id, PRIORITY priority);

/*******************************************************************************
 * Function Definitions
 ******************************************************************************/



PRIORITY _task_change_priority(PRIORITY priority) {
  return task_change_priority(current_process->id, priority);
}

PRIORITY task_change_priority(PID id, PRIORITY priority) {
  // TODO: check if process is in queue before dequeue
  dequeue(&Process[id - 1]);
  PRIORITY old_priority = Process[id - 1].priority;
  enqueue(&Process[id - 1]);
  return old_priority;
}

void kernel_create_task_at(volatile ProcessDescriptor *process, voidfuncptr function) {
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
  process->id = ((process-Process) + 1);
  process->suspended = false;
}



void static enqueue(volatile ProcessDescriptor * pd) {
  if(running_queue[pd->priority]==NULL) {
    running_queue[pd->priority] = pd;
    pd->next = pd;
    pd->prev = pd;
  } else {
    running_queue[pd->priority]->prev->next = pd;
    pd->prev = running_queue[pd->priority]->prev;
    running_queue[pd->priority]->prev = pd;
    pd->next = running_queue[pd->priority];
  }
}

void static dequeue(volatile ProcessDescriptor * pd) {
  if(pd == pd->next) {
    signal_debug(4,true);
    running_queue[pd->priority] = NULL;
  } else {
    pd->next->prev = pd->prev;
    pd->prev->next = pd->next;
  }
  if(pd == running_queue[pd->priority]) {
    running_queue[pd->priority] = pd->next;
  }
  pd->next = NULL;
  pd->prev = NULL;
}

static void schedule_task(volatile ProcessDescriptor *pd) {
  uint8_t flag = disable_global_interrupts();
  enqueue(pd);
  restore_global_interrupts(flag);
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
  for (int i = 0; true; i=(i+1)%(MINPRIORITY+1)) {
    if(running_queue[i]!=NULL) {
      current_process = running_queue[i] = running_queue[i]->next;
      break;
    }
  }
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

      //
      current_process->state = READY;

    // begin process of switching context to next task in queue
      ENTER:
      dispatch();
      break;

    case TERMINATE:
      // deallocate all resources used by this task
      current_process->state = DEAD;

      dequeue(current_process);
      // decrement current task count
      --tasks;
      // dipatch next task
      dispatch();
      break;
    case TIMER_EXPIRED:
        //kernel_update_ticker();

        /* Round robin tasks get pre-empted on every tick. */
        //if(current_process->level == RR && current_process->state == RUNNING) {
            current_process->state = READY;
            //enqueue(&rr_queue, cur_task);
        //}
  case EVENT_WAIT:
        /* idle_task does not wait. */
    //if(current_process != idle_task){
    //        kernel_event_wait();
    //}
    break;

  case EVENT_SIGNAL:
      //kernel_event_signal();
      break;

  case EVENT_SIGNAL_AND_NEXT:
      //if(cur_task->level == PERIODIC) {
      //    slot_task_finished = 1;
      //}

      //kernel_event_signal(0 /* not broadcast */, 1 /* is task_next */);

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
  for (int i = 0; i < 40; i++) {
    PORTB ^= (1<<PB7);
    uint16_t i;
    /* 4 * 100000 CPU cycles = 25 ms */
    for (int j = 0; i < 100; j++)
      asm volatile ("1: sbiw %0,1" "\n\tbrne 1b" : "=w" (i) : "0" (100000));
  }
  soft_reset();
}

PID Task_Create(void (*f)(void), PRIORITY py, int arg) {
  PID id;
  if (KernelActive) {
    uint8_t inturupt_flag = disable_global_interrupts();
    current_process->request = CREATE;
    current_process->code = f;
    //current_process->priority = py;
    //current_process->argument = arg;
    id = (current_process-Process) + 1;
    Enter_Kernel();
  } else {
    /* call the RTOS function directly */
    id = kernel_create_task(f);
    Process[id].priority = py;
    Process[id].argument = arg;
  }
  return id;
}

void Task_Terminate(void) {
  if (KernelActive) {
    uint8_t flag = disable_global_interrupts();
    current_process->request = TERMINATE;
    Enter_Kernel();
  }
}

void Task_Yield(void) {
  if (KernelActive) {
    uint8_t flag = disable_global_interrupts();
    current_process->request = NEXT;
    Enter_Kernel();
  }
}

/**
 * @brief Retrieve the assigned parameter.
 */
int Task_GetArg(void) {
  uint8_t sreg = disable_global_interrupts();
  int arg = current_process->argument;
  restore_global_interrupts(sreg);
  return arg;
}

void task_suspend(void) {
  uint8_t flag = disable_global_interrupts();
  Task_Suspend(current_process->id);
  restore_global_interrupts(flag);
}



void Task_Suspend(PID id) {
  uint8_t flag = disable_global_interrupts();
  volatile ProcessDescriptor * pd = &Process[id - 1];
  if (!pd->suspended) {
    pd->suspended = true;
    dequeue(pd);
    if (current_process->id == id) Task_Yield();
  }
  restore_global_interrupts(flag);
}

void task_resume(void) {
  uint8_t flag = disable_global_interrupts();
  Task_Resume(current_process->id);
  restore_global_interrupts(flag);
}

void Task_Resume(PID id) {
  uint8_t flag = disable_global_interrupts();
  volatile ProcessDescriptor * pd = &Process[id - 1];
  if (pd->suspended) {
    pd->suspended = false;
    //pd->state = READY;
    enqueue(pd);
  }
  restore_global_interrupts(flag);
}

void task_sleep(PID id, TICK t) {
  uint8_t flag = disable_global_interrupts();
  volatile ProcessDescriptor * pd = &Process[id - 1];
  volatile ProcessDescriptor * ptr;
  memset(ptr, 0, sizeof(ProcessDescriptor));
  pd->expires = ticks + t;
  if(sleep_queue==NULL) {
    dequeue(pd);
    sleep_queue = pd;
  } else {
    for (ptr->next=sleep_queue;true;ptr=ptr->next) {
      if (ptr->next==NULL) {
        dequeue(pd);
        ptr->next = pd;
        break;
      } else if (ptr->next->expires > pd->expires) {
        dequeue(pd);
        pd->next = ptr->next;
        ptr->next = pd;
        break;
      }
    }
  }
  restore_global_interrupts(flag);
}

void Task_Sleep(TICK t) {
  uint8_t flag = disable_global_interrupts();
  task_sleep(current_process->id, t);
  restore_global_interrupts(flag);
}


MUTEX kernel_init_mutex(MUTEX mid){
    if(num_mutexes == MAXMUTEX){
      // return 0; error: can't create more mutexes
    }

    //if at last spot in array, loop through to find inactive mutex
    //TODO: Keep this?
   if((mid == 7) || (Lock[mid+1].m_state != INACTIVE)){
      for(int i=0; i<MAXMUTEX; i++){
         if(Lock[i].m_state == INACTIVE){
            next_lock = &(Lock[i]);
         }
         break;
      }
    }else{
       next_lock = &(Lock[mid+1]);
    }


    current_lock = &(Lock[mid]);
    current_lock->mid = mid;
    current_lock->m_state = UNLOCKED;
    current_lock->lock_count = 0;
    current_lock->requests = NULL;

    ++num_mutexes;
    return mid;
}

static void kernel_mutex_lock(volatile MutexLock *mid, volatile ProcessDescriptor *rp){
     //TODO: Priority Inheritance

     if(current_lock->m_state == UNLOCKED){
        //Case where mutex is free to be locked
        current_lock->m_state = LOCKED;
        current_lock->owner = rp;
        ++current_lock->lock_count;

        //Save current priority for later
        rp->old_priority = rp->priority;
     }
     else if((current_lock->m_state == LOCKED) && (current_lock->owner == rp)){
        //Case where requesting process already owns mutex it is trying to lock
        ++current_lock->lock_count;
     }
     else{
       //Case where requesting process is trying to lock mutex owned by someone else
       //TODO:BLOCK_ON_MUTEX:new case or keep this as suspended?
       rp->suspended = true;
       listAddNodeTail(current_lock->requests, rp);
       //If the requesting task is of higher priority, switch priority of owner
       if((rp->priority) > (current_lock->owner->priority)){
          task_change_priority(current_lock->owner->id, rp->priority);
       }
       //resechedule and call the next process
       //schedule_task();
     }

}

static void kernel_mutex_unlock(volatile MutexLock *mid, volatile ProcessDescriptor *rp){
     //TODO: Priority Inheritance: go  back to original priority
     if(current_lock->owner != rp){
       //error: not owner! Abort
     }
     else if((current_lock->m_state == LOCKED) && (current_lock->lock_count > 1)){
       //Recursiveness: must unlock the mutex as many times as it has been locked by owner
       --current_lock->lock_count;
     }
     else if(current_lock->requests != NULL){
        //If owner releases mutex, give mutex to next task in Requests list
        //FCFS as per project specs


        current_lock->m_state = UNLOCKED;
        current_lock->lock_count = 0;
        current_lock->owner = current_process;
        kernel_mutex_lock(current_lock, current_process);

     }
     else{
       current_lock->m_state = UNLOCKED;
       current_lock->lock_count = 0;
       //Go back to old priority if it was changed
      /* if(rp->old_priority != ){
         task_change_priority(rp, rp->old priority);
         //schedule_task();
       }*/
     }
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
  uint8_t id;
  tasks = 0;
  KernelActive = 0;
  sleep_queue = NULL;
  for(int j = 0; j < MINPRIORITY; j++) {
    running_queue[j] = NULL;
  }
  // Reminder: Clear the memory for the task on creation.
  for (id = 1; id < MAXTHREAD; id++) {
    memset(&(Process[id - 1]), 0, sizeof(ProcessDescriptor));
    Process[id - 1].state = DEAD;
  }
}

/**
 * This function starts the RTOS after creating a few tasks.
 */
void OS_Start() {
  if ((!KernelActive) && (tasks > 0)) {
    disable_global_interrupts();
    KernelActive = true;
    next_kernel_request();
  }
}

ISR(TIMER1_COMPA_vect) {
  ++ticks;
  if (KernelActive) {
    if (sleep_queue != NULL && ticks >= sleep_queue->expires) {
      //Task_Resume(sleep_queue->id);
      sleep_queue = sleep_queue->next;
      //schedule_task(&(Process[sleep_queue->id - 1]));
    }
    Task_Yield();
  }
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
void idle (void) {
  for(;;) {

  }
}

// OS_Init function
void kernel_init(void) {
  DDRB = 0xFF;
  DDRL = 0xFF;

  // TODO ISSUE[QUEUE_FREEZE]
  //_delay_ms(500);

  OS_Init();

  // Add main to task queue to be defined later
  Task_Create(idle, 0, MINPRIORITY);
  Task_Create(main, 0, 0);
  init_timer();
  OS_Start();
}
