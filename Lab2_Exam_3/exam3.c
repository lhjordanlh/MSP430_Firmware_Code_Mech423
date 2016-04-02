#include "msp430x22x4.h"

/*
 * Exam_Exer3
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

int aveY=1;

//Queue structure and its elements
volatile char queue[16];
volatile int startIndex = 0;
volatile int endIndex = 0;
volatile int size = 0;

void Enqueue(char queue[], char element) {
	if(size<16){
		queue[endIndex]= element;
		size++;
		if(endIndex<=15)
			endIndex++;
		else
			endIndex=0;
	}
	else{
		int temp=Dequeue(queue);
		queue[endIndex]= element;
		size++;
		if(endIndex<=15)
			endIndex++;
		else
			endIndex=0;
	}
}

char Dequeue(char queue[]) {
	if(size>0){
		char temp= queue[startIndex];
		queue[startIndex]= '0';
		size--;
		if(startIndex<15)
			startIndex++;
		else
			startIndex=0;

		return temp;
	}
	else{
		return '0';
	}
}

int GetQueueSize(int queue[]){
	return size;
}

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

	Enqueue(queue,tx_packet[2]);
//	while((IFG2 & UCA0TXIFG) == 0);
//		UCA0TXBUF = tx_packet[2];

/*
	for(idx = 0; idx < packetSize; idx++)		//Send data packet
	{
		while((IFG2 & UCA0TXIFG) == 0);
		UCA0TXBUF = tx_packet[idx];
	}
*/


	TACCTL0 &= ~CCIFG;							//Reset interrupt flag
}

int averageCal(char queue[]){
	int temp=0;
	int count=0;
	int ave=0;
	int sum=0;

	while(count<16){
		temp=sum;
		sum=temp+queue[count];
		count++;
	}
	ave = sum/16;
	return ave;
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
	TACCR0 = 1224;								//Set 100Hz

	ADC10CTL0 |= SREF_0 + ADC10ON;				//Turn on ADC and set Vcc/Vss as ref
	ADC10AE1 |= BIT5 + BIT6 + BIT7;				//ADC Analog Enable - A13, A14, A15

	tx_packet[0] = 255;							//Insert start byte into packet

	__delay_cycles(1000);						//Wait for clock to stabilize
	
	TACCTL0 |= CCIE;							//Enable TACCR0 interrupt
	_EINT();									//Global interrupt enable





    TBCTL |= MC0; //Setup Timer A in the "up count" mode

    TBCTL |= TASSEL1; //Let Timer1 use SMCLK

//    TACCTL0 = CCIE; // Enable counter interrupts for TACCR0
//    TACCTL1 = CCIE; // Enable counter interrupts for TACCR1

    TBCCR0 = 999; 	// PWM period
    						// 500KHz clock gives 500,000/1000 = 500Hz = 1/500.s period
    TACCR1 = 800-1; // Duty Cycle equals 80%

    TACCTL1 |= OUTMOD_7; //Set reset and set mode

//    P1DIR |= BIT5; //step one to set P1.5 to output TA0
//    P1SEL |= BIT5; //step two to set P1.5 to output TA0

    P1DIR |= BIT6; //Step one to set P1.6 to output TA1
    P1SEL |= BIT6; // Step two to set P1.6 to output TA1

	while(1){
		aveY=averageCal(queue);
	    TACCR1 = 500/aveY;
	}
}
