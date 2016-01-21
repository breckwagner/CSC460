/*******************************************************************************
 *
 *
 ******************************************************************************/

#include <Servo.h>
#include "Waveforms.h"

#define SIN_WAVEFORM 0
#define CENTER 1500
#define SCALING_FACTOR 4.096
#define LOWER_BOUND 1000
#define THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING 42

Servo s;

int theta = 0;
int counter = 0;

void setup() {
  s.attach(9);
  s.writeMicroseconds(CENTER);
  delay(15);

  Serial.begin(9800);
}

void loop() {
  theta = (int) waveformsTable[SIN_WAVEFORM][counter++ % maxSamplesNum] / SCALING_FACTOR + LOWER_BOUND;
  s.writeMicroseconds(theta);
  delay(THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING);
  Serial.println(theta);
}

