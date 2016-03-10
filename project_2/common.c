
#include <stdbool.h>

#include <common.h>


void signal_debug (uint8_t value, bool pulse) {
bool flag = true;
  while (flag) {
    switch (value) {
      case 0: PORTL ^= (1<<PL3); break;
      case 1: PORTL ^= (1<<PL2); break;
      case 2: PORTL ^= (1<<PL1); break;
      case 3: PORTL ^= (1<<PL0); break;
      case 4: PORTB ^= (1<<PB3); break;
      case 5: PORTB ^= (1<<PB2); break;
      case 6: PORTB ^= (1<<PB1); break;
      case 7: PORTB ^= (1<<PB0); break;
    }
    if (pulse) {
      pulse = false;
    } else {
      flag = false;
    }
  }
}

uint8_t enable_global_interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("sei"::);
  return sreg;
}

uint8_t disable_global_interrupts() {
  uint8_t sreg = SREG;
  asm volatile ("cli"::);
  return sreg;
}

uint8_t restore_global_interrupts(uint8_t saved_sreg) {
  if (saved_sreg & 0x80) {
    return enable_global_interrupts();
  } else {
    return disable_global_interrupts();
  }
}

// To be called at boot to reset watchdog timer
void wdt_init(void) {
  MCUSR = 0;
  wdt_disable();
  return;
}
