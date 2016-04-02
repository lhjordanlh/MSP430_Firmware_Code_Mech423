#include "msp430x22x4.h"

/*
 * This project is used to do the following:
 * 1. set up MCLK and ACKL to run on XT1
 * 2. set up SMCLK to run on the DCO
 * 3. Configure DCO to run at 500 kHz
 * 4. set up the SMCLK with a divider of 4
 * 5. Configure P2,1 as an output and set it to output the SMCLK. (SMCLK freq=125kHz)
 * main.c
 */
int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    P9DIR |= 0x01;                            // Set P2.1 to output direction

    BCSCTL2 |= SELM0 + SELM1;	// XT1 -> MCLK
    BCSCTL2 &= ~SELS; // DCO -> SMCLK

    BCSCTL1 &= ~RSEL1; //

    DCOCTL &= ~DCO1;


    while (1);
}
