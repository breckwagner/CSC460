
#include <Arduino.h>
#include "scheduler.h"
#include "util.h"

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

//I2C set up
#define I2C_ADDR    0x3F 
#define BACKLIGHT_PIN     3
#define En_pin  2
#define Rw_pin  1
#define Rs_pin  0
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

uint8_t X_PIN = 1;
int servoVal = 0;

// Arduino pins
const int SW_pin = 2;  // digital pin connected to switch output
const int X_pin = 1;  // analog pin connected to X output
const int Y_pin = 2;  //analog pin connected to Y output
const int photoCellPin = 0;   //analog pin connected to the light sensor

int photoCellReading;  

byte laserState = false;           //initally the laser should be OFF


int lastValue = 0;
int lastLaserState = laserState;



/**
 * Sends a Byte of data with the the PS2 stae and push button state
 */
void sendState() {
  byte encode;
  laserState = !digitalRead(SW_pin);
  servoVal = analogRead(X_pin); 
  //if((laserState != lastLaserState) || (servoVal != lastValue)){
     encode = ((laserState << 7) + map(servoVal,0,1023,0,127));
    Serial1.write(encode);  
  //}

  lcd.setCursor(15,0);
  lcd.print(" ");
  lcd.setCursor(12,0);
  lcd.print(servoVal);
  lastValue = servoVal;
  lastLaserState = laserState;
}


/**
 * 
 */
void isHit(){
  if (analogRead(photoCellPin) > 600) {   //this value was chosen after reading the examples by AdaFruit, anything less than this is too dark to be the lazer
    targetHit();                  //add status for target missed 
  }
 /* static byte i = 0;
  static int reading[20];
  int avg = (int)mean(reading[5]);

  
  
  photoCellReading = analogRead(photoCellPin);  //take in reading from the light sensor
  reading[i%20] = photoCellReading;
  if (photoCellReading > avg) {   //this value was chosen after reading the examples by AdaFruit, anything less than this is too dark to be the lazer
    targetHit();                  //add status for target missed 
  }
  */
}

void missed_target() {
  if ((laserState == HIGH) && (photoCellReading < 800) ){
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print("Missed Target");  
   
  }
}

void clear_bottom_row(){
  lcd.setCursor(0,1);
  lcd.print("                ");
}

//the seonsor has detected the laser's light
void targetHit(){
  clear_bottom_row();
  lcd.setCursor(0,1);
  lcd.print("Target is hit");
  lcd.setBacklight(LOW);      // Backlight off
  delay(100);
  lcd.setBacklight(HIGH);     // Backlight on
  delay(100);
}


// idle task
void idle(uint32_t idle_period)
{
	// this function can perform some low-priority task while the scheduler has nothing to run.
	// It should return before the idle period (measured in ms) has expired.  For example, it
	// could sleep or respond to I/O.
}
 
void setup() {
  // Setup Bluetooth
  Serial.begin(9600);
  Serial1.begin(9600);

  //LCD set up
  lcd.begin (16,2); 
 
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home();
  lcd.clear();
  lcd.print("x:");
  

  pinMode(SW_pin, INPUT);     //joystick button
  Serial1.begin(9600);

 
	Scheduler_Init();
 // Scheduler_StartTask(0, 10, isHit);
  Scheduler_StartTask(0, 10, sendState);
 // Scheduler_StartTask(0, 10, missed_target);

  //attachInterrupt(digitalPinToInterrupt(SW_pin), fire_laser, CHANGE);
  //Scheduler_StartTask(0, 2, clear_bottom_row); 
 
}
 
void loop() {                                                                              
	uint32_t idle_period = Scheduler_Dispatch();
	if (idle_period)
	{
		idle(idle_period);
	}
}
 

