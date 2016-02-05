#include <Servo.h>
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

#define CENTER 1500
#define THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING 42

//Servo setup
Servo s;
int servoVal;
int theta;

// Arduino pins
// Digital pin connected to switch output
const int SW_pin = 2; 
// Analog pin connected to X output
const int X_pin = 1;  
//Analog pin connected to Y output
const int Y_pin = 2; 
//Analog pin connected to the light sensor
const int photoCellPin = 0;   

//Variable to store input from light sensor 
int photoCellReading;  

//Variable for Debouncing
//Start with the button reading as HIGH
int lastButtonState = HIGH;     
int currentButtonState;
//Initally the laser should be OFF
int laserState = LOW;       
//The last time the button was pressed   
long lastDebounceTime = 0;  
//The debounce time
long debounceDelay = 50;    

void setup() {

  //Servo Setup
  s.attach(9);
  s.writeMicroseconds(CENTER);
  delay(15);

  //LCD set up
  lcd.begin (16,2); 
 
  // Switch on the backlight
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH);
  lcd.home ();
  lcd.print("Ready for battle");  
  delay(5000);

  //Joystick button
  pinMode(SW_pin, INPUT);     
  digitalWrite(SW_pin, HIGH);
  //The laser
  pinMode (13, OUTPUT); 
        
  Serial.begin(9600);

  
}
 
void loop() {

  //Start with the laser off
  digitalWrite(13, laserState);            

  //Read input from button
  int reading = digitalRead(SW_pin);      

   //Reset the debouncing timer if the button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = micros();
  }

 /* Whatever the reading is at, it's been there for longer
  than the debounce delay, so take it as the actual current state:
 */
  if ((micros() - lastDebounceTime) > debounceDelay) {

    // If the button state has changed, change variable
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Only toggle the laser if the new button state is LOW
      if (currentButtonState == LOW) {
        laserState = !laserState;
      }
    }
  }
    
  //Toggle laser
  digitalWrite(13, laserState);
  
  //reading is saved so that it will be the lastButtonState on the next iteration
  lastButtonState = reading;

  //Take in reading from the light sensor
  photoCellReading = analogRead(photoCellPin); 

  /*This value was chosen after reading the examples by AdaFruit, 
   * anything less than this is too dark to be the laser
   */
  if (photoCellReading > 800) {   
    targetHit();                 
  }

 
  
   servoVal = analogRead(X_pin); 
   //Print position on X-axis
   lcd.setCursor(0,0);
   lcd.print("At position: ");
   lcd.print(servoVal);
  
  //Maps the joystick inputs to the range that's safe for the servo motors 
   theta = map(servoVal, 0, 1023, 1000, 2000);       
   s.writeMicroseconds(theta);
   delay(THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING); 


}

//the seonsor has detected the laser's light
void targetHit(){
  lcd.setCursor(0,1);
  lcd.print("Target is hit");
  lcd.setBacklight(LOW);      // Backlight off
  delay(100);
  lcd.setBacklight(HIGH);     // Backlight on
  delay(100);
  lcd.clear();
}

