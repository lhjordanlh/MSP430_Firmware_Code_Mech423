#include "msp430x22x4.h"

/*
 * Timer 1
 * 1. Set up Timer A in the "up count" mode
 * 2. Configure TA0 to produce a 500 Hz square wave. You may need to use frequency divider when setting up the clock and the timer. Output on P1.5.
 * 3. Configure TA1 to produce a 500 Hz square wave at 25% duty cycle. Output on P1.6.
 * 4. Connect LED2 to TA1. Show vary the duty cycle and show that it leads to a change in brightness
 */

/*
#pragma vector= TIMERA0_VECTOR
__interrupt void timer_Interrupt(void){
	TACCTL0 ^= OUT;
	TACCTL0 &= ~CCIFG; //Reset the interrupt flag
}
*/

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    volatile unsigned int i;            // volatile to prevent optimization

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	BCSCTL2 |= SELM0 + SELM1;	// XT1 -> MCLK
	BCSCTL2 &= ~SELS; // DCO -> SMCLK

	BCSCTL1 &= ~RSEL1; // first step to set DCO to run 500 kHz
	DCOCTL &= ~(DCO0); // another line to help set DCO to run at 500 kHz

//	BCSCTL2 |= DIVS1; // set up the SMCLK with a diveder of 4

	P2DIR |= BIT1; //Step one to set P2.1 to Output SMCLK
	P2SEL |= BIT1; //Step two to set P2.1 to Output SMCLK


    TACTL |= MC0; //Setup Timer A in the "up count" mode

    TACTL |= TASSEL1; //Let Timer1 use SMCLK

//    TACCTL0 = CCIE; // Enable counter interrupts for TACCR0
//    TACCTL1 = CCIE; // Enable counter interrupts for TACCR1

    TACCR0 = 499; 	// PWM period
    						// 500KHz clock gives 500,000/1000 = 500Hz = 1/500.s period
 //   TACCR1 = 250-1; // Duty Cycle equals 25%

    TACCTL0 |= OUTMOD_4; //Set reset and set mode

    P1DIR |= BIT5; //step one to set P1.5 to output TA0
    P1SEL |= BIT5; //step two to set P1.5 to output TA0

//   P1DIR |= BIT6 //Step one to set P1.6 to output TA1
//   P1SEL |= BIT6 // Step two to set P1.6 to output TA1

    while (1);

}
