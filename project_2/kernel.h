#ifndef _KERNAL_H_
#define _KERNAL_H_

#include <stdbool.h>

// Needs to be here to promise void kernal_init(void) that main exists for
// Task_Creates call
extern void main();

typedef void (*voidfuncptr) (void);      /* pointer to void f(void) */

typedef enum process_states {
  DEAD = 0,
  READY,
  RUNNING,
  SUSPENDED
} PROCESS_STATES;

typedef enum kernel_request_type {
  NONE = 0,
  CREATE,
  NEXT,
  TERMINATE,
  TIMER_EXPIRED,
  EVENT_WAIT,
  EVENT_SIGNAL,
  EVENT_SIGNAL_AND_NEXT
} KERNEL_REQUEST_TYPE;

typedef struct process_descriptor {
  volatile unsigned char *stack_pointer;
  unsigned char workSpace[WORKSPACE];
  PROCESS_STATES state;
  voidfuncptr code;
  KERNEL_REQUEST_TYPE request;
  PRIORITY priority;
  PRIORITY old_priority;
  PID id;
  int argument;
  uint32_t expires;
  bool suspended;
  volatile struct process_descriptor * next;
  volatile struct process_descriptor * prev;
} ProcessDescriptor;

// TODO: We MIGHT use this (sleeping_process) but unused for now
typedef struct sleeping_process {
  ProcessDescriptor * pd;

  // Absolute time for the process to wake since system start (ticks)
  uint32_t expires;
} SleepingProcess;

static const struct ProcessDescriptor EmptyProcessDescriptor;

typedef enum mutex_states{
  UNLOCKED = 0,
  LOCKED = 1,
  INIT,
  INACTIVE
} MUTEX_STATES;

//Use ProcessDescriptor to track owner/requesting process
typedef struct mutex_lock{
  MUTEX mid;
  MUTEX_STATES m_state;
  int lock_count;
  ProcessDescriptor *owner;
  //requests will be cast to process descriptors
  ProcessDescriptor *requests;
} MutexLock;

/*******************************************************************************
 *
 ******************************************************************************/

//extern void CSwitch();


extern void Exit_Kernel();

extern void Enter_Kernel();

#endif /* _KERNAL_H_ */
