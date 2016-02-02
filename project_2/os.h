#ifndef _OS_H_  
#define _OS_H_  
   
#define MAXTHREAD     16       
#define WORKSPACE     256   // in bytes, per THREAD
#define MAXMUTEX      8 
#define MAXEVENT      8      
#define TICK          10    // resolution of system clock in milliseconds
#define MINPRIORITY   15   // 0 is the highest priority, 15 the lowest

#ifndef NULL
#define NULL          0   /* undefined */
#endif

typedef unsigned int PID;        // always non-zero if it is valid
typedef unsigned int MUTEX;      // always non-zero if it is valid
typedef unsigned char PRIORITY;
typedef unsigned int EVENT;      // always non-zero if it is valid

void OS_Init(void);
void OS_Abort(void);

PID  Task_Create( void (*f)(void), PRIORITY py, int arg);
void Task_Terminate(void);
void Task_Yield(void);
int  Task_GetArg(void);
void Task_Suspend( PID p );          
void Task_Resume( PID p );

void Task_Sleep(int t);

MUTEX Mutex_Init(void);
void Mutex_Lock(MUTEX m);
void Mutex_Unlock(MUTEX m);
int  Mutex_TryLock(MUTEX m);

EVENT Event_Init(void);
void Event_Wait(EVENT e);
void Event_Signal(EVENT e);

#endif /* _OS_H_ */
