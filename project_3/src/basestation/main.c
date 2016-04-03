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

#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

void adc_init(void){

	//16MHz/128 = 125kHz the ADC reference clock

	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

	ADMUX |= (1<<REFS0);       //Set Voltage reference to Avcc (5v)

	ADCSRA |= (1<<ADEN);       //Turn on ADC

	ADCSRA |= (1<<ADSC);
}     //Do an initial conversion


//map input values to those appropriate for Roomba
int16_t map(int16_t x, int16_t in_min, int16_t in_max, int16_t out_min, int16_t out_max){ 
  	
	int16_t result =  ((x - in_min) * (out_max - out_min)) / ((in_max - in_min) + out_min);
	return result;
}

uint16_t read_adc(uint8_t channel){
	ADMUX &= 0xE0;           //Clear bits MUX0-4
	ADMUX |= channel&0x07;   //Defines the new ADC channel to be read by setting bits MUX0-2
	ADCSRB = channel&(1<<3); //Set MUX5
	ADCSRA |= (1<<ADSC);      //Starts a new conversion
	while(ADCSRA & (1<<ADSC));  //Wait until the conversion is done
	return ADCW;
}         //Returns the ADC value of the chosen channel


void pole_sensors(){
	uart1_init(UART_BAUD_SELECT(9600, F_CPU));
	//uart0_init(UART_BAUD_SELECT(9600, F_CPU));
	//char str[16];
	for(;;){
		//read input from analog pins 14,15
		uint16_t x = read_adc(14);
		uint16_t y = read_adc(15);
		//sprintf(str, "%d, %d\n", x,y);
		//uart0_puts(str);
		//init digital pin 22 and set as input
		//DDRB &= ~_BV(DDA0);
		//int val = PINA & _BV(PA0);
		//map values from joystick
		int16_t velocity = map(y, 0, 1023, -500, 500);
		int16_t radius = 0x8000;//map(x, 0, 1023, -2000, 2000);
		//send values to trigger manual control
		uart1_putc(1);
		uart1_putc(5);
		uart1_putc(ROOMBA_DRIVE);
		//split bytes per Roomba specs
		uart1_putc(HIGH_BYTE(velocity));
		uart1_putc(LOW_BYTE(velocity));
		uart1_putc(HIGH_BYTE(radius));
		uart1_putc(LOW_BYTE(radius));
		//toggle laser and send input from button
		//uart1_putc(1);
		//uart1_putc(val);	
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
	adc_init();
	//uart0_init(UART_BAUD_SELECT(19200, F_CPU));
	uart1_init(UART_BAUD_SELECT(19200, F_CPU));
	uart2_init(UART_BAUD_SELECT(19200, F_CPU));

	Task_Create(idle, 9, 0);
	//Task_Create(blink, 8, 0);
	Task_Create(pole_sensors, 8, 0);
	return 0;
}
