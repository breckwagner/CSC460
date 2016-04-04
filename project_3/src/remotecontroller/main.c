//#include "LED_Test.h"
#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include <string.h>

#include "../lib/roomba-lib/roomba.h"
#include "../lib/avr-uart/uart.h"
#include "../lib/rtos/os.h"

#define ROOMBA_RPC 0x01
#define REMOTE_PROCEDURE_CALL 0x01
#define REMOTE_FIRE 0x02

#define LOW_BYTE(v)   ((unsigned char) (v))
#define HIGH_BYTE(v)  ((unsigned char) (((unsigned int) (v)) >> 8))

#define DD_DDR DDRE
#define DD_PORT PORTE
#define DD_PIN PE4


/*******************************************************************************
 * Global Variables
 ******************************************************************************/

ROOMBA_PACKET_GROUP_100 roomba_state;

int32_t current_x = 0;
int32_t current_y = 0;


MUTEX uart_mutex;



void adc_init(void);

uint16_t read_adc(uint8_t channel);

void roomba_stop();

void toggle_laser(uint8_t val);

/******************************************************************************/

void idle();



void layer_1();
void layer_2();
void layer_3();

void roomba_init();

void handle_radio();

void update_roomba_state();

void pole_sensors();

void blink();

void Task_P1();



int a_main() {

  // Init Lazer
  DDRB = 0xFF;
  PORTB &= ~(1 << PB6); // Go low
  //PORTB |= (1 << PB6); // Pin goes high
	DDRL = 0xFF;
	PORTL = 0xFF;
	DD_DDR |= _BV(DD_PIN);
	DD_PORT &= ~_BV(DD_PIN);

  // init random number geneerator
  srand(1234);

  uart_mutex = Mutex_Init();

	adc_init();

  // Initialize debug interface with PC
	uart0_init(UART_BAUD_SELECT(38400, F_CPU));

	// initialize Communication with Roomba
	uart1_init(UART_BAUD_SELECT(19200, F_CPU));

  // Init Bluetooth communication
  uart2_init(UART_BAUD_SELECT(9600, F_CPU));

  Task_Create(roomba_init, 1, 0);
  Task_Create(idle, MINPRIORITY, 0);

  Task_Terminate();
  //return 0;
}


/*******************************************************************************

*******************************************************************************/

void roomba_init() {
	DD_DDR |= _BV(DD_PIN);
	DD_PORT &= ~_BV(DD_PIN);
	Task_Sleep(50); // sleep 500ms
	DD_PORT |= _BV(DD_PIN);
  Task_Sleep(200); // sleep 2000ms
	for (uint8_t i = 0; i < 6; i++) {
		DD_PORT ^= _BV(DD_PIN);
		Task_Sleep(5); // sleep 50ms
	}
	DD_PORT &= ~_BV(DD_PIN);
	uart1_putc(ROOMBA_START);
	Task_Sleep(2); // sleep 20ms


  uart1_putc(ROOMBA_BAUD);
  uart1_putc(ROOMBA_38400BPS);
  Task_Sleep(10); // sleep 100ms


  uart1_init(UART_BAUD_SELECT(38400, F_CPU));

  uart1_putc(ROOMBA_SAFE);
  Task_Sleep(2); // sleep 20ms

  Task_Create(layer_1, 1, 0);

  //Task_Create(layer_2, 1, 0);

  //Task_Create(blink, 1, 0);

  Task_Create(pole_sensors, 1, 0);

  Task_Terminate();
}


void idle() {
  for(;;);
}

void adc_init(void){
	//16MHz/128 = 125kHz the ADC reference clock
	ADCSRA |= ((1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0));
	ADMUX  |= (1<<REFS0);       //Set Voltage reference to Avcc (5v)
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
	uint8_t i = 0;
	for(;;i++) {
		DD_PORT ^= _BV(DD_PIN);
		uart1_putc(ROOMBA_LEDS);
		uart1_putc(0x01);
		if(i%2==0) uart1_putc(0xFF);
		else uart1_putc(0x00);
		uart1_putc(0xFF);
		if(i==50) Task_Terminate();
		else Task_Sleep(50);
	}
}

void handle_radio() {
  // GET USER input

  // Wait for data otherwise sleep
  while(!uart2_available());

  // Check that the data is a remote procedure call
  if((uint8_t)uart2_getc()==ROOMBA_RPC) {
    // Wait for data otherwise sleep
    while(!uart2_available());
    uint8_t tmp_cmd = (uint8_t)uart2_getc();
    if(tmp_cmd==ROOMBA_RPC+1) {
      uint8_t command[16];
      while(!uart2_available());
      uint8_t size = uart2_getc();
      for(uint8_t i = 0; i < size; i++){
        while(!uart2_available());
        command[i] = uart2_getc();
      }
      for(uint8_t i = 0; i < size; i++) {
        uart1_putc(command[i]);
      }
      //uart2_flush();
    } else if (tmp_cmd==ROOMBA_RPC+2) {
      while(!uart2_available());
      toggle_laser(uart2_getc());
    } else if (tmp_cmd==UART_NO_DATA) {
      uart2_flush();
    }
  } else {
    uart2_flush();
  }
}

