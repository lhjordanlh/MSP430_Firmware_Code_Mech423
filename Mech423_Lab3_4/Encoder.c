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

//Parameter to calculate the speeds
volatile unsigned int forward_1 = 0;
volatile unsigned int forward_2 = 0;
volatile unsigned int state1 = 0;
volatile unsigned int periodF= 0;
volatile unsigned int backward_1 = 0;
volatile unsigned int backward_2 = 0;
volatile unsigned int state2 = 0;
volatile unsigned int periodB= 0;

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

#pragma vector= TIMER1_A0_VECTOR
__interrupt void timerA0_Interrupt(void){
	if(state1 == 0){
		forward_1 = TA0CCR2;
		state1 = 1;
	}
	else if(state1 ==1){
		forward_2 = TA0CCR2;
		state1 = 0;
	}

	periodF= forward_2 - forward_1; //Calculate the period for direction forward

	UCA1TXBUF= periodF;

	volatile unsigned int i;            // volatile to prevent optimization
	i = 10000;                          // SW Delay
 		do i--;
		while (i != 0);

	TBCCTL1 &= ~CCIE;
}

#pragma vector= TIMER1_A1_VECTOR
__interrupt void timerA1_Interrupt(void){
	if(state2 == 0){
		forward_1 = TA1CCR1;
		state2 = 1;
	}
	else if(state2 ==1){
		forward_2 = TA1CCR1;
		state2 = 0;
	}
	periodB= backward_2 - backward_1; //Calculate the period for direction backward

	UCA1TXBUF= periodF;

    volatile unsigned int i;            // volatile to prevent optimization
	i = 10000;                          // SW Delay
 		do i--;
		while (i != 0);

	TBCCTL1 &= ~CCIE;
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

	TB0CTL = TBSSEL_2 + ID_0 + MC_1;	//Set Timer B0 with SMCLK and divider to 0 and Up Mode
	TB0CCTL1 |= OUTMOD_7;				//Set TB0.1 to reset/set output mode
	TB0CCTL2 |= OUTMOD_7;				//Set TB0.2 to reset/set output mode
	//TB0CCR0 = 0x5C30;					//Set TB0CCR0 to max count to get 1kHz - 23.6MHz/1kHz ~ 23.6kHz
	TB0CCR0 = 0xFFFF;
	P1DIR |= BIT4 + BIT5;				//Set ports 1.4 and 1.5 to OUT mode
	P1OUT &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to output 0
	P1SEL1 &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to get signal from TB0.1 and TB0.2 respectively
	P1SEL0 |= (BIT4 + BIT5);
	TB0CCR1 = 0;						//Set TB0CCRn to 0
	TB0CCR2 = 0;

	TB1CTL = TBSSEL_2 + ID_0 + MC_1;	//Set Timer B0 with SMCLK and divider to 0 and Up Mode
	TB1CCTL1 |= OUTMOD_7;				//Set TB0.1 to reset/set output mode
	TB1CCTL2 |= OUTMOD_7;				//Set TB0.2 to reset/set output mode
	//TB1CCR0 = 0x5C30;					//Set TB0CCR0 to max count to get 1kHz - 23.6MHz/1kHz ~ 23.6kHz
	TB1CCR0 = 0xFFFF;
	P3DIR |= BIT4 + BIT5;				//Set ports 1.4 and 1.5 to OUT mode
	P3OUT &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to output 0
	P3SEL1 &= ~(BIT4 + BIT5);			//Set ports 1.4 and 1.5 to get signal from TB0.1 and TB0.2 respectively
	P3SEL0 |= (BIT4 + BIT5);
	TB1CCR1 = 0;						//Set TB0CCRn to 0
	TB1CCR2 = 0;

//Set up Timer A0 in Capture mode to record the forward speed
    P1DIR &= ~BIT1; //Set Port 1.1 to input
    P1SEL1 |= BIT1; //Set Port 1.1 -> TimerA0.CCI2A
    TA0CTL |= MC_2 + TASSEL_2; //Set TimerA0 in continuos mode & let TimerA0 use SMCLK
    TA0CCTL2 |= SCS + CAP + CM_1 + CCIE; // Set timer A0 to sync, capture mode and capture on rising edge and enable interrupt

//Set up Timer A1 in Capture mode to record the backward speed
     P1DIR &= ~BIT2; //Set Port 1.2 to input
     P1SEL1 |= BIT2; //Set Port 1.2 -> TimerA1.CCI1A
     TA1CTL |= MC_2 + TASSEL_2; //Set TimerA1 in continuos mode & let TimerA1 use SMCLK
     TA1CCTL1 |= SCS + CAP + CM_1 + CCIE; // Set timer A1 to sync, capture mode and capture on rising edge and enable interrupt
	
	UCA1IE |= UCRXIE;					//UART interrupt enable
	_EINT();							//Global interrupt enable
	while(1){
		ProcessData();					//Process data
	};
}

void ProcessData(){
	while(GetQueueSize() >= 3){
		if(DeQueue() == startByte){
//			byte1 = DeQueue();
//			byte2 = DeQueue();
			byte1 = 0;
			byte2 = 0;

			if(byte1 != currentDir){
				currentDir = byte1;
				TB0CCR1 = 0;
				TB0CCR2 = 0;
				TB1CCR1 = 0;
				TB1CCR2 = 0;
			}

			if(byte1 == 1){
				TB0CCR1 = (byte2/100.0)*(TB0CCR0);
				TB1CCR1 = TB0CCR1;
			}else if(byte1 == 2){
				TB0CCR2 = (byte2/100.0)*(TB0CCR0);
				TB1CCR2 = TB0CCR2;
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
