#ifndef _KERNAL_H_
#define _KERNAL_H_

extern int main();

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
  TERMINATE
} KERNEL_REQUEST_TYPE;

typedef struct process_descriptor {
  unsigned char *stack_pointer;
  unsigned char workSpace[WORKSPACE];
  PROCESS_STATES state;
  voidfuncptr code;
  KERNEL_REQUEST_TYPE request;
  PRIORITY priority;
  PID id;
  int argument;
  LIST_ENTRY(process_descriptor) pointers;
} ProcessDescriptor;

/*******************************************************************************
 *
 ******************************************************************************/

//extern void CSwitch();


extern void Exit_Kernel();

extern void Enter_Kernel();

#endif /* _KERNAL_H_ */
