#include "msp430x22x4.h"

/*Precessing Message Packets
 *
 * 1. We will provide a simple C# program to transmit messages to the serial port formatted as below.
 *      Use a command button to trigger sending the message.
 *      The two data bytes are the lower and upper 8 bytes from a single 16 bit number obtained from a textbox in the C# program.
 *      Any data bytes that contain the value 0xFF is changed to 0x00.
 *      The escape byte uses single bits to identify data bytes that have been changed so that they can be changed back by the MSP430 program.
           Start byte | Command byte | Data byte #1 | Data byte #2 | Escape byte
			   0xFF         0x01
 *	2. Modify the circular buffer code from Exercise 9 to handing the incoming message. Use the debugger to check that the message is being correctly generated.
 *	3. When a packet is available in the buffer, extract the data bytes and combine together into a single 16 bit number.
 *	4. Write code to change back any modified data bytes identified by the escape byte.
 *	5. Remove the processed bytes from the buffer.
 *	6. Use the received 16 bit number to set the frequency and/or the duty cycle of the square wave produced by Timer A in Exercise 7.
 *	7. Verify the output frequency is correct using an oscilloscope or using the code in Exercise 8.
 *	8. Add new commands to turn on and off LED3 from the C# program. For example, 0x02 = turn on LED3, 0x03 = turn off LED3, data bytes are irrelevant in this case.
 */

//Queue structure and its elements
volatile char queue[32];
volatile char packet[5];
volatile int startIndex = 0;
volatile int endIndex = 0;
volatile int size = 0;

#pragma vector=USCIAB0RX_VECTOR // defined in "msp430x22x4.h"
__interrupt void UART_RX(void) // interrupt on incoming data
{
  unsigned char rx_byte;

  rx_byte = UCA0RXBUF; // grab byte from RX buffer

  Enqueue(queue, rx_byte);

/*
  if(rx_byte=='\r'){
	  unsigned char tx_byte;
	  tx_byte = Dequeue(queue);

	  while((IFG2 & UCA0TXIFG) == 0); // wait for TX buffer to be clear
  	  UCA0TXBUF = tx_byte;
*/

  	  volatile unsigned int i=1000;            // volatile to prevent optimization;
  	  do(i--);
  	  	  while(i!=0);
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
/*	else{
		const char text[] = "Buffer  Overload";
		sendMsg(text);
*/	}
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
/*		const char text[] = "Buffer Under-run";
		sendMsg(text);
*/
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

    while (1){
    	char start=0; //Startbyte
    	char command=0; //commandbyte
    	char byte1=0;	//Byte1
    	char byte2=0; //Byte2
    	char escape=0; //EscapeByte
    	char byte=0; //combined byte
    	char temp=0;

    	if(size>4){
    		while(queue[startIndex] != 255)
    			temp=Dequeue(queue);

    		start=Dequeue(queue);
    		command=Dequeue(queue);
    		byte1=Dequeue(queue);
    		byte2=Dequeue(queue);
    		escape=Dequeue(queue);
    	}

    	if(escape==0)
    		temp=0;
    	else if(escape==1)
    		byte1= 0xFF;
    	else if(escape==2)
    		byte2= 0xFF;
    	else if (escape==3){
    		byte1= 0xFF;
    		byte2= 0xFF;
    	}


    	byte1= byte1<<8; //most significant one
    	byte= byte1|byte2; //Combine them;

    };

}
