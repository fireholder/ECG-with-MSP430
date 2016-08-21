/***************************************************************
 *
 * �ĵ������
 *
 * MCU:MSP430F5529
 * Baudrate��9600
 * ADC PIN��P6.0
 * UART��P3.3  P3.4
 *
 *
 *                MSP430F552x
 *             -----------------
 *         /|\|                 |
 *          | |                 |
 *          --|RST              |
 *            |     P3.3/UCA0TXD|----------->
 *     Vin -->|P6.0/CB0/A0      | 9600 - 8N1
 *                  P3.4/UCA0RXD <-----------
 *
 * Buit with CCSv6
 * by X.
 ***************************************************************/

#include<msp430f5529.h>
#include<stdio.h>

#define false 0
#define true 1
#define CPUCLK  			8000000		// 8MHZ       ---һ������0.125us
#define	 delay_us(us)		__delay_cycles((long)(CPUCLK*(double)us/1000000.0))
#define delay_ms(ms)	__delay_cycles((long)(CPUCLK*(double)ms/1000.0))


void UART_init(void);
void ADC_init();
void timer_init();
void sendData(char symbol,int dat);
void UARTSendString(unsigned char *str);
void UARTSendByte(unsigned char data);
char inputchar(unsigned char dat);
int calculate();
unsigned char PulsePin=0;
int fadeRate=0;

unsigned char txBuff[4];

volatile unsigned int IBI=600;                //���ʼ���
volatile unsigned int BPM;                     //����ֵ
volatile unsigned int Signal;                  //�������ԭʼ����
volatile int Pulse=false;                     //�������ߵ�ʱΪfalse
volatile int QS=false;                       //��������ʱΪ��
volatile int rate[10];                        //������ʼ���
volatile unsigned long sampleCounter=0;       //����������ʱ
volatile unsigned long lastBeatTime=0;         //������IBI
volatile int Peak=512;                         //�������߸߷�ֵ
volatile int Trough=512;                       //�������ߵ͹�ֵ
volatile int Thresh=512;
volatile int amp=100;                         //�����������
volatile int firstBeat=true;
volatile int secondBeat=false;
// static unsigned char order=0;


void timer_init(void){                           //��ʼ��Ĭ�϶�ʱ��,ACLK,XT1

/*
	P1DIR |=BIT0;                           //ACLK
	P1SEL |=BIT0;
	P5SEL |=BIT4+BIT5;                      //ѡ��XT1

	UCSCTL6 &=~(XT1OFF);                    //��XT1

	 do {
   	 UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                            // Clear XT2,XT1,DCO fault flags
   	 SFRIFG1 &= ~OFIFG;                      // Clear fault flags
 	 }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

 	 UCSCTL6 &= ~(XT1DRIVE_3);                 // Xtal is now stable, reduce drive strength

 	 UCSCTL4 |= SELA_0;                        // ACLK = LFTX1 (by default)
*/
 	 UCSCTL3 = SELREF_2;                       // Set DCO FLL reference = REFO
       	 UCSCTL4 |= SELA_2;                        // Set ACLK = REFO
 	 __bis_SR_register(SCG0);                  // Disable the FLL control loop
	 UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
	 UCSCTL1 = DCORSEL_7;                      // Select DCO range 50MHz operation
	 UCSCTL2 = FLLD_0 + 487;                   // Set DCO Multiplier for 25MHz
	 __bic_SR_register(SCG0);                 //enable the FLL control loop
	 __delay_cycles(782000);                  // Loop until XT1,XT2 & DCO stabilizes - In this case only DCO has to stabilize

	do{
		UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
				                                            // Clear XT2,XT1,DCO fault flags
		SFRIFG1 &= ~OFIFG;                      // Clear fault flags
	}while (SFRIFG1&OFIFG);                   // Test oscillator fault flag
    // USART-0 , 9600

}

void UART_init(void){                            //UART��ʼ��

	P3SEL=BIT3+BIT4;                         // P3.3,4 = USCI_A0 TXD/RXD

	UCA0CTL1 |= UCSWRST;                      // **Put state machine
	UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
	UCA0BR1=0x00;
	UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

	}


 void ADC_init(){
 	P6SEL |= 0x01;                            // Enable A/D channel A0
//	ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC; // Turn on ADC12, set sampling time
	ADC12CTL0 =ADC12ON +ADC12SHT02;
//	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode
	ADC12CTL1=ADC12SHP+ADC12CONSEQ_0;
	ADC12IE = 0x01;                           // Enable ADC12IFG.0
  	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
  	ADC12CTL0 |= ADC12SC;                     // Start conversion
}

