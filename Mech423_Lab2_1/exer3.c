#include "msp430x22x4.h"

/*
 * Interrupts
 * 1. Configure P1.0 as a digital input
 * 2. Hook up the pushbutton switch to P1.0.
 * 	*Note the pushbutton on the Gumstick alread contains a pull-up resistor and a debouncing capacitor.
 * 3. Set P1.0 to get interrupted from a rising edge. Enable local and global interrupts.
 * 4. Write an interrupt service routine to toggle P1.1 when interrupted by an upward swinging pulse.
 * 5. Congifute P1.1 as an output and hook up an LED
 * 6. Show that each press of the button tottles the LED
 */

#pragma vector= PORT1_VECTOR
__interrupt void P1_ISR(void){
	P1OUT ^= BIT1; // toggle Pin1.1
    P1IFG &= ~BIT0; //Resetting the flag to zero
}

void main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    volatile unsigned int i;            // volatile to prevent optimization

    P1DIR |= BIT1; //Set pin1.1 to output
    P1IES &= ~BIT0; //Setting low to high transition
    P1IFG &= ~BIT0; //Resetting the flag

    _EINT(); //Enable global interrupts
    P1IE |= BIT0; //Enable local interrupt

    while (1);
}
