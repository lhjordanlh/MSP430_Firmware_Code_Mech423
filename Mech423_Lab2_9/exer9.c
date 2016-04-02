#include "msp430x22x4.h"

/*
 * Cicular Buffer
 * 1. Setup a queue (Circular Buffer). The queue data should be stored in an array of 32 bytes
 * 		Create functions: EnQueue, DeQueue, and GetQueueSize. Manage valid data in the queue
 * 		using StartIndex and StopIndex variables.
 * 2. Set up UART interrupts such that each new Byte received by the UART is added
 * 		to the end of the buffer
 * 3. Connect to the Gumstick using PuTTY. Type single characters into the terminal and then
 * 		use the debugger to confirm that they are being correctly stored in the buffer
 * 4. Add code such that whenever a carriage return (ASCII code 13) is received,
 * 		a single character is removed from the top of the buffer. Transmitt the single character
 * 		back to the PC when it is removed from the buffer. Check using PuTTY.
 * 5. Add error-checking code to the queue to prevent buffer over-run (trying to enqueue when
 * 		the queue already has 32 elements) and buffer under-run (trying to deququ when the queue
 * 		has zero elements) Send a short message to the serial port when these errors occur.
 * 6. Use PuTTY to check the correct operation of the circular buffer.
 */
/*
volatile unsigned int xAccel = 0;
volatile unsigned int yAccel = 0;
volatile unsigned int zAccel = 0;
*/

//Queue structure and its elements
volatile char queue[32];
volatile int startIndex = 0;
volatile int endIndex = 0;
volatile int size = 0;

/*
#pragma vector= ADC10_VECTOR
__interrupt void ADC10_interrupt(void){

}
*/

#pragma vector=USCIAB0RX_VECTOR // defined in "msp430x22x4.h"
__interrupt void UART_RX(void) // interrupt on incoming data
{
  unsigned char rx_byte;

  rx_byte = UCA0RXBUF; // grab byte from RX buffer

  if(rx_byte!='\r')
	  Enqueue(queue, rx_byte);

  if(rx_byte=='\r'){
	  unsigned char tx_byte;
	  tx_byte = Dequeue(queue);

	  while((IFG2 & UCA0TXIFG) == 0); // wait for TX buffer to be clear
  	  UCA0TXBUF = tx_byte;
  	  volatile unsigned int i=1000;            // volatile to prevent optimization;

  	  do(i--);
  	  	  while(i!=0);
  }
/*
  while((IFG2 & UCA0TXIFG) == 0); // wait for TX buffer to be clear

  UCA0TXBUF = rx_byte;	// Echo the byte back
  volatile unsigned int i=1000;            // volatile to prevent optimization;

  do(i--);
  	  while(i!=0);

  UCA0TXBUF = rx_byte+1;	// Echo the next byte back

  }
  */
}

void sendMsg(char string[])
{
	int length = 16;
	int temp=0;

  while(temp<length)
  {
    // Wait for TX buffer to be ready for new data
    while((IFG2 & UCA0TXIFG) == 0);

    // Push data to TX buffer
    UCA0TXBUF = string[temp];

    // Update variables
    temp++;
  }
}

void Enqueue(char queue[], char element) {
	if(size<32){
		queue[endIndex]= element;
		size++;
		if(endIndex<=31)
			endIndex++;
		else
			endIndex=0;
	}
	else{
		const char text[] = "Buffer  Overload";
		sendMsg(text);
	}
}

char Dequeue(char queue[]) {
	if(size>0){
		char temp= queue[startIndex];

		queue[startIndex]= '0';
		size--;
		if(startIndex<31)
			startIndex++;
		else
			startIndex=0;

		return temp;
	}
	else{
		const char text[] = "Buffer Under-run";
		sendMsg(text);
		return '0';
	}
}

int GetQueueSize(int queue[]){
	return size;
}


int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

//set up timer
	BCSCTL1 = XT2OFF + XTS;				// XT2 off, XT1 high freq
	BCSCTL2 = SELM0 + SELM1 + SELS;		// XT1 -> MCLK, XT1 -> SMCLK
	BCSCTL3 = LFXT1S1;					// XT1 freq 3-16 MHz

	volatile unsigned int i;            // volatile to prevent optimization

	P1DIR |= BIT7; //Set pin1.1 to output

//Set up the UART
	P3SEL = BIT4 | BIT5;				// Set up ports 3.4 and 3.5 as UART ports

	UCA0CTL1 = UCSSEL1;					// Use SMCLK for UART
	UCA0BR0 = 1666 & 0xFF; 				// Set baud rate
	UCA0BR1 = 1666 >> 8;
	UCA0MCTL = UCBRS_6;					// Set modulation bits
	IE2 |= UCA0RXIE;					// Enable Rx interrupt

	_EINT();		// Global interrupt enable

}
