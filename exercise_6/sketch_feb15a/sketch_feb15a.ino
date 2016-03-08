
#include "Arduino.h"
#include <stdio.h>
#include <LiquidCrystal.h>

#define TICKS_PER_SECOND 100ul
#define ONE_SECOND (TICKS_PER_SECOND)
#define ONE_MINUTE (60ul * ONE_SECOND)
#define ONE_HOUR (60ul * ONE_MINUTE)

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

volatile unsigned long elapsed = 0;

int main() {

  init();
  lcd.begin(16, 2);
  DDRA = (1<<PA0);
  DDRA = (1<<PA1);
  
  //Clear timer config.
  TCCR3A = 0;
  TCCR3B = 0;  
  //Set to CTC (mode 4)
  TCCR3B |= (1<<WGM32);
  
  //Set prescaler to 256
  //TCCR3B= ((1 << CS31) | (1 << CS30));
  TCCR3B |= (1<<CS32);
  
  //Set TOP value (0.01 seconds)
  OCR3A = 625;
  //OCR3A = TICKS_PER_SECOND;
  
  //Enable interupt A for timer 3.
  TIMSK3 |= (1<<OCIE3A);
  
  //Set timer to 0 (optional here).
  TCNT3 = 0;
  
  //Enable interupts
  sei();
  
  while(true){
    print_time(elapsed);
  };
}

void print_time(unsigned long t) {
  char tmp[17] = { 0 };
  sprintf(tmp,"%02ld:%02ld:%02ld:%02ld", ((t/ONE_HOUR)%60), ((t/ONE_MINUTE)%60), ((t/ONE_SECOND)%60),((long)(t))%100);
  
  lcd.home();
  lcd.print(tmp);
}

ISR(TIMER3_COMPA_vect) {
  PORTA |= (1<<PA0);
  elapsed++;
  PORTA &= ~(1<<PA0);
  //Slowly incrase the duty cycle of the LED.
  // Could change OCR1A to increase/decrease the frequency also.
  
}
