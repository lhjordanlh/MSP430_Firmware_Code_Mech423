#include "msp430fr5739.h"

//Initializing Queue Variables
unsigned int dataQueue[32];
volatile unsigned int startIndex = 0;
volatile unsigned int stopIndex = 0;
volatile unsigned int full = 0;

//Initializing Data Received Variables
volatile unsigned int byte1 = 0;
volatile unsigned int byte2 = 0;
volatile unsigned int startByte = 255;
volatile unsigned int result = 0;

//Motor Drive Variables
volatile unsigned int currentDir = 1;

//Function Declarations
void EnQueue(int data);
int DeQueue();
int GetQueueSize();
void ProcessData();

#pragma vector=USCI_A1_VECTOR
__interrupt void UART_RX(void){
//UART receive ISR - get byte and enqueue
	unsigned char rx_byte;
	rx_byte = UCA1RXBUF;
	EnQueue(rx_byte);
}

int main(void){
	WDTCTL = WDTPW + WDTHOLD;			//Stop Watchdog Timer

	CSCTL0 = CSKEY;						//Access Clock Register
	CSCTL1 |= DCORSEL + DCOFSEL_3;		//Set DCO Frequency to Max Settings (24MHz)
	CSCTL2 = SELA_3 + SELS_3 + SELM_3;	//Set DCO = ACLK = MCLK = SMCLK
	CSCTL3 = DIVA_0 + DIVS_0 + DIVM_0;	//Set all dividers to 0

	UCA1CTLW0 |= UCSWRST;				//Access UART Register
	UCA1CTLW0 |= UCSSEL_2;				//Set SMCLK as UART source
	UCA1BR0 = 2500 & 0xFF;				//Set 9600baudrate (24MHz/9600 = 2500)
	UCA1BR1 = 2500 >> 8;
	P2SEL1 |= BIT5 + BIT6;				//Set UART pins
	P2SEL0 &= ~(BIT5 + BIT6);
	UCA1CTLW0 &= ~UCSWRST;				//Close Access to UART Register

	TB0CTL = TBSSEL_2 + ID_0 + MC_2;	//Set Timer B0 with SMCLK and divider to 0 and Continuous Mode
	TB0CCTL1 |= OUTMOD_7;				//Set TB0.1 to reset/set output mode
	TB0CCTL2 |= OUTMOD_7;				//Set TB0.2 to reset/set output mode
	TB0CCR0 = 0xFFFF;					//Set TB0CCR0 to max count
	P1DIR |= BIT4 + BIT5;				//Set ports 1.4 and 1.5 to OUT mode
	P1OUT &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to output 0
	P1SEL1 &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to get signal from TB0.1 and TB0.2 respectively
	P1SEL0 |= (BIT4 + BIT5);
	TB0CCR1 = 0;						//Set TB0CCRn to 0
	TB0CCR2 = 0;

	UCA1IE |= UCRXIE;					//UART interrupt enable
	_EINT();							//Global interrupt enable
	while(1){
		ProcessData();					//Process data
	};
}

void ProcessData(){
	while(GetQueueSize() >= 3){
		if(DeQueue() == startByte){
			byte1 = DeQueue();
			byte2 = DeQueue();

			if(byte1 != currentDir){
				currentDir = byte1;
				TB0CCR1 = 0;
				TB0CCR2 = 0;
			}

			if(byte1 == 1){
				TB0CCR1 = (byte2/100.0)*(0xFFFF);
			}else if(byte1 == 2){
				TB0CCR2 = (byte2/100.0)*(0xFFFF);
			}
		}
	}
}

void EnQueue(int data){
//Enqueues data
	stopIndex = (stopIndex+1)%(sizeof(dataQueue)/sizeof(int));		//Update stop index
	dataQueue[stopIndex] = data;									//Enqueue data in new stop index location
	if(startIndex == stopIndex)										//Check if stop index and start index are the same
		full = 1;													//If true, set full variable to true
}

int DeQueue(){
//Dequeue data
	startIndex = (startIndex+1)%(sizeof(dataQueue)/sizeof(int));	//Update start index
	int dat = dataQueue[startIndex];								//Get char data at new start index
	if(full == 1){													//Check if full is true
		full = 0;													//If true, turn false
	}
	return dat;														//Return dequeued data
}

int GetQueueSize(){
//Get queue size
	int size;
	if(stopIndex < startIndex){
		size = sizeof(dataQueue)/sizeof(int) - startIndex + stopIndex;
	}else if(stopIndex == startIndex){
		if(full==1){
			size = sizeof(dataQueue)/sizeof(int);
		}else{
			size = 0;
		}
	}else{
		size = stopIndex - startIndex;
	}
	return size;
}
