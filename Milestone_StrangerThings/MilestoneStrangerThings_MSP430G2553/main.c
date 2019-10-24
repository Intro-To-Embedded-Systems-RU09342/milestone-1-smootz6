#include <msp430.h>

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;           //stop watchdog timer

    //assign outputs
    P1DIR |= BIT6;                      //set Red LED Direction Register to output
    P1SEL |= BIT6;                      //enable pwm for Red LED
    P2DIR |= BIT1 + BIT5;               //set Green(Bit1) and Blue(BIT5) LED Direction Register to output
    P2SEL |= BIT1 + BIT5;               //enable pwm for Green and Blue LED

    //set up timers
    TA0CTL |= TASSEL_2 + MC_1;          //set smclk, up mode
    TA0CCTL1 |= OUTMOD_7;               //set/reset output
    TA0CCR0 = 255;                      //pwm period
    TA0CCR1 = 0;                        //initialize red pwm
    TA1CTL |= TASSEL_2 + MC_1;          //set smclk, up mode
    TA1CCTL1 |= OUTMOD_7;               //set/reset output
    TA1CCTL2 |= OUTMOD_7;               //set/reset output
    TA1CCR0 = 255;                      //pwm period
    TA1CCR1 = 0;                        //initialize green pwm
    TA1CCR2 = 0;                        //initialize blue pwm

    //set up uart stuff
    P1SEL |= BIT1 + BIT2;               //P1.1 = RXD P1.2 = TXD
    P1SEL2 |= BIT1 + BIT2;              //P1.1 = RXD P1.2 = TXD
    UCA0CTL1 |= UCSSEL_2;               //smclk
    UCA0BR0 = 104;                      //1MHz 9600 baud rate
    UCA0BR1 = 0;                        //1MHz 9600 baud rate
    UCA0MCTL = UCBRS0;                  //modulation UBRSx = 1
    UCA0CTL1 &= ~UCSWRST;               //initialize usci state machine
    IE2 |= UCA0RXIE;                    //enable RX interrupt

    __bis_SR_register(LPM0_bits + GIE); //low power mode and interrupt enabled
}

int length = -1;                        //initialize length variable
int i = 0;                              //initialize counter variable

#pragma vector = USCIAB0RX_VECTOR       //interrupt routine
__interrupt void RXInterrupt(void) {
    while (!(IFG2 & UCA0TXIFG));        //wait until a byte is ready, is USCI_A0 TX buffer ready?
    switch (i) {                        //switch statement for looking at each byte depending on position
        case 0:                         //length byte
            length = UCA0RXBUF;         //set length var to input
            UCA0TXBUF = length - 3;     //set output to length - 3 (remove bytes for this led)
            break;
        case 1:                         //red input signal
            TA0CCR1 = UCA0RXBUF;        //set red duty cycle to its timer register
            break;
        case 2:                         //green input signal
            TA1CCR1 = UCA0RXBUF;        //set green duty cycle to its timer register
            break;
        case 3:                         //blue input signal
            TA1CCR2 = UCA0RXBUF;        //set blue duty cycle to its timer register
            break;
        default:
            UCA0TXBUF = UCA0RXBUF;      //if we aren't at the first byte or any of the R G B values, output the byte
    }
    if (++i == length) {                //once all the bytes are read
        i = 0;                          //reset counter back to zero
    }
}
