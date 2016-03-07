/*os.c

RTOS setup
This is where everything that is needed to fire up the RTOS will be written
Need to create an initial task, called a_main, then terminate

*/

#include <avr/io.h>
#include <avr/interrupt.h>

#include "os.h"
#include "kernel.h"