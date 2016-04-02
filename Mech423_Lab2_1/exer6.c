#include "msp430x22x4.h"

/*
 * Timer 2
 * 1. Set up Timer B to measure the length of time of a pulse from a rising edge to a falling edge
 * 2. Connect the timer output from the previous exercise to the input of this timer
 * 3. Using the debugger to check the measured 16-bit value
 */

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    volatile unsigned int i;            // volatile to prevent optimization

    P1DIR = BIT0; //set P1.0 as output


    _EINT(); //Enable global interrupts
    while (1){
    	P1OUT ^= BIT7;                      // Toggle P1.0 using exclusive-OR

 		i = 10000;                          // SW Delay
 		do i--;
		while (i != 0);

    };

}
