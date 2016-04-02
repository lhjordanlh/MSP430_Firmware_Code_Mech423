#include "msp430x22x4.h"

/*
 * UART
 * 1. configure the UART to operate at 9600, 8, N, 1
 * 2. Set up P3.4 and P3.5 for UART communications
 * 3. Write a program to priodically transmit the letter'a' to the serial port
 * 4. Check the transmission using PuTTY
 * 5. Enable UART receive interrupt. Enable Global Interrupt
 * 6. Set up an interrupt service routine so that when a single byte is received,
 * 		the same byte is transmitted by back (or echoed) to the serial port. Check using PuTTy
 * 7. In addition to echoing, also transmit the next byte in the ASCII table. Check again using PuTTy
 * 8. Add Code to turn on LED3 when a 'j' is receivd and turn off LED3 when 'k' is received/.
 * 9. Change the UART speed to 57.7 kbaud and make sure the code in 4.7 and 4.8 still work
 */

#pragma vector=USCIAB0RX_VECTOR // defined in "msp430x22x4.h"
__interrupt void UART_RX(void) // interrupt on incoming data
{
  unsigned char rx_byte;
  rx_byte = UCA0RXBUF; // grab byte from RX buffer
  while((IFG2 & UCA0TXIFG) == 0); // wait for TX buffer to be clear
  UCA0TXBUF = rx_byte;	// Echo the byte back
  volatile unsigned int i=1000;            // volatile to prevent optimization;

  do(i--);
  	  while(i!=0);

  UCA0TXBUF = rx_byte+1;	// Echo the next byte back

  if(rx_byte=='j'){
	  P1OUT |=  BIT7;
  }

  if(rx_byte=='k'){
	  P1OUT &=~ BIT7;
  }
}

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

//set up timer
    BCSCTL1 = XT2OFF + XTS;				// XT2 off, XT1 high freq
    BCSCTL2 = SELM0 + SELM1 + SELS;		// XT1 -> MCLK, XT1 -> SMCLK
    BCSCTL3 = LFXT1S1;					// XT1 freq 3-16 MHz

    volatile unsigned int i;            // volatile to prevent optimization

    P1DIR |= BIT7; //Set pin1.1 to output


    P3SEL = BIT4 | BIT5;				// Set up ports 3.4 and 3.5 as UART ports


    UCA0CTL1 = UCSSEL1;					// Use SMCLK for UART
    UCA0BR0 = 1666 & 0xFF; 				// Set baud rate
    UCA0BR1 = 1666 >> 8;
    UCA0MCTL = UCBRS_6;					// Set modulation bits
    IE2 |= UCA0RXIE;					// Enable Rx interrupt

    _EINT();		// Global interrupt enable

	while (1){

		UCA0TXBUF= 'a';

		i = 10000;                          // SW Delay
	 		do i--;
			while (i != 0);

    };

}
