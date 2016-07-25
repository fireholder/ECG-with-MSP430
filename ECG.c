/***************************************************************
 *
 * 心电监测代码
 *
 ***************************************************************/

#include<msp430f5529.h>
#include<stdio.h>

#define false 0
#define true 1
#define BAUD 9600                               //邦特率为9600

void UART_init(void);
void ADC_init(unsigned char channel);
void ACLK_init();
void sendDataToProcessing(char symbol,int dat);
void delay(unsigned int n);
void UART_send(char dat);
char inputchar(unsigned char dat);

unsigned char PulsePin=0;
int fadeRate=0;

volatile unsigned int IBI=600;                //心率间期
volatile unsigned int BPM;                     //心率值
volatile unsigned int Signal;                  //存放心率原始数据
volatile int Pulse=false;                     //心率曲线低时为false
volatile int QS=false;                       //发现心跳时为真
volatile int rate[10];                        //存放心率间期
volatile unsigned long sampleCounter=0;       //决定心跳计时
volatile unsigned long lastBeatTime=0;         //用来找IBI
volatile int Peak=512;                         //心率曲线高峰值
volatile int Trough=512;                       //心率曲线低谷值
volatile int Thresh=512;
volatile int amp=100;                         //心率曲线振幅
volatile int firstBeat=true;
volatile int secondBeat=false;
// static unsigned char order=0;


void ACLK_init(void){                           //初始化默认定时器,ACLK,XT1

//	WDTCTL=EDTPW+WDTHOLD;                    //关闭看门狗
//	P1DIR |=BIT1;                            //P1.1 output

	P1DIR |=BIT0;                           //ACLK
	P1SEL |=BIT0;
//	P2SIR |=BIT2;
//	P2SEL= |BIT2;
	P5SEL |=BIT4+BIT5;                      //选择XT1

	UCSCTL6 &=~(XT1OFF);                    //打开XT1

	 do {
   	 UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG);
                                            // Clear XT2,XT1,DCO fault flags
   	 SFRIFG1 &= ~OFIFG;                      // Clear fault flags
 	 }while (SFRIFG1&OFIFG);                   // Test oscillator fault flag

 	 UCSCTL6 &= ~(XT1DRIVE_3);                 // Xtal is now stable, reduce drive strength

 	 UCSCTL4 |= SELA_0;                        // ACLK = LFTX1 (by default)
}

void UART_init(void){                            //UART初始化

	P3SEL=BIT3+BIT4;                         // P3.4,5 = USCI_A0 TXD/RXD

	UCA0CTL1 |= UCSWRST;                      // **Put state machine
	UCA0CTL1 |= UCSSEL_1;                     // CLK = ACLK
	UCA0BR0 = 0x03;                           // 32kHz/9600=3.41 (see User's Guide)
	UCA0BR1=0x00;
	UCA0MCTL = UCBRS_3+UCBRF_0;               // Modulation UCBRSx=3, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

	}


 void ADC_init(unsigned char channel){
 	P6SEL |= 0x01;                            // Enable A/D channel A0
	ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC; // Turn on ADC12, set sampling time
	ADC12CTL1 = ADC12SHP+ADC12CONSEQ_2;       // Use sampling timer, set mode
	ADC12IE = 0x01;                           // Enable ADC12IFG.0
  	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
  	ADC12CTL0 |= ADC12SC;                     // Start conversion
}

void sys_init(){

	WDTCTL=WDTPW+WDTHOLD;                    //关闭看门狗
	UART_init();
	ADC_init(PulsePin);
	ACLK_init();
}

void main(void){
	sys_init();                             //初始化
	LPM3;                                   //进入低功耗模式3
	_EINT();                               //打开总中断

	while(1){
//		sendDataToProcessing('S',Signal);
		if(UCA0RXBUF==4){
			sendDataToProcessing('S',Signal);  //发送原始心率数据电压
		if(QS==true){
			fadeRate=255;
		sendDataToProcessing('B',BPM);             //发送心率值
		sendDataToProcessing('Q',IBI);             //发送心率值
		QS=false;
		}
	}
		__delay_cycles(138);                  // Delay
	}
}

void sendDataToProcessing(char symbol,int dat)
{
	inputchar(symbol);
printf("%d\r\n",dat);
}

char inputchar(unsigned char dat)
{
	UCA0TXBUF=dat;                  //TODO 中断请求停止位清零
	return UCA0TXBUF;
}

unsigned int analogRead(unsigned char channel){
	unsigned int result;
        ADC12MCTL0 &=!ADC12IFG;
	result+=ADC12MEM0;
	result=result<<8;

	ADC12CTL0 = ADC12ON+ADC12SHT0_8+ADC12MSC; // Turn on ADC12, set sampling time
       	//TODO
  	ADC12CTL0 |= ADC12ENC;                    // Enable conversions
  	ADC12CTL0 |= ADC12SC;                     // Start conversion
  	return result;
}

#pragma vector=ADC12_VECTOR
__interrupt void timer(void)
{
	int n;
	unsigned char i;
	unsigned int runningTotal=0;        //保存前10个心率间期的值，并清0
	__bic_SR_register(GIE);            //关闭总中断
	                                   //TODO 重载定时器
	Signal=analogRead(PulsePin);        //读取心率值
	sampleCounter+=2;                    //计时总时间
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

				_EINT();                       //打开总中断
				return;

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

	if(n>2500){                               //如果2s没有心跳
		Thresh=512;
		Peak=512;
		Trough=512;
		lastBeatTime=sampleCounter;
		firstBeat=true;
		secondBeat=false;
}
	_EINT();
}
