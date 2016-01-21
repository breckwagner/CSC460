
 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/delay.h>

// for incoming serial data
int incomingByte = 0;

// control-c char
int EOT = 0x03;

int count = 0;

int LINE_WIDTH = 30;

int sub_task () {
	PORTB ^= _BV(PB7);
	_delay_ms(250);	
}

ISR (UART0_RX_vect) {
	PORTB ^= _BV(PB7);
}




int main() {
	
	DDRB |= _BV(PB7);
	
	sub_task();
	sub_task();
	sub_task();
	sub_task();
	sub_task();
	sub_task();
	
	
	
	
	sei();                    // turn on interrupts
	
	sleep_mode();
	
	while(1) {
	}
}
