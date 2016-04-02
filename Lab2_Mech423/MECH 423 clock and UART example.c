#include "msp430x22x4.h"

#pragma vector=USCIAB0RX_VECTOR // defined in "msp430x22x4.h"
__interrupt void UART_RX(void) // interrupt on incoming data
{
  unsigned char rx_byte;
  rx_byte = UCA0RXBUF; // grab byte from RX buffer
  while((IFG2 & UCA0TXIFG) == 0); // wait for TX buffer to be clear
  UCA0TXBUF = rx_byte;	// Echo the byte back
}

void main(void)
{
  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
  BCSCTL1 = XT2OFF + XTS;				// XT2 off, XT1 high freq
  BCSCTL2 = SELM0 + SELM1 + SELS;		// XT1 -> MCLK, XT1 -> SMCLK
  BCSCTL3 = LFXT1S1;					// XT1 freq 3-16 MHz

  P3SEL = BIT4 | BIT5;					// Set up ports 3.4 and 3.5 as UART ports

  UCA0CTL1 = UCSSEL1;					// Use SMCLK for UART
  UCA0BR0 = 1666 & 0xFF; 				// Set baud rate
  UCA0BR1 = 1666 >> 8;
  UCA0MCTL = UCBRS_6;					// Set modulation bits
  IE2 |= UCA0RXIE;						// Enable Rx interrupt

  _EINT();		// Global interrupt enable

  while (1);
}
