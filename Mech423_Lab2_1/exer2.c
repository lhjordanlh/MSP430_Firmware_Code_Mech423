#include "msp430x22x4.h"

/*
 * Digital I/O
 * 1. Configure P1.0 through P1.7 as digital outputs
 * 2. Set the output on port 1 to 10010011
 * 3. Use LED2 on the gumstick PCB to verify the output on port 1
 * 4. Write a program to blink LED3 connected to P1.7. Use a delay loop in the main infinite loop to visibly blink the LED
 */

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    volatile unsigned int i;            // volatile to prevent optimization

    P1DIR = BIT0 + BIT1 + BIT2 + BIT3+ BIT4+ BIT5+ BIT6+ BIT7; //configure all Pin P1.0 to P1.7 as output
    P1OUT = 0; // Set all P1 pin to zero output
    P1OUT = BIT0 + BIT1 + BIT4 + BIT7; // Set port 1 to 10010011

    _EINT(); //Enable global interrupts
    while (1){
    	P1OUT ^= BIT7;                      // Toggle P1.0 using exclusive-OR

 		i = 10000;                          // SW Delay
 		do i--;
		while (i != 0);

    };

}
