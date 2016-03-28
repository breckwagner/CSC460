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


void blink() {
	static uint8_t i = 0;
	for(;;i++) {
		uart1_putc(ROOMBA_LEDS);
		uart1_putc(0x01);
		if(i%2==0) uart1_putc(0xFF);
		else uart1_putc(0x00);
		uart1_putc(0xFF);
		//Task_Sleep(100);
	}
}

void pole_sensors(){
	for(;;){
		uint16_t tmp = read_adc(PF0);
		char buffer[4];
		char message[15];// = {ROOMBA_DIGIT_LEDS_ASCII,'F','U','L','A'};
		itoa(tmp, message, 10);
		//buffer[0] = ROOMBA_DIGIT_LEDS_ASCII;
		uart1_putc(ROOMBA_DIGIT_LEDS_ASCII);
		//uart1_puts(buffer);
		uart1_putc(message[0]);
		uart1_putc(message[1]);
		uart1_putc(message[2]);
		uart1_putc(message[3]);
		//sprintf(message, "0%.4d\0", tmp);
		//for(uint8_t i = 0; i < 5; i++) uart1_putc(message[i]);
		//blink();
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

	roomba_init(); // initialize the roomba
	Task_Create(idle, 9, 0);
	//Task_Create(blink, 8, 0);
	Task_Create(pole_sensors, 8, 0);
	return 0;
}
