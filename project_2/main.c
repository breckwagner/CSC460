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
  bool x = true;
  for(;;) {
    PORTB |= (1<<PB7); // OFF
    signal_debug(0, true);
    //Task_Resume(2);
    //return;
    if(x){
      //Task_Suspend(2);
      //x = false;
    }

  }
}
void Pong() {
  uint32_t i = 0;
  for(;;) {

    PORTB &= ~(1<<PB7); // OFF
    signal_debug(1, true);
    //Task_Sleep(30);
    //Task_Suspend(1);
    //return;
    //Task_Terminate();
    //Task_Sleep(100);
    //Task_Resume(2);
    //return;
  }
}


int main () {
  Task_Create(Ping, 0, 0);
  Task_Create(Pong, 0, 0);
}
