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
  }
}
void Pong() {

  for(;;) {
    PORTB &= ~(1<<PB7); // OFF
    signal_debug(0, true);
  }
}


void main () {
  Task_Create(Pong, 1, 0);
  Task_Create(Ping, 1, 0);
  for(;;){
    Task_Yield();
  }
}
