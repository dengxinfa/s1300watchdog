/*---------------------------------------------------------------------*/
/* --- STC MCU International Limited ----------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.GXWMCU.com --------------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* ���Ҫ�ڳ�����ʹ�ô˴���,���ڳ�����ע��ʹ���˺꾧�Ƽ������ϼ�����   */
/*---------------------------------------------------------------------*/

#include    <reg52.h>

/*************	���س�������	**************/
sfr	AUXR = 0x8E;
sfr P3M1  = 0xB1;	//P3M1.n,P3M0.n 	=00--->Standard,	01--->push-pull
sfr P3M0  = 0xB2;	//					=10--->pure input,	11--->open drai
sfr WDT_CONTR = 0xC1; //���Ź��Ĵ���
sfr INT_CLKO  = 0x8f;

sbit reset = P3^3;
sbit feeddog_to_sgm706 = P3^2;
sbit feeddog_by_b1300 = P3^5;
//sbit P11 = P1^1;

#define     MAIN_Fosc		22118400L	//������ʱ��
#define		Timer0_Reload	(65536UL -(MAIN_Fosc / (12*50)))		//Timer 0 �ж�Ƶ��, 50��/��
#define 	Timer0_12T()				AUXR &= ~(1<<7)	//Timer0 clodk = fo/12	12��Ƶ,	default
#define		Timer0_16bitAutoReload()	TMOD &= ~0x03					//16λ�Զ���װ
#define		Timer0_AsTimer()			TMOD &= ~4		//ʱ��0������ʱ��
#define		Timer0_Load(n)		TH0 = (n) / 256,	TL0 = (n) % 256
#define 	Timer0_InterruptEnable()	ET0 = 1				//����Timer0�ж�.
#define 	Timer0_Run()	 			TR0 = 1				//����ʱ��0����

#define D_WDT_FLAG			(1<<7)
#define D_EN_WDT			(1<<5)
#define D_CLR_WDT			(1<<4)	//auto clear
#define D_IDLE_WDT			(1<<3)	//WDT counter when Idle
//#define D_WDT_SCALE_2		0
//#define D_WDT_SCALE_4		1
//#define D_WDT_SCALE_8		2		//T=393216*N/fo
//#define D_WDT_SCALE_16		3
//#define D_WDT_SCALE_32		4
//#define D_WDT_SCALE_64		5
#define D_WDT_SCALE_128		6
//#define D_WDT_SCALE_256		7
#define	WDT_reset(n)	WDT_CONTR = D_EN_WDT + D_CLR_WDT + D_IDLE_WDT + (n)		//��ʼ��WDT��ι��

#define     WatchDog_Of_B1300_S    60 //ʱ�䵥λs
#define     WatchDog_Of_STC15_MS    1600 //ʱ�䵥λms
/*************	���ر�������	**************/

unsigned long dog_count = 0;
unsigned long watchdog_of_b1300 = WatchDog_Of_B1300_S;
/*************	���غ�������	**************/



/*************  ��ʱ���� *****************/

void  delay_ms(unsigned long ms)
{
     unsigned int i,j=0,k=0;
	 i = MAIN_Fosc / 21000UL;
	 for(k=0;k<ms;k++)
        for(j=0;j<i;j++);
}

/****************��λ����**************************/
void reset_b1300(void)
{
		reset = 0;
		delay_ms(250);
		reset = 1;
}
/******************** IO���ú��� **************************/
void GPIO_config(void)
{
	P3M1 &= ~4;
	P3M0 |=  4;	 //p32�������
	P3M1 &= ~8;
	P3M0 |=  8;	 //p33�������
	P3M1 |=  32;
	P3M0 &= ~32;  //p35��������
	reset = 1;
}

/******************* ��ʱ��0�ж����� **********************/
void time0_interrupt_init(void)
{
	Timer0_12T(); //��ʱ��12��Ƶ
	Timer0_16bitAutoReload();//16λ�Զ�װ��
	Timer0_AsTimer();//ʱ��0������ʱ��
	Timer0_Load(Timer0_Reload);
	Timer0_InterruptEnable();
	Timer0_Run();
}

/******************* �ⲿ�ж�1���� *********************/
//void ext_interrupt1_init(void)
//{
//	IT1 = 1;
//	EX1 = 1;
//	PX1 = 1;//�ⲿ�ж�1������ȼ�
//}
/******************** ������ **************************/
void main(void)
{	
	
	GPIO_config();//GPIO��ʼ��
	//WDT_reset(D_WDT_SCALE_128);//���Ź���ʼ��
	time0_interrupt_init();//��ʱ����ʼ��
	//ext_interrupt1_init();//�ⲿ�жϳ�ʼ��
	
	INT_CLKO = 0x20;//�ⲿ�ж�4ʹ��'
	EA  = 1;//ʹ�����ж�	
	delay_ms(5000);
	while(1)
	{
		//WDT_reset(D_WDT_SCALE_128);//ι������оƬ��
				
		feeddog_to_sgm706 = 1;
		delay_ms(300);
		feeddog_to_sgm706 = 0;
		delay_ms(300);
	}
}


/********************** INT0 �ⲿ�жϺ��� ************************/
void exint3 (void) interrupt 11		//���ж�ʱ�Ѿ������־
{
 	
	dog_count = 0;
	INT_CLKO &= 0xdf;//�ֶ����� 
	INT_CLKO |= 0x20;//�����ж�4

}


/********************** Timer0 1ms�жϺ��� ************************/
void timer0 (void) interrupt 1
{
	if(dog_count > 500){
		reset_b1300();
		dog_count = 0;
	}
	else
		dog_count++;
}

