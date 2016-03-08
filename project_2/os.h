#ifndef _OS_H_
#define _OS_H_

#define MAXTHREAD     16
#define WORKSPACE     256   // in bytes, per THREAD
#define MAXMUTEX      8
#define MAXEVENT      8
#define MSECPERTICK   10   // resolution of a system tick in milliseconds
#define MINPRIORITY   10   // 0 is the highest priority, 10 the lowest


#ifndef NULL
#define NULL          0   /* undefined */
#endif

typedef unsigned int PID;        // always non-zero if it is valid
typedef unsigned int MUTEX;      // always non-zero if it is valid
typedef unsigned char PRIORITY;
typedef unsigned int EVENT;      // always non-zero if it is valid
typedef unsigned int TICK;

// void OS_Init(void);      redefined as main()

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void OS_Abort(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
PID  Task_Create( void (*f)(void), PRIORITY py, int arg);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Task_Terminate(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Task_Yield(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
int  Task_GetArg(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Task_Suspend( PID p );

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Task_Resume (PID p);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Task_Sleep (TICK t);  // sleep time is at least t*MSECPERTICK

/**
 * @brief
 *
 *
 * @param
 * @return
 */
MUTEX Mutex_Init(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Mutex_Lock(MUTEX m);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Mutex_Unlock(MUTEX m);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
EVENT Event_Init(void);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Event_Wait(EVENT e);

/**
 * @brief
 *
 *
 * @param
 * @return
 */
void Event_Signal(EVENT e);

#endif /* _OS_H_ */
