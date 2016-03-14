#include <string.h>
#include <stdbool.h>

#include <avr/io.h>
/*
#include <avr/interrupt.h>
#include <avr/delay.h>
*/
#include "os.h"
#include "common.h"

void Ping() {
  for(;;) {
    PORTB |= (1<<PB7); // OFF
    signal_debug(1, true);
    //return;

  }
}
void Pong() {

  for(;;) {
    PORTB &= ~(1<<PB7); // OFF
    signal_debug(0, true);
    //Task_Sleep(420);
    //Task_Suspend(1);
    //return;
    //Task_Terminate();
    //Task_Sleep(400);
    //Task_Suspend(2);
  }
}

void Pang() {

  for(;;) {
    signal_debug(2, true);
    //Task_Suspend()
    //return;
    //Task_Terminate();
    //Task_Sleep(10);
    //Task_Suspend(1);
  }
}


int main () {
  Task_Create(Ping, 0, 0);
  Task_Create(Pong, 0, 0);
  //Task_Create(Pang, 0, 0);
  //Task_Create(Ping, 4, 0);
  for(;;){

  }
}
