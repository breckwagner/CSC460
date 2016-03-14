/*
Instructions
Please read the schematics of DFRobot LCD shield on how the buttons are connected to the Arduino board. They are wired into the A/D port. This exercise is about understanding the A/D feature of AVR family of microcontrollers.

Use the DFRobot LCD module to test different sampling frequency (from 2Hz to as fast as possible) on the keypad inputs. Display your key inputs, number of registered inputs, and sampling frequency on the LCD. What is the lowest sampling frequency that still guarantees "good" results, i.e., minimum errors. (Note: If you don't get any errors, then something in the code is "filtering" the bouncing behaviour for you.)

An error is defined by either a "missing" or "erroneous" key input. Try counting number of key presses yourself in your head and observe what is being registered by your software. Do as fast as you can and see whether error rates increase.

This A/D interface to key inputs is similar to your PS/2 joystick. Read the actual library code and make sure that you understand how the A/D interface on the ATMEL chip works.

Submit your working code, screen-captured of the LCD and your findings in the submission box.
*/
//Sample using LiquidCrystal library
#include <LiquidCrystal.h>

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

long COUNT = 0;
int DELAY_Hz = 1;


/**
 * @paream frequancy in Hz
 * @return the period in ms
 */
int period (int frequancy) {
    return 1000/(frequancy);
}

// read the buttons
int read_LCD_buttons() {
    adc_key_in = analogRead(0);      // read the value from the sensor 
    // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
    // we add approx 50 to those values and check to see if we are close
    if (adc_key_in > 1000) return btnNONE;
    else {
        COUNT++;
        return btnSELECT;
    }
    
}

void setup() {
    lcd.begin(16, 2);              // start the library
    lcd.setCursor(0,0);
    lcd.print("Count:"); // print a simple message
}
 
void loop() {
    lcd.setCursor(6,0);
    lcd.print(COUNT);
    
    lcd.setCursor(0,1);
    lcd.print(millis()/1000);
    lcd.print("s");

    lcd.print(" freq: ");

    lcd.print(DELAY_Hz);

    lcd.print("Hz");
    
    lcd_key = read_LCD_buttons();
    delay(period(DELAY_Hz));
}
