
#include <Arduino.h>
#include "scheduler.h"
#include "util.h"

#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C/LiquidCrystal_I2C.h>

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

 
uint8_t pulse1_pin = 3;
uint8_t pulse2_pin = 4;
uint8_t idle_pin = 7;
uint8_t X_PIN = 1;
int servoVal = 0;
int theta = 0;

// Arduino pins
const int SW_pin = 2;  // digital pin connected to switch output
const int X_pin = 1;  // analog pin connected to X output
const int Y_pin = 2;  //analog pin connected to Y output
const int photoCellPin = 0;   //analog pin connected to the light sensor

int photoCellReading;  

int laserState = LOW;           //initally the laser should be OFF

int buf=0;
int reading;
int lastValue = 1500;
int lastLaserState = laserState;
long debounceTimer =0;
long test = 10; 

double mean(int array[]) {
  int i = 0;
  double avg = 0.0;
  double len = (sizeof(array)/sizeof(int));
  while (i++ < len) {
    avg += array[i];
  }
  return avg / len;
}


//periodic tasks - status of laser, , and detect when pressed but miss 
void fire_laser(){
  
  reading = digitalRead(SW_pin);      //periodically read input from button
  laserState = !reading;
}

void move_servo()
{
     
  servoVal = analogRead(X_pin); 
  if((laserState != lastLaserState) || (servoVal != lastValue)){
     buf = ((!reading << 7) + map(servoVal,0,1023,0,127));
    Serial1.write(buf);  
  }

  lcd.setCursor(15,0);
  lcd.print(" ");
  lcd.setCursor(12,0);
  lcd.print(servoVal);
  lastValue = servoVal;
  lastLaserState = reading;
  
}

void laser_state(){
  static byte i = 0;
  static int reading[20];
  int avg = (int)mean(reading[5]);

  
  
  photoCellReading = analogRead(photoCellPin);  //take in reading from the light sensor
  reading[i%20] = photoCellReading;
  if (photoCellReading > avg) {   //this value was chosen after reading the examples by AdaFruit, anything less than this is too dark to be the lazer
    targetHit();                  //add status for target missed 
  }
  
}

void missed_target(){
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
  lcd.setCursor(0,1);
  lcd.print("                ");
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
 
	// example idle function that just pulses a pin.
	digitalWrite(idle_pin, HIGH);
	delay(idle_period);
	digitalWrite(idle_pin, LOW);
}
 
void setup()
{
	pinMode(pulse1_pin, OUTPUT);
	pinMode(pulse2_pin, OUTPUT);
	pinMode(idle_pin, OUTPUT);

  // Setup Bluetooth
  Serial.begin(9600);
  Serial1.begin(9600);

  //LCD set up
  lcd.begin (16,2); 
 
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  lcd.print("Ready for battle");  
  delay(2000);
  lcd.clear();
  lcd.print("At position:");
  

  pinMode(SW_pin, INPUT);     //joystick button
  digitalWrite(SW_pin, HIGH);
  pinMode (13, OUTPUT);       //the laser
  digitalWrite(13, laserState);            //start with the laser off
  Serial1.begin(9600);

 
	Scheduler_Init();
 
	// Start task arguments are:
	//		start offset in ms, period in ms, function callback
 

  Scheduler_StartTask(0, 10, fire_laser);     //task for laser firing
  Scheduler_StartTask(0, 10, laser_state);    //detect if target hit or missed
  Scheduler_StartTask(0, 10, move_servo);     //task for servo movement 
  Scheduler_StartTask(0, 10, missed_target); 
  //Scheduler_StartTask(0, 2, clear_bottom_row); 
 
}
 
void loop()
{
	uint32_t idle_period = Scheduler_Dispatch();
	if (idle_period)
	{
		idle(idle_period);
	}
}
 

