#include <Servo.h>



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


void setup() {

  s.attach(9);
  s.writeMicroseconds(CENTER);
  delay(15);
  
  pinMode(SW_pin, INPUT);
  digitalWrite(SW_pin, HIGH);
  pinMode (13, OUTPUT); 
  Serial.begin(9600);
}
 
void loop() {
  
  digitalWrite(13, LOW);            //start with the lazer off
    
  if(digitalRead(SW_pin)== LOW){    //if the button is pressed, turn the lazer on - note: needs debouncing, TA suggests averaging
      digitalWrite(13, HIGH);    
  }

  photoCellReading = analogRead(photoCellPin);  //take in reading from the light sensor
   
  if (photoCellReading > 800) {   //this value was chosen after reading the examples by AdaFruit, anything less than this is too dark to be the lazer
    targetHit();                  //add status for dark, dim, hit?
  }

  Serial.print("\n");
  
  servoVal = analogRead(X_pin); 
  theta = map(servoVal, 0, 1023, 1000, 2000);       //maps the joystick inputs to the range that's safe for the servo motors - note: this may need to be adjusted
  s.writeMicroseconds(theta);
  delay(THE_ANSWER_TO_THE_ULTIMATE_QUESTION_OF_LIFE_THE_UNIVERSE_AND_EVERYTHING);   //I understood that reference! Though this delay could probably be removed if we ignore the y-axis


  /*do we realy need to read the y-azis input? It only goes 180 degrees
  Serial.print("\n");
  Serial.print("Y-axis: ");  
  Serial.println(analogRead(Y_pin));
  Serial.print("\n\n");
  */
  delay(500);
}

//the seonsor has detected the lazer's light
void targetHit(){
  Serial.println("Target is hit");
}

