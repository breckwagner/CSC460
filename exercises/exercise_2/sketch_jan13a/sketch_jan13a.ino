
/*******************************************************************************
 * This is a continuation of Exercise 1. We would like you to use the DFRobot 
 * LCD shield to compare the I/O speed (or latency) for displaying 32 
 * characters (2 lines by 16 chars) on the LCD vs on the Arduino console 
 * (UART).

 * Measure the total delay (latency) of displaying 32 characters. Do it 
 * multiple times and measure the latency using your USB logic analyzer. Get 
 * the measurements as accurate as possible.

 * The documentation for the LCD shield is available through Project 1 or 
 * under the "Resources/Driver" folder.

 * Submit your source code, measurements and a picture of your LCD display and 
 * Console.
 */
 
//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

/*******************************************************

This program will test the LCD panel and the buttons
Mark Bramwell, July 2010

********************************************************/

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);



// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

void printToDisplay() {
    lcd.setCursor(0,0);
    lcd.print("0123456789012345");
    lcd.setCursor(0,1);
    lcd.print("0123456789012345");
}

void printToDisplay_2(int i) {
    lcd.setCursor(0,0);
    
    lcd.print("0123456789012345");
    lcd.setCursor(0,1);
    lcd.print("0123456789012345");
}

void setup() {
    // initialize digital pin 13 as an output.
    pinMode(13, OUTPUT);
    Serial.begin(9600);
    
    lcd.begin(16, 2);

    lcd.setCursor(0,0);
    lcd.print("3");
    delay(1000);
    lcd.setCursor(0,0);
    lcd.print("2");
    delay(1000);
    lcd.setCursor(0,0);
    lcd.print("1");
    delay(1000);
    lcd.setCursor(0,0);
    lcd.print("0");
    delay(500);

    Serial.println("01234567890123450123456789012345");
    
    printToDisplay();
    int i = 0;

    digitalWrite(13, HIGH);
    while(i++ < 100) {
        printToDisplay();
    }
    digitalWrite(13, LOW);
}
 
void loop() {
  
}

