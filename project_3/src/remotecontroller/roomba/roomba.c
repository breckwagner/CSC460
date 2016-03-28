/*
 * roomba.c
 *
 *  Created on: 4-Feb-2009
 *      Author: nrqm
 */
 #include <avr/io.h>
 #include <avr/interrupt.h>
#include <util/delay.h>
#include "../uart/uart.h"
#include "roomba.h"
//#include "sensor_struct.h"

#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define DD_DDR DDRE
#define DD_PORT PORTE
#define DD_PIN PE4



void roomba_init() {
	uint8_t i;
	DD_DDR |= _BV(DD_PIN);
	// Wake up the Roomba by driving the DD pin low for 500 ms.
	DD_PORT &= ~_BV(DD_PIN);
	_delay_ms(500);
	DD_PORT |= _BV(DD_PIN);

	// Wait for 2 seconds, Then pulse the DD pin 3 times to set the Roomba to operate at 19200 baud.
	// This ensures that we know what baud rate to talk at.
	_delay_ms(2000);
	for (i = 0; i < 6; i++) {
		DD_PORT ^= _BV(DD_PIN);
		_delay_ms(50);
	}

	DD_PORT &= ~_BV(DD_PIN);

	sei();

	uart1_init(UART_BAUD_SELECT(19200, F_CPU));
	// start the Roomba's SCI
	uart1_putc(ROOMBA_START);
	_delay_ms(20);

	// See the appropriate AVR hardware specification, at the end of the USART section, for a table of baud rate
	// framing error probabilities.  The best we can do with a 16 or 8 MHz crystal is 38400 bps, which has a framing
	// error rate of 0.2% (1 bit out of every 500).  Well, the best is 76800 bps, but the Roomba doesn't support
	// that.  38400 at 0.2% is sufficient for our purposes.  An 18.432 MHz crystal will generate all the Roomba's
	// baud rates with 0.0% error!.  Anyway, the point is we want to use a 38400 bps baud rate to avoid framing
	// errors.  Also, we have to wait for 100 ms after changing the baud rate.
  roomba_set_baud(ROOMBA_38400BPS);

	// change the AVR's UART clock to the new baud rate.
	uart1_init(UART_BAUD_SELECT(38400, F_CPU));

  roomba_start();
}


void roomba_set_baud(ROOMBA_BITRATE baudrate) {
  uart1_putc(ROOMBA_BAUD);
  uart1_putc(baudrate);
  _delay_ms(100);
}

void roomba_start() {
  // put the Roomba into safe mode.
	uart1_putc(ROOMBA_SAFE);
	_delay_ms(20);
}
