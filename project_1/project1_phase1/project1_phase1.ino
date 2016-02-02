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

Servo s;
int servoVal;
int theta;

// Arduino pins
const int SW_pin = 2;  // digital pin connected to switch output
const int X_pin = 1;  // analog pin connected to X output
const int Y_pin = 2;  //analog pin connected to Y output
const int photoCellPin = 0;   //analog pin connected to the light sensor

int photoCellReading;  

int lastButtonState = HIGH;     //start with the button reading as HIGH
int currentButtonState;
int laserState = LOW;           //initally the laser should be OFF
long lastDebounceTime = 0;  // the last time the button was pressed
long debounceDelay = 50;    // the debounce time; increase if the output flickers

void setup() {

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
  
  pinMode(SW_pin, INPUT);     //joystick button
  digitalWrite(SW_pin, HIGH);
  pinMode (13, OUTPUT);       //the laser
  Serial.begin(9600);

  
}
 
void loop() {
  
  digitalWrite(13, laserState);            //start with the laser off

  //debouncing
  int reading = digitalRead(SW_pin);      //read input from button

   // reset the debouncing timer if the button state has changed
  if (reading != lastButtonState) {
    lastDebounceTime = micros();
  }

 // whatever the reading is at, it's been there for longer
 // than the debounce delay, so take it as the actual current state:
  if ((micros() - lastDebounceTime) > debounceDelay) {

    // if the button state has changed, change variable
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // only toggle the laser if the new button state is LOW
      if (currentButtonState == LOW) {
        laserState = !laserState;
      }
    }
  }
    
  //toggle laser
  digitalWrite(13, laserState);
  
  //reading is saved so that it will be the lastButtonState on the next iteration
  lastButtonState = reading;
  
  photoCellReading = analogRead(photoCellPin);  //take in reading from the light sensor
   
  if (photoCellReading > 800) {   //this value was chosen after reading the examples by AdaFruit, anything less than this is too dark to be the lazer
    targetHit();                  //add status for dark, dim, hit?
  }

 
  
   servoVal = analogRead(X_pin); 
   lcd.setCursor(0,0);
   lcd.print("At position: ");
   lcd.print(servoVal);
  //maps the joystick inputs to the range that's safe for the servo motors - note: this may need to be adjusted
  theta = map(servoVal, 0, 1023, 1000, 2000);       
  s.writeMicroseconds(theta);
  delay(THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING); 

  /*do we realy need to read the y-azis input? It only goes 180 degrees
  Serial.print("\n");
  Serial.print("Y-axis: ");  
  Serial.println(analogRead(Y_pin));
  Serial.print("\n\n");
  */
  //delay(500);
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

