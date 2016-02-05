/*
 * scheduler.cpp
 *
 *  Created on: 17-Feb-2011
 *      Author: nrqm
 *      Based on code by Jacob Schwartz
 */
#include "scheduler.h"
 
#include <Arduino.h>
#include <avr/interrupt.h>
 
typedef struct task_t {
	int32_t period;
	int32_t remaining_time;
	uint8_t is_running;
	task_cb callback;
  task_t * next;
} task_t;
 
task_t tasks[MAXTASKS];
 
uint32_t last_runtime;
 
void Scheduler_Init()
{
	last_runtime = millis();
}
 
uint8_t Scheduler_StartTask(int16_t delay, int16_t period, task_cb task) {
	static uint8_t id = 0;

  // Total number of tasks
  uint8_t taskCount = 0;
  static task_t * head;
  task_t task_tmp;
  
  if(head!=NULL) task_tmp.next = head;
  else head = &task_tmp;
  
  task_t * pointer = head;
  
  while (pointer.next!=NULL) {
    taskCount++;
    pointer = pointer.next;
  }
  
	if (id < MAXTASKS) {
		task_tmp.remaining_time = delay;
		task_tmp.period = period;
		task_tmp.is_running = 1;
		task_tmp.callback = task;
   return id;
	} 
	else return -1;
}

void Scheduler_RemoveTask(uint8_t s_id) {
  
}
/*
void Scheduler_RemoveTask(uint8_t s_id) {
  
}
 */
uint32_t Scheduler_Dispatch() {
	uint8_t i;
 
	uint32_t now = millis();
	uint32_t elapsed = now - last_runtime;
	last_runtime = now;
	task_cb t = NULL;
	uint32_t idle_time = 0xFFFFFFFF;
 
	// update each task's remaining time, and identify the first ready task (if there is one).
	for (i = 0; i < MAXTASKS; i++) {
		if (tasks[i].is_running)
		{
			// update the task's remaining time
			tasks[i].remaining_time -= elapsed;
			if (tasks[i].remaining_time <= 0)
			{
				if (t == NULL)
				{
					// if this task is ready to run, and we haven't already selected a task to run,
					// select this one.
					t = tasks[i].callback;
					tasks[i].remaining_time += tasks[i].period;
				}
				idle_time = 0;
			}
			else
			{
				idle_time = min((uint32_t)tasks[i].remaining_time, idle_time);
			}
		}
	}
	if (t != NULL)
	{
		// If a task was selected to run, call its function.
		t();
	}
	return idle_time;
}