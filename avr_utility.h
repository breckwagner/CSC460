

/**
 * @file avr_utility.h
 * @author Richard B. Wagner
 *
 */

#define DIGITAL 0x7F
#define ANALOG 0xFF

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
