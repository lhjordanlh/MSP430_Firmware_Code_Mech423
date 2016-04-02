#include "msp430x22x4.h"

/*
 * Exam_Exer2
 * Setup ADC Samples
 *  * ADCI
 * 1. Set up the ADC to sample from ports A13, A14, and A15
 * 2. Write code to sample data from all three accelerometer axes
 * 3. Bit shift the 10-bits result to an 8-bits value
 * 4. In the main infinite loop, transmit the result using the UART with 255 as the starting byte
 * 5. Set up a timer to trigger an interrupt every 40 ms (25 Hz)
 * 6. Sample the ADC inside the interrupt service routine and transmit the result to the PC.
 * 		Check the transmission rate using an Oscilloscope by probing the UART Tx port, P3.4
 * 7.Test the MSP430 code using the C# program from Lab1
 */

/*
#pragma vector= TIMERA0_VECTOR
__interrupt void timer_Interrupt(void){
	TACCTL0 ^= OUT;
	TACCTL0 &= ~CCIFG; //Reset the interrupt flag
}
*/

volatile unsigned int tempADC = 0;
volatile unsigned int tx_data = 0;
static unsigned int packetSize = 4;
volatile unsigned int tx_packet[4];
volatile unsigned int idx = 1;

#pragma vector=TIMERA0_VECTOR
__interrupt void TIMER_A0_INTERRUPT(void)
{
	for(idx = 1; idx < packetSize; idx++)		//Constructing data packet
	{
		ADC10CTL0 &= ~ENC;
		ADC10CTL1 &= ~(INCH0+INCH1+INCH2);
		if(idx == 1){
			ADC10CTL1 |= INCH_13;
		}else if(idx == 2){
			ADC10CTL1 |= INCH_14;
		}else if(idx == 3){
			ADC10CTL1 |= INCH_15;
		}
		ADC10CTL0 |= ENC + ADC10SC;
		while((ADC10CTL1 & ADC10BUSY) != 0);
		tempADC = ADC10MEM;
		tx_data = tempADC >> 2;
		tx_packet[idx] = tx_data;
	}

	while((IFG2 & UCA0TXIFG) == 0);
		UCA0TXBUF = tx_packet[2];

/*
	for(idx = 0; idx < packetSize; idx++)		//Send data packet
	{
		while((IFG2 & UCA0TXIFG) == 0);
		UCA0TXBUF = tx_packet[idx];
	}
*/
	  P1OUT ^=  BIT7;

	TACCTL0 &= ~CCIFG;							//Reset interrupt flag
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;					//Stop Watchdog Timer

	BCSCTL2 = SELM1 + SELM0 + DIVS1;			//XT1 -> MCLK + ACLK, SMCLK div 4, DCO -> SMCLK
	BCSCTL3 = LFXT1S1;							//3MHz-16MHz Crystal Oscillator
	BCSCTL1 = XTS + XT2OFF + RSEL0 + RSEL2;		//XT1 High frequency, DCO(5,2) ~500kHz
	DCOCTL = DCO1;

	P3SEL = BIT4 | BIT5;						//Set UART ports

	UCA0CTL1 = UCSSEL_1;						//ACLK -> UART
	UCA0CTL0 &= ~(UCPEN + UCSPB);				//Parity bit disabled, one stop bit
	UCA0BR0 = 1666 & 0xFF;						//Set 9600Hz baud rate
	UCA0BR1 = 1666 >> 8;
	UCA0MCTL = UCBRS_6;							//Set modulation bits

	TACTL = TASSEL1 + MC0;						//Clock source: SMCLK, Up Mode
	TACCR0 = 24499;								//Set 5Hz

	ADC10CTL0 |= SREF_0 + ADC10ON;				//Turn on ADC and set Vcc/Vss as ref
	ADC10AE1 |= BIT5 + BIT6 + BIT7;				//ADC Analog Enable - A13, A14, A15

	tx_packet[0] = 255;							//Insert start byte into packet

	__delay_cycles(1000);						//Wait for clock to stabilize
	
	TACCTL0 |= CCIE;							//Enable TACCR0 interrupt
	_EINT();									//Global interrupt enable
	while(1);
}
