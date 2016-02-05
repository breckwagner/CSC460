/***********************************************************************
 * main_attacker.ino
 * 
 * @author Richard B. Wagner
 * @author Navdeep Bahia
 **********************************************************************/


#include <Arduino.h>
#include <Servo.h>

#include "scheduler.h"
#include "util.h"

#define DEBUG true

// pin number on the Arduino ATMEGA2560 used for the laser
uint8_t laserPin = 23;

// pin number on the Arduino ATMEGA2560 used for the servo
uint8_t servoPin = 22;

// State of the laser true = 'on', false = 'off'
bool laserState = false;

// The servo "object" that represents the state of the 'x' axis servo
Servo servo_x;

// The value of the servo with range [1000, 2000]
uint16_t servoValue = SG91R_SERVO_CENTER;

int8_t servoVelocity = 0;

uint8_t previous_encoded_byte = 0;

//int8_t servoAcceleration = 0;


/** 
 * TTA Schedualer TASK
 * pole_serial is a Task for the TTA that checks the serial buffer is available
 * @return (void)
 */
void pole_serial () {
  #ifdef DEBUG
    digitalWrite(2, HIGH);
  #endif
  
  if(Serial1.available()) {
    // A byte used to read from the serial buffer highest order bit encodes 
    // laser and the next 7 bits encode the servo acceleration
    uint8_t encoded_byte = Serial1.read();

    // Bitmask 10000000 over the recieved byte
    laserState = (encoded_byte >> 7) & 0x01;

    // Bitmask 01111111 over the recieved byte
    //if ((encoded_byte & 0x7F) > 71 || (encoded_byte & 0x7F) < 55)
    if( (encoded_byte & 0x7F) != previous_encoded_byte)
      servoValue += map((encoded_byte & 0x7F), 0, 127, - SG91R_MAX_ACCELERATION, SG91R_MAX_ACCELERATION);
    else if ((previous_encoded_byte & 0x7F) > 71 || (previous_encoded_byte & 0x7F) < 55)
      servoValue += map((previous_encoded_byte & 0x7F), 0, 127, - SG91R_MAX_ACCELERATION, SG91R_MAX_ACCELERATION);
    //servoVelocity += map((encoded_byte & 0x7F), 0, 127, - SG91R_MAX_ACCELERATION, SG91R_MAX_ACCELERATION);
    //if (abs(servoVelocity) > SG91R_MAX_SPEED) {
    //  servoVelocity = ((servoVelocity < 0)?-SG91R_MAX_SPEED:SG91R_MAX_SPEED);
    //}
    
    
    


  }
  update_actuator_states();

  previous_encoded_byte = servoValue;
  
  #ifdef DEBUG
    digitalWrite(2, LOW);
  #endif
}

/** 
 * TTA Schedualer TASK
 * update_actuator_states updates the state of the laser and servo and 
 * asserts correct values
 * @return (void)
 */
void update_actuator_states () {
    //servoValue += servoVelocity;
    
    if (servoValue < SG91R_SERVO_LEFT) servoValue = SG91R_SERVO_LEFT;
    else if (servoValue > SG91R_SERVO_RIGHT) servoValue = SG91R_SERVO_RIGHT;
    
    servo_x.writeMicroseconds(servoValue);
    digitalWrite(laserPin, laserState);
}
 
/** 
 * TTA Schedualer TASK
 * idle is a Task for the TTA that executes when the TTA is idle
 */
void idle(uint32_t idle_period) {
  
}

/**
 * @param period in ms
 * @return frequancy in Hz
 * 
 * Note: The number will be truncated not rounded because of the division
 */
long Period_MSToFrequancy_HZ(long period) {
  return 1000/period;
}

/** 
 * Arduino Setup
 */
void setup() {
  // Setup Servo
  servo_x.attach(servoPin);
  servo_x.writeMicroseconds(servoValue);
  delay(15);
  #ifdef DEBUG
  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
  #endif

  // Setup Laser
  pinMode(laserPin, OUTPUT);
  digitalWrite(laserPin, LOW);

  // Setup Bluetooth
  // Serial.begin(9600);
  Serial1.begin(9600);
  
  // Setup Scheduler
  Scheduler_Init();
  Scheduler_StartTask(0, 50, pole_serial);
}

/** 
 * Arduino Loop
 */
void loop() {
  uint32_t idle_period = Scheduler_Dispatch();
  if (idle_period) idle(idle_period);
}
 

