
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

#define DEBUG true

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

int ambiantLightConstant;



/**
 * Sends a Byte of data with the the PS2 stae and push button state
 */
void sendState() {
  #ifdef DEBUG
    digitalWrite(3, HIGH);
  #endif
  
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
  
  #ifdef DEBUG
    digitalWrite(3, LOW);
  #endif
}


/**
 * 
 */
void whileHit(){
  #ifdef DEBUG
    digitalWrite(4, HIGH);
  #endif
  static bool last_hit_state = false;
  static bool last_laser_state = false;
  bool hit = isHit();
  
  if (hit) {
    targetHit(hit, last_hit_state);
  }
  else if (!hit && last_hit_state) {
    lcd.setBacklight(HIGH);
    clear_bottom_row();
  } else if (laserState && !last_laser_state) {
      printLaserState();
  } else if (!laserState && last_laser_state) {
      clear_bottom_row();
  }

  last_laser_state = laserState;
  last_hit_state = hit;

  #ifdef DEBUG
    digitalWrite(4, LOW);
  #endif
}

bool isHit() {
  
  return (analogRead(photoCellPin) > ambiantLightConstant);
}

void printLaserState() {
  //if ((laserState) && (!tmp_hit) ){
    clear_bottom_row();
    lcd.setCursor(0,1);
    lcd.print("Missed Target");  
  //} 
}

void clear_bottom_row(){
  lcd.setCursor(0,1);
  lcd.print("                ");
}

//the seonsor has detected the laser's light
void targetHit(bool hit_state, bool last_hit_state){

  static bool toggle = false;
  static long time_delay = millis();
  
  if (!last_hit_state && hit_state) {
  //if (tmp_hit){
    clear_bottom_row();
    lcd.setCursor(0,1);
    lcd.print("HIT");
  }
  
  if (millis() - time_delay > 100) {
    if (toggle=!toggle) {
      lcd.setBacklight(LOW);
    } else {
      lcd.setBacklight(HIGH);
    }
    time_delay = millis();
  }
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
  

  pinMode(SW_pin, INPUT_PULLUP);     //joystick button
  
  #ifdef DEBUG
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  #endif
  

  ambiantLightConstant = analogRead(photoCellPin) + 20;
  
  Serial1.begin(9600);

 
	Scheduler_Init();
  Scheduler_StartTask(0, 10, whileHit);
  Scheduler_StartTask(0, 10, sendState);
 
}
 
void loop() {                                                                              
	uint32_t idle_period = Scheduler_Dispatch();
	if (idle_period)
	{
		idle(idle_period);
	}
}
 

