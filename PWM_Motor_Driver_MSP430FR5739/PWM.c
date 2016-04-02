#include <msp430.h> 

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

//    volatile unsigned int i;            // volatile to prevent optimization
    CSCTL0 = CSKEY;
	CSCTL2 = SELS_3; // DCO -> SMCLK

	CSCTL1 &= ~(DCORSEL+DCOFSEL1+DCOFSEL0) ; // set DCO to run 5.33 MHz
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;        // set all dividers

    PJOUT = 0;
	PJDIR |= BIT0; //Step one to set PJ.0 to Output SMCLK
	PJSEL0 |= BIT0; //Step two to set PJ.0 to Output SMCLK
	PJSEL1 &= ~BIT0; //Step three to set PJ.0 to Output SMCLK


//    TACTL |= MC0; //Setup Timer A in the "up count" mode
//
//    TACTL |= TASSEL1; //Let Timer1 use SMCLK
//
////    TACCTL0 = CCIE; // Enable counter interrupts for TACCR0
////    TACCTL1 = CCIE; // Enable counter interrupts for TACCR1
//
//    TACCR0 = 499; 	// PWM period
//    						// 500KHz clock gives 500,000/1000 = 500Hz = 1/500.s period
// //   TACCR1 = 250-1; // Duty Cycle equals 25%
//
//    TACCTL0 |= OUTMOD_4; //Set reset and set mode
//
//    P1DIR |= BIT5; //step one to set P1.5 to output TA0
//    P1SEL |= BIT5; //step two to set P1.5 to output TA0
//
////   P1DIR |= BIT6 //Step one to set P1.6 to output TA1
////   P1SEL |= BIT6 // Step two to set P1.6 to output TA1

    while (1);

}
