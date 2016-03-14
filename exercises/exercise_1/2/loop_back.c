#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>  
#ifndef F_CPU
//define cpu clock speed if not defined
#define F_CPU 3686400
#endif
//set desired baud rate
#define BAUDRATE 19200
//calculate UBRR value
#define UBRRVAL ((F_CPU/(BAUDRATE*16UL))-1)
void USART_Init()
{
	//Set baud rate
	UBRRL=UBRRVAL;		//low byte
	UBRRH=(UBRRVAL>>8);	//high byte
	//Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size
	UCSRC=(1<<URSEL)|(0<<UMSEL)|(0<<UPM1)|(0<<UPM0)|
		(0<<USBS)|(0<<UCSZ2)|(1<<UCSZ1)|(1<<UCSZ0);	
	//Enable Transmitter and Receiver and Interrupt on receive complete
	UCSRB=(1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
	//enable global interrupts
	set_sleep_mode(SLEEP_MODE_IDLE);
	sei();
}
ISR(USART_RXC_vect)
{
	//defien temp value for storing received byte
	uint8_t Temp;
	//Store data to temp
	Temp=UDR;
	Temp++;//increment
	//send received data back
	// no need to wait for empty send buffer
	UDR=Temp;
}
int main(void)
{
	USART_Init();
	while(1)
	sleep_mode();
	//nothing here interrupts are working
	return 0;
}