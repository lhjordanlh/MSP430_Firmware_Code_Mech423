#include <msp430.h>


void main(void)
{
	volatile unsigned int i;            // volatile to prevent optimization

	  WDTCTL = WDTPW + WDTHOLD;             // Stop watchdog timer
	  P1DIR |= BIT7;                        // Set P1.0 to output direction

	  for (;;)
	  {

		P1OUT ^= BIT7;                      // Toggle P1.0 using exclusive-OR

		i = 10000;                          // SW Delay
		do i--;
		while (i != 0);
	  }

}
