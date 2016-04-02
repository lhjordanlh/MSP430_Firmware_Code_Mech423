#include "msp430x22x4.h"

/*
 * ADCII
 * 1. Enable the internal 2.5 V reference and configure the P2.4 pin to output this value
 * 2. Conncect a 10kohm potentiometer between the 2.5V reference and ground.
 * 3. Connect the wiper terminal of the potentiometer to _4.3/A12
 * 4. Sample the voltage on A12, and bit shift the 10-bit result to an 8-bit value
 * 5. Tansmit the resulf using the UART
 * 6. Show this result using the simple C# program
 * 7. Rotate potentiometer wiper and show the range of values that could be obtained
 */

volatile unsigned int tempADC = 0;
volatile unsigned int tx_data = 0;

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
	tempADC = ADC10MEM;							//Store ADC 10bits
	tx_data = tempADC >> 2;						//Shift ADC 10bits into 8bits
	while((IFG2 & UCA0TXIFG)==0);				//Wait for free TX
	UCA0TXBUF = tx_data;						//Send data
	ADC10CTL0 &= ~ADC10IFG;						//Reset interrupt flag
}

void main(void)
{
	WDTCTL = WDTPW + WDTHOLD;					//Stop Watchdog Timer

	BCSCTL1 = XTS;								//Set high freq XT1
	BCSCTL2 = SELM0 + SELM1 + SELS;				//XT1 -> MCLK, ACLK, SMCLK
	BCSCTL3 = LFXT1S1;							//3MHz-16MHz crystal oscillator

	P3SEL = BIT4 | BIT5;						//Set UART ports - Pins 3.4 and 3.5

	UCA0CTL1 = UCSSEL1;							//SMCLK -> UART
	UCA0CTL0 &= ~(UCPEN + UCSPB);				//No parity bit and one stop bit
	UCA0BR0 = 125 & 0xFF;						//Set 128kHz baud rate
	UCA0BR1 = 125 >> 8;
	UCA0MCTL = UCBRS_6;							//Set modulation bits

	ADC10AE0 |= BIT4;							//Enable Vref pin out
	ADC10AE1 |= BIT4;							//Enable ADC analog input A12

	ADC10CTL0 |= SREF_1 + ADC10ON + ADC10IE + REFON + REFOUT;	//Set Vref and Vss as ref, ADC on, ADC interrupt enabled, ref on, ref out on
	ADC10CTL0 |= REF2_5V;						//Set 2.5V as reference

	ADC10CTL1 |= INCH_12;						//ADC input select A12

	__delay_cycles(1000);						//Wait for clock to settle

	_EINT();									//Enable global interrupt
	while(1){
		ADC10CTL0 |= ENC + ADC10SC;				//Turn on ADC conversion and sample
	};
}
