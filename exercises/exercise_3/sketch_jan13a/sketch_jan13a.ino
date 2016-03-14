/*
On every Arduino board, pin 13 is connected to a built-in LED on the board.

On Arduino Mega, this same pin is also connected to one of the PWM timer pins.

Implement a simple C program which will fade this LED from minimum brightness (OFF) to maximum brightness (ON) using the PWM timer. The PWM timer is controlled by the analogWrite() function in Arduino.

The fading period (min to max to min) should be close to our "breathing" period, similar to when a laptop sleeps, it fades a small LED at the rate of human breathing.

Use a logic analyzer to visualize the PWM timer output of the LED.

Submit your code and a screen capture of your USB logic analyzer. Please explain your observation from the screen captured about your code.
*/

/*
 Fade

 This example shows how to fade an LED on pin 9
 using the analogWrite() function.

 This example code is in the public domain.
 */
#include "Waveforms.h"

int led = 13;           // the pin that the LED is attached to
int count = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

// the setup routine runs once when you press reset:
void setup() {
  // declare pin 9 to be an output:
  Serial.begin(9600);
  pinMode(led, OUTPUT);
}

void loop() {
  analogWrite(led, waveformsTable[0][count++%120]/16);
  delay(30);
}

