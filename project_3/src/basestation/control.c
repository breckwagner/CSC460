/*
 * control.c
 *
 *  Created on: 14-July-2010
 *      Author: lienh
 */
//#include "radio/radio.h"
#include "roomba/roomba.h"
#include "uart/uart.h"
#include "rtos/os.h"

#include <util/delay.h>
#include <avr/interrupt.h>

void adc_init(void){

	//16MHz/128 = 125kHz the ADC reference clock

	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));

	ADMUX |= (1<<REFS0);       //Set Voltage reference to Avcc (5v)

	ADCSRA |= (1<<ADEN);       //Turn on ADC

	ADCSRA |= (1<<ADSC);
}     //Do an initial conversion


uint16_t read_adc(uint8_t channel){
	ADMUX &= 0xE0;           //Clear bits MUX0-4
	ADMUX |= channel&0x07;   //Defines the new ADC channel to be read by setting bits MUX0-2
	ADCSRB = channel&(1<<3); //Set MUX5
	ADCSRA |= (1<<ADSC);      //Starts a new conversion
	while(ADCSRA & (1<<ADSC));  //Wait until the conversion is done
	return ADCW;
}         //Returns the ADC value of the chosen channel


void pole_sensors(){
	for(;;){
		uint16_t x = read_adc(PF0);
		uint16_t y = read_adc(PF1);
		Task_Next();
		Task_Sleep(100);
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