void sys_init(){

	WDTCTL=WDTPW+WDTHOLD;                    //�رտ��Ź�
	timer_init();
	__delay_cycles(782000);
	UART_init();
	ADC_init();
}

void main(void){
	sys_init();                             //��ʼ��
	//LPM3;                                   //����͹���ģʽ3
//	_EINT();                               //�����ж�

	TA0CTL = TACLR + TAIE;			 				//�����жϲ�����
	TA0CTL = TASSEL__ACLK + MC__UP +  TACLR;	//ѡ��SCLK32.768KHZ��Ϊʱ�ӣ�ѡ������ģʽ���������ж�
	//�ж�Ƶ��100HZ
	TA0CCTL0 = CCIE;                         			 // CCR0 interrupt enabled
	TA0CCR0 = 327;
	__enable_interrupt();

	while(1){
		;
		}


	//	__delay_cycles(138);                  // Delay
	}


#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_RX(void){            //UART�ж�

	switch(__even_in_range(UCA0IV,4)){
	case 0:break;
	case 2:

//	while(!(UCA0IFG&UCTXIFG));
//	UCA0TXBUF=UCA0RXBUF;
	break;
	case 4:break;
	default:break;
	}
}


void sendData(char symbol,int dat)
{
	inputchar(symbol);
	printf("%x\n",dat);
}

char inputchar(unsigned char data)
{
	if(data=='\n'){
		while(UCA0STAT & UCBUSY);
		UCA0TXBUF='\r';
		}
	while(UCA0STAT & UCBUSY);

	UCA0TXBUF=data;
	return UCA0TXBUF;
}

void UARTSendByte(unsigned char data){
	while((UCA0IFG & UCTXIFG)==0);
	UCA0TXBUF=data;
}

void UARTSendString(unsigned char *str){
	while(*str !='\0'){
		UARTSendByte(*str++);
		delay_ms(2);
	//	__delay_cycles(1);
	}
}


#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerA0(void){
	int i;
	ADC12CTL0 &=~ADC12SC;
	ADC12CTL0 |=ADC12SC;
	Signal=ADC12MEM0;
//	while(!(UCA0IFG & UCTXIFG));
//	UCA0RXBUF=Signal;
	ECG_ProcessCurrSample(&Signal,&Signal);
	calculate();
	txBuff[0]='s';
	txBuff[1]=Signal;
	txBuff[2]='b';
	txBuff[3]=BPM;
//	txBuff[4]='\n';
	for(i=0;i<4;i++){
		while(!(UCA0IFG & UCTXIFG));
		inputchar(txBuff[i]);
	}

}


#pragma vector=ADC12_VECTOR
__interrupt void timer(void)
{
	ADC12IFG &=~BIT0;
}



int calculate(void){

	int n;
	unsigned char i;
	unsigned int runningTotal=0;        //����ǰ10�����ʼ��ڵ�ֵ������0
	__bic_SR_register(GIE);            //�ر����ж�
	
	sampleCounter+=2;                    //��ʱ��ʱ��
	n=sampleCounter-lastBeatTime;

	if(Signal<Thresh && N>(IBI/5)*3){
		if(Signal<Trough){
			Trough=Signal;
		}
	}

	if(Signal>Thresh && Signal>Peak){
		Peak=Signal;

	}

	if(n>250){
		if((Signal>Thresh)&&(Pulse==false)&&(n>(IBI/5)*3)){
			Pulse=true;
			IBI=sampleCounter-lastBeatTime;
			lastBeatTime=sampleCounter;

			if(secondBeat){
				secondBeat=false;
				for(i=0;i<=9;i++){
					rate[i]=IBI;
				}
			}

			if(firstBeat){
				firstBeat=false;
				secondBeat=true;

				_EINT();                       //�����ж�
				return 0;

			}

			for (i=0;i<=8;i++){
				rate[i]=rate[i+1];
				runningTotal+=rate[i];

			}

			rate[9]=IBI;
			runningTotal+=rate[9];
			runningTotal /=10;
			BPM=60000/runningTotal;
			if(BPM>200)
				BPM=200;
			if(BPM<30)
				BPM=30;
			QS=true;

		}
	}

	if(Signal<Thresh && Pulse==true){
		Pulse=false;
		amp=Peak-Trough;
		Thresh=amp/2 + Trough;
		Peak=Thresh;
		Trough=Thresh;

	}

	if(n>2500){                               //���2sû������
		Thresh=512;
		Peak=512;
		Trough=512;
		lastBeatTime=sampleCounter;
		firstBeat=true;
		secondBeat=false;
}
	_EINT();
}

