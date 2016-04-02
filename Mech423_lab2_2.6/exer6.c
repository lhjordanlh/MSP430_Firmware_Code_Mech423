#include "msp430x22x4.h"

/*
 * Timer 2
 * 1. Set up Timer B to measure the length of time of a pulse from a rising edge to a falling edge
 * 2. Connect the timer output from the previous exercise to the input of this timer
 * 3. Using the debugger to check the measured 16-bit value
 */
volatile unsigned int rise_temp =0;
volatile unsigned int fall_temp = 0;
volatile unsigned int period= 0;

#pragma vector= TIMERB1_VECTOR
__interrupt void timerB_Interrupt(void){
	if(((TBCCTL1 & CM0)>>14)==1){
		rise_temp = TBCCR1;	//Measuring the rise TBR
		TBCCTL1 |= CM1; //Toggle capture on mode
		TBCCTL1 &= ~CM0;  //toggle capture on mode 2
	}
	else
	{
		fall_temp=TBCCR1; //Measuring the fall TBR
		TBCCTL1 |= CM0; //Toggle capture on mode
		TBCCTL1 &= ~CM1; //Toggle capture on mode 2
	}

	period= fall_temp - rise_temp; //Calculate the TBR
	TBCCTL1 &= ~CCIFG;
}

int main(void) {

	  	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	    volatile unsigned int i;            // volatile to prevent optimization

	    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

		BCSCTL2 |= SELM0 + SELM1;	// XT1 -> MCLK
		BCSCTL2 &= ~SELS; // DCO -> SMCLK

		BCSCTL1 &= ~RSEL1; // first step to set DCO to run 500 kHz
		DCOCTL &= ~(DCO0); // another line to help set DCO to run at 500 kHz

		P2DIR |= BIT1; //Step one to set P2.1 to Output SMCLK
		P2SEL |= BIT1; //Step two to set P2.1 to Output SMCLK


	//	TACCTL0 = CCIE; // Enable counter interrupts for TACCR0

//Set up TimerA
		TACTL |= MC0; //Setup Timer A in the "up count" mode
	    TACTL |= TASSEL1; //Let Timer1 use SMCLK
	    TACCR0 = 999; 	// PWM period
	    				// 500KHz clock gives 500,000/1000 = 500Hz = 1/500.s period
	    TACCR1 = 500-1; // Duty Cycle equals 50%

	    TACCTL1 |= OUTMOD_7; //Set reset and set mode


//Setup outputs
    P1DIR |= BIT6; //step one to set P1.6 to output TA1
    P1SEL |= BIT6; //step two to set P1.6 to output TA1

//Set up TimerB
    P4DIR &= ~BIT1; //Set Port 4.1 to input
    P4SEL |= BIT1; //Set Port 4.1 -> TimerB.CCI1A
    TBCTL |= MC1; //Set timer B in continuos mode
    TBCTL |= TBSSEL1; //Let TimerB use SMCLK
    TBCCTL1 |= SCS + CAP + CM0 + CCIE; // Set timer B to sync, capture mode and capture on rising edge and enable interrupt
 //   TBCTL |= ID1; //Slow down the clock by divided by 4

    _EINT(); //enable global interrupt

    while (1);

}
