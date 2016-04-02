#include "msp430x22x4.h"

/*
 * ADCI
 * 1. Set up the ADC to sample from ports A13, A14, and A15
 * 2. Write code to sample data from all three accelerometer axes
 * 3. Bit shift the 10-bits result to an 8-bits value
 * 4. In the main infinite loop, transmit the result using the UART with 255 as the starting byte
 * 5. Set up a timer to trigger an interrupt every 40 ms (25 Hz)
 * 6. Sample the ADC inside the interrupt service routine and transmit the result to the PC.
 * 		Check the transmission rate using an Oscilloscope by probing the UART Tx port, P3.4
 * 7.Test the MSP430 code using the C# program from Lab1
 */

volatile unsigned int xAccel = 0;
volatile unsigned int yAccel = 0;
volatile unsigned int zAccel = 0;

/*
#pragma vector= ADC10_VECTOR
__interrupt void ADC10_interrupt(void){
	tempADC = ADC10MEM;							//Store ADC 10 bits data
	tx_data = tempADC >> 2;						//Shift ADC data from 10 bits to 8 bits, removing least sig bits
	tx_packet[idx] = tx_data;					//Store in data packet
	if(idx == 3){								//If data packet full
		for(idx = 0; idx < packetSize; idx++)	//Sending data packet over UART
		{
			while((IFG2 & UCA0TXIFG) == 0);
			UCA0TXBUF = tx_packet[idx];
		}
		idx = 1;								//Resetting index
	}else{										//If data packet is NOT full
		idx++;									//Increasing index
	}
	ADC10CTL0 &= ~ADC10IFG;						//Reset interrupt flag
}
}
*/

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    volatile unsigned int i;            // volatile to prevent optimization

//Set up Timer
    BCSCTL2 |= SELM0 + SELM1;	// XT1 -> MCLK
    BCSCTL2 &= ~SELS; // DCO -> SMCLK

    BCSCTL2 |= SELM0 + SELM1;	// XT1 -> MCLK
    BCSCTL2 &= ~SELS; // DCO -> SMCLK

//Set up ADC10
    ADC10CTL0 |= ADC10ON + ADC10IE; //ADC on and enable Interrupt
    ADC10CTL0 &= ~(SREF0 + SREF1 + SREF2); //Set reference to 000 -> Vr+ =Vcc & Vr- = Vss

    ADC10CTL1 |= ADC10SSEL0 + ADC10SSEL1;	//SMCLK -> ADC10



 //Set up UART
    UCA0CTL1 = UCSSEL1;					// Use SMCLK for UART
    UCA0BR0 = 125 & 0xFF; 				// Set baud rate as 128 kHz
    UCA0BR1 = 125 >> 8;
    UCA0MCTL = UCBRS_6;					// Set modulation bits
    IE2 |= UCA0RXIE;					// Enable Rx interrupt

    //Set up ports
    P3SEL |= BIT4 +BIT5; //Set UART ports to 3.4 and 3.5

    _EINT();	//Enable global interrupt

    while (1){
    	ADC10CTL0 &= ~ENC; //Turn off conversation
    	ADC10CTL1 &= ~(INCH0 +INCH1+ INCH2 + INCH3); //turn off all input

    	UCA0TXBUF= 255;

//Select A13 as the input channel and send accel X over
    	ADC10CTL1 = INCH0 + INCH2 + INCH3;
    	ADC10CTL0 |= ENC + ADC10SC; //ADC Enable and start conversation
    	while(ADC10CTL1 & ADC10BUSY); //wait until conversation is done
    	ADC10CTL0 &= ~ENC; //ADC Disable
    	xAccel = ADC10MEM >>2;
    	UCA0TXBUF= xAccel; //Send xAccel through UART

//Select A14 as the input channel and send accel Y over
        ADC10CTL1 = INCH1 + INCH2 + INCH3 ; //Select A14 as the input channel
		ADC10CTL0 |= ENC + ADC10SC; //ADC Enable and start conversation
		while(ADC10CTL1 & ADC10BUSY); //wait until conversation is done
		ADC10CTL0 &= ~ENC; //ADC Disable
		yAccel = ADC10MEM >>2;
		UCA0TXBUF= yAccel; //Send yAccel through UART

//Select A15 as the input channel and send accel Z over
	    ADC10CTL1 = INCH0 + INCH1 + INCH2 + INCH3; //Select A15 as the input channel
		ADC10CTL0 |= ENC + ADC10SC; //ADC Enable and start conversation
		while(ADC10CTL1 & ADC10BUSY); //wait until conversation is done
		ADC10CTL0 &= ~ENC; //ADC Disable
		zAccel = ADC10MEM >>2;
		UCA0TXBUF= zAccel; //Send zAccel through UART
    };

}
