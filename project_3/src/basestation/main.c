/*
 * control.c
 *
 *  Created on: 14-July-2010
 *      Author: lienh
 */
//#include "radio/radio.h"
#include "../lib/roomba-lib/roomba.h"
#include "../lib/avr-uart/uart.h"
#include "../lib/rtos/os.h"
#include "../lib/shared.h"

#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

int lcd_key     = 0;
int adc_key_in  = 0;
uint8_t send_flag = true;
uint8_t prev_send_flag = true;


void adc_init(void);

uint16_t read_adc(uint8_t channel);


int read_lcd_buttons() {
 adc_key_in = read_adc(0);      // read the value from the sensor
 // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
 // we add approx 50 to those values and check to see if we are close
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 /*
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;
*/
 // For V1.0 comment the other threshold and use the one below:

 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 195)  return btnUP;
 if (adc_key_in < 380)  return btnDOWN;
 if (adc_key_in < 555)  return btnLEFT;
 if (adc_key_in < 790)  return btnSELECT;
 return btnNONE;  // when all others fail, return this...
}


void adc_init(void){

	//16MHz/128 = 125kHz the ADC reference clock

	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

	ADMUX |= (1<<REFS0);       //Set Voltage reference to Avcc (5v)

	ADCSRA |= (1<<ADEN);       //Turn on ADC

	ADCSRA |= (1<<ADSC);
}     //Do an initial conversion


//map input values to those appropriate for Roomba
int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max){
	return ((x - in_min) * (out_max - out_min)) / ((in_max - in_min) + out_min);
}

uint16_t read_adc(uint8_t channel){
	ADMUX &= 0xE0;           //Clear bits MUX0-4
	ADMUX |= channel&0x07;   //Defines the new ADC channel to be read by setting bits MUX0-2
	ADCSRB = channel&(1<<3); //Set MUX5
	ADCSRA |= (1<<ADSC);      //Starts a new conversion
	while(ADCSRA & (1<<ADSC));  //Wait until the conversion is done
	return ADCW;
}         //Returns the ADC value of the chosen channel


void button_pole (){
	lcd_key = read_lcd_buttons();  // read the buttons

switch (lcd_key)               // depending on which button was pushed, we perform an action
{
	case btnRIGHT:
		{
		//lcd.print("RIGHT ");
		break;
		}
	case btnLEFT:
		{
		//lcd.print("LEFT   ");
		break;
		}
	case btnUP:
		{
		//lcd.print("UP    ");
		break;
		}
	case btnDOWN:
		{
		//lcd.print("DOWN  ");
		break;
		}
	case btnSELECT:
		{
		//lcd.print("SELECT");
		break;
		}
		case btnNONE:
		{
		//lcd.print("NONE  ");
		break;
		}
}
}

//TODO: Constrain values in mapped ranges?
void pole_sensors(){
	uart1_init(UART_BAUD_SELECT(9600, F_CPU));
	uart0_init(UART_BAUD_SELECT(9600, F_CPU));
	//char str[32];
	char test[16];
	uint8_t last_val = 1;
	for(;;){
		uint16_t x = read_adc(14);
		int16_t radius = x*3.9101-2000;
		//map(x, 0, 1023, -2000, 2000);
		if(radius>0) radius = (2000 - abs(radius));
		else radius = -(2000 - abs(radius));
		if(abs(radius) > 1000) radius = ROOMBA_ANGLE_STRAIGHT;
		else {
			radius = (radius==abs(radius))?1:-1;
		}
		/*if(abs(radius) < 100){
			if (radius > 0) radius = ROOMBA_ANGLE_CLOCKWISE;
			else radius = ROOMBA_ANGLE_COUNTER_CLOCKWISE;
      velocity = 500;
		}*/

		uint16_t y = read_adc(15);
		int16_t velocity = -(y*0.97752-500);
		if(abs(velocity)<25) velocity = 0;
    if(radius==ROOMBA_ANGLE_CLOCKWISE||radius==ROOMBA_ANGLE_COUNTER_CLOCKWISE) {
      velocity = abs(velocity);
    }

		uint8_t val = PINA & _BV(PA0);
		//(PINA & (1<<PA0));

		//sprintf(test, "val: %d, last: %d\n", val, last_val);
		//uart0_puts(test);

		//map values from joystick

		//int16_t velocity = map(y, 0, 1023, -500, 500);
		/*int16_t velocity = 0;
		if (y>800) velocity = 500;
		else if (y> 500) velocity = 300;
		else if (y> 300) velocity =100;
		else velocity = 0;
		// Inverts y controls
		velocity *= -0.5;
		int16_t radius = 0x8000;//map(x, 0, 1023, -2000, 2000);

		//send values to trigger manual control*/
    // reset send flag
    //send_flag=true;

    // test if in dead zone
    /*if(x> (512-25) && x < (512+25) && y > (512-25) && y < (512+25)) {
      if(prev_send_flag) send_flag = prev_send_flag;
    } else {
      send_flag = true;
    }*/


    if (send_flag==true) {
  		uart1_putc(1);
  		uart1_putc(2);
  		//uart1_putc(3);
  		uart1_putc(5);
  		uart1_putc(ROOMBA_DRIVE);
  		//split bytes per Roomba specs
  		uart1_putc(HIGH_BYTE(velocity));
  		uart1_putc(LOW_BYTE(velocity));
  		uart1_putc(HIGH_BYTE(radius));
  		uart1_putc(LOW_BYTE(radius));
    }
		//toggle laser and send input from button
		//uart1_putc(3);
		if(val!=last_val){
			uart1_putc(1);
			uart1_putc(3);
			uart1_putc(last_val=val);
		}
		//Task_Next();
		Task_Sleep(5);
	}
}

void idle(){
	for(;;) {

		Task_Next();
	}
}



int a_main() {
	DDRL = 0xFF;
	PORTL = 0xFF;
	DDRA &= ~_BV(DDA0);

	adc_init();
	//uart0_init(UART_BAUD_SELECT(19200, F_CPU));
	uart1_init(UART_BAUD_SELECT(19200, F_CPU));
	uart2_init(UART_BAUD_SELECT(19200, F_CPU));

	Task_Create(idle, 9, 0);
	//Task_Create(blink, 8, 0);
	Task_Create(pole_sensors, 8, 0);

	Task_Terminate();
	//return 0;
}
