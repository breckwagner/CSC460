/*
 * main.cpp
 *
 *  Created on: 18-Feb-2011
 *      Author: nrqm
 *
 * Example program for time-triggered scheduling on Arduino.
 *
 * This program pulses one pin every 500 ms and another pin every 300 ms
 *
 * There are two important parts in this file:
 * 		- at the end of the setup function I call Scheduler_Init and Scheduler_StartTask.  The latter is called once for
 * 		  each task to be started (note that technically they're not really tasks because they share the stack and don't
 * 		  have separate contexts, but I call them tasks anyway because whatever).
 * 		- in the loop function, I call the scheduler dispatch function, which checks to see if a task needs to run.  If
 * 		  there is a task to run, then it its callback function defined in the StartTask function is called.  Otherwise,
 * 		  the dispatch function returns the amount of time (in ms) before the next task will need to run.  The idle task
 * 		  can then do something useful during that idle period (in this case, it just hangs).
 *
 * To use the scheduler, you just need to define some task functions and call Scheduler_StartTask in the setup routine
 * for each of them.  Keep the loop function below the same.  The scheduler will call the tasks you've created, in
 * accordance with the creation parameters.
 *
 * See scheduler.h for more documentation on the scheduler functions.  It's worth your time to look over scheduler.cpp
 * too, it's not difficult to understand what's going on here.
 */
 
#include <Arduino.h>
#include <Servo.h>
#include "scheduler.h"
#include "util.h"
 
uint8_t laser_pin = 23;
uint8_t servo_pin = 22;
bool laser_state = false;
char serial_buffer;






Servo servo_x;
int servoVal;
int theta;

int photoCellReading;         

void pole_serial () {
    if(Serial1.available()) {
        serial_buffer = Serial1.read();
        if(serial_buffer == 0x30){ // This is '0'
          digitalWrite(laser_pin, LOW);
          Serial1.println("LASER_LOW");
        } else if (serial_buffer == 0x31) {
          digitalWrite(laser_pin, HIGH);
          Serial1.println("LASER_HIGH");
        } else if (serial_buffer == 's') {
          if(Serial1.available()) 
            theta = Serial1.parseInt();
            //theta = int(0x03 & serial_buffer);
            //serial_buffer = Serial.read();
            //theta = theta + (serial_buffer << 2);

            
            Serial1.println(theta);
          
        }

        
    }
}
 
// idle task
void idle(uint32_t idle_period) {}
 
void setup() {
    // Setup Servo
    servo_x.attach(servo_pin);
    servo_x.writeMicroseconds(SERVO_CENTER);
    delay(15);
    
    // Setup Laser
    pinMode(laser_pin, OUTPUT);
    digitalWrite(laser_pin, LOW);
    
    // Setup Bluetooth
    Serial.begin(9600);
    Serial1.begin(9600);
    
    // Setup Scheduler
    Scheduler_Init();
    Scheduler_StartTask(0, 10, pole_serial);
}
 
void loop() {
    uint32_t idle_period = Scheduler_Dispatch();
    if (idle_period) {
    	  idle(idle_period);
    }
}
 

