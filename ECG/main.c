#include <msp430f5529.h>
#include "ADS1x9x.h"
#include "types.h"

extern unsigned char ADS1x9xRegVal[16];
unsigned char ECGRxPacket[64];
/*
 * main.c
 */

void Init_UART(void){
	P3SEL |=BIT3+BIT4;
	UCA0CTL1 |=UCSWRST;               //put state machine
	UCA0CTL1=UCSSEL_1;                //ACLK
	UCA0BR0=0x03;                     //9600
	UCA0BR1=0x00;
	UCA0MCTL=UCBRS_3+UCBRF_0;

}

void Init_Clock(void){
	P5SEL |= 0x0C;
	// Use the REFO oscillator as the FLL reference, and also for ACLK
	UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
	UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__XT2CLK);
}
void Init_StartUp(void)
{
    __disable_interrupt();               // Disable global interrupts

    Init_Clock();
    Init_UART();

    __enable_interrupt();                // enable global interrupts
}

void UARTSendByte(unsigned char data){
	while((UCA0IFG & UCTXIFG)==0);
	UCA0TXBUF=data;
}

void UARTSendString(unsigned char *str){
	while(*str !='\0'){
		UARTSendByte(*str++);
	//	delay_ms(2);
		__delay_cycles(1);
	}
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
    // 	unsigned char ADS1x9xegs[27];
    	volatile unsigned short i, j;
    //unsigned char Flash_data_buf[2048];

        WDTCTL = WDTPW + WDTHOLD;	    // Stop watchdog timer
        Init_StartUp();                 //initialize device



    	ADS1x9x_PowerOn_Init();
    	//Filter_Option = 3;						// Default filter option is 40Hz LowPass
       	Start_Read_Data_Continuous();			//RDATAC command
    	for ( i =0; i < 10000; i++);
    	for ( i =0; i < 10000; i++);
    	for ( i =0; i < 10000; i++);

        ADS1x9x_Disable_Start();
    	ADS1x9x_Enable_Start();

    	Set_ADS1x9x_Chip_Enable();					// CS = 0
		__delay_cycles(300);
		Clear_ADS1x9x_Chip_Enable();				// CS = 1

		__delay_cycles(30000);
		__delay_cycles(30000);

		Set_ADS1x9x_Chip_Enable();				// CS =0
		__delay_cycles(300);
		Start_Read_Data_Continuous();			//RDATAC command
		__delay_cycles(300);
		Enable_ADS1x9x_DRDY_Interrupt();		// Enable DRDY interrupt
		ADS1x9x_Enable_Start();				// Enable START (SET START to high)
//	    UCA0IFG |= UCTXIFG; // 设置中断标志，进入发送中断程序


	return 0;
}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCII_A0_ISR(void){
	switch (__even_in_range(UCA0IV,4)){
	case 0:
		break;
	case 2:
		while(!(UCA0IFG & UCTXIFG));
		//   ADS1x9x_Reg_Write (ECGRxPacket[2], ECGRxPacket[3]);
	    //   ADS1x9xRegVal[ECGRxPacket[2]] = ECGRxPacket[3];

/*	       ECGRxPacket[3] = ADS1x9x_Reg_Read (ECGRxPacket[2]);
		   ADS1x9xRegVal[ECGRxPacket[2]] = ECGRxPacket[3];

		   UCA0TXBUF=ECGRxPacket[2];
		   while(!(UCA0IFG & UCTXIFG));
		   UCA0TXBUF=ECGRxPacket[3];
		   __delay_cycles(300);
		   */
		UCA0TXBUF=UCA0RXBUF;
		   break;
	case 4:
		break;
	default:
		break;

	}
}
