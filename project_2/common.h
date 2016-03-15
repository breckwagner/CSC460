

/**
 * @file avr_utility.h
 * @author Richard B. Wagner
 *
 */


#ifndef _COMMON_H_
#define _COMMON_H_

#define DIGITAL 0x7F
#define ANALOG 0xFF

/*******************************************************************************
 * MACROS
 ******************************************************************************/
#include <stdbool.h>
#include <avr/wdt.h>

#define soft_reset()      \
do {                      \
  wdt_enable(WDTO_15MS);  \
  for (;;) {}             \
} while(0)

/*******************************************************************************
 * Function Prototypes
 ******************************************************************************/

void signal_debug (uint8_t value, bool pulse);

/**
 * Enables inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t enable_global_interrupts();
/**
 * Disables inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t disable_global_interrupts();

/**
 * Restores inturupts by setting the global inturupt flag
 * @param (void)
 * @return the previous state of the interrupt flag
 */
uint8_t restore_global_interrupts(uint8_t saved_sreg);







// Function Pototype
void wdt_init(void) __attribute__((naked)) __attribute__((section(".init3")));


void kernel_init(void) __attribute__((naked)) __attribute__((section(".init8")));
/*
uint8_t set(uint8_t arduino_pin, uint16_t value) {
  switch (arduino_pin) {
    case 0: PORTL &= ~((value)<<PB0);
    PORTB |= (1<<PB7);
  }
}


void timer_init () {
  uint16_t CPU_FREQUANCY  = 16000000;
  uint8_t timer_resolution = 16;
  uint16_t prescaler = 1;
  total_timer_ticks;
  overflow_count;
  remainder_timer_ticks:
  real_time_seconds
  new_frequancy_hz


}
*/
#endif /* _COMMON_H_ */