void update_roomba_state() {
	char x[] = {ROOMBA_QUERY_LIST,
		ROOMBA_IR_OPCODE,
		ROOMBA_IR_OPCODE_LEFT,
		ROOMBA_IR_OPCODE_RIGHT, '\0'};

	uart1_puts(x);

	while(uart1_available()) {
		//uart0_puts(uart1_getc());
	}
	Task_Sleep(10);
}

void pole_sensors(){
  uint8_t i = 0;
	uint8_t buffersize = 16;
	uint16_t value[buffersize];
	uint8_t flag = 0;
	for(i = read_adc(PF0);;i = ((++i)%buffersize)){
		value[i] = read_adc(PF0);
		char message[15];
		itoa(value[i], message, 10);
		uart1_putc(ROOMBA_DIGIT_LEDS_ASCII);
		uart1_putc(message[0]);
		uart1_putc(message[1]);
		uart1_putc(message[2]);
		uart1_putc(message[3]);
		//sprintf(message, "0%.4d\0", tmp);
		//for(uint8_t i = 0; i < 5; i++) uart1_putc(message[i]);
		//blink();
		if(!flag && (i)?(value[i]>(value[i-1]+10)):(value[i]>(value[buffersize]+10))){
      uart2_putc(REMOTE_PROCEDURE_CALL);
      uart2_putc(2);
			Task_Create(blink, 8, 0);
			flag = 10;
		}
		if(flag) flag--;
		Task_Sleep(25);
	}
}

void toggle_laser(uint8_t val) {
  if (val==2) PORTB ^= (1<<PB6);
  else if (val) PORTB &= ~(1<<PB6);
  else PORTB |= (1<<PB6);
}



void layer_1() {

  for(;;) {
    handle_radio();

    Task_Sleep(1);
  }
}


void roomba_turn(int16_t degrees) {
  if(!degrees) return;

  {
    int16_t velocity = 500;
    int16_t angle = ROOMBA_ANGLE_COUNTER_CLOCKWISE;
    //if(degrees>0) angle = ROOMBA_ANGLE_CLOCKWISE;
    //else angle = ROOMBA_ANGLE_COUNTER_CLOCKWISE;

    uart1_putc(ROOMBA_DRIVE);
    uart1_putc(HIGH_BYTE(velocity));
    uart1_putc(LOW_BYTE(velocity));
    uart1_putc(HIGH_BYTE(angle));
    uart1_putc(LOW_BYTE(angle));
  }


  // Read it once to get rid of cummulative angle
  uart1_flush();
  uart1_putc(ROOMBA_QUERY_LIST);
  uart1_putc(1);
  uart1_putc(ROOMBA_ANGLE);
  while(!uart1_available());
  uart1_flush();
  Task_Sleep(1);
  for (int16_t angle = 0; angle < degrees;) {
    uart1_putc(ROOMBA_QUERY_LIST);
    uart1_putc(1);
    uart1_putc(ROOMBA_ANGLE);
    while(!uart1_available()); // TODO: get rid of magic number
    angle += (uart1_getc() << 8) | uart1_getc();
    Task_Sleep(10);
  }

  {
    int16_t velocity = 0;
    int16_t angle = 0x7FFF;
    uart1_putc(ROOMBA_DRIVE);
    uart1_putc(HIGH_BYTE(velocity));
    uart1_putc(LOW_BYTE(velocity));
    uart1_putc(HIGH_BYTE(angle));
    uart1_putc(LOW_BYTE(angle));
  }
}

void layer_2() {

  /*int16_t velocity = 250;
  int16_t angle = -1;
  char message[] = "TEST";
  uart1_putc(ROOMBA_DIGIT_LEDS_ASCII);
  uart1_putc(message[0]);
  uart1_putc(message[1]);
  uart1_putc(message[2]);
  uart1_putc(message[3]);

  //Mutex_Lock(uart_mutex);
  uart1_putc(ROOMBA_DRIVE);
  uart1_putc(HIGH_BYTE(velocity));
  uart1_putc(LOW_BYTE(velocity));
  uart1_putc(HIGH_BYTE(angle));
  uart1_putc(LOW_BYTE(angle));*/
  //Mutex_Unlock(uart_mutex);
  Task_Create(blink, 0, 0);
  for(;;) {
    roomba_turn(rand()%360);
    Task_Sleep(10000);
  }
}

void layer_3() {

}





void roomba_stop() {
  uart1_putc(ROOMBA_DRIVE);
  uart1_putc(HIGH_BYTE(0x00));
  uart1_putc(LOW_BYTE(0x00));
  uart1_putc(HIGH_BYTE(0x7FFF));
  uart1_putc(LOW_BYTE(0x7FFF));
}
