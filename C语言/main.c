/*---------------------------------------------------------------------*/
/* --- STC MCU International Limited ----------------------------------*/
/* --- STC 1T Series MCU Demo Programme -------------------------------*/
/* --- Mobile: (86)13922805190 ----------------------------------------*/
/* --- Fax: 86-0513-55012956,55012947,55012969 ------------------------*/
/* --- Tel: 86-0513-55012928,55012929,55012966 ------------------------*/
/* --- Web: www.GXWMCU.com --------------------------------------------*/
/* --- QQ:  800003751 -------------------------------------------------*/
/* 如果要在程序中使用此代码,请在程序中注明使用了宏晶科技的资料及程序   */
/*---------------------------------------------------------------------*/

#include    <reg52.h>

/*************	本地常量声明	**************/
sfr	AUXR = 0x8E;
sfr P3M1  = 0xB1;	//P3M1.n,P3M0.n 	=00--->Standard,	01--->push-pull
sfr P3M0  = 0xB2;	//					=10--->pure input,	11--->open drai
sfr WDT_CONTR = 0xC1; //看门狗寄存器
sfr INT_CLKO  = 0x8f;

sbit reset = P3^3;
sbit feeddog_to_sgm706 = P3^2;
sbit feeddog_by_b1300 = P3^5;
//sbit P11 = P1^1;

#define     MAIN_Fosc		22118400L	//定义主时钟
#define		Timer0_Reload	(65536UL -(MAIN_Fosc / (12*50)))		//Timer 0 中断频率, 50次/秒
#define 	Timer0_12T()				AUXR &= ~(1<<7)	//Timer0 clodk = fo/12	12分频,	default
#define		Timer0_16bitAutoReload()	TMOD &= ~0x03					//16位自动重装
#define		Timer0_AsTimer()			TMOD &= ~4		//时器0用做定时器
#define		Timer0_Load(n)		TH0 = (n) / 256,	TL0 = (n) % 256
#define 	Timer0_InterruptEnable()	ET0 = 1				//允许Timer0中断.
#define 	Timer0_Run()	 			TR0 = 1				//允许定时器0计数

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
#define	WDT_reset(n)	WDT_CONTR = D_EN_WDT + D_CLR_WDT + D_IDLE_WDT + (n)		//初始化WDT，喂狗

#define     WatchDog_Of_B1300_S    60 //时间单位s
#define     WatchDog_Of_STC15_MS    1600 //时间单位ms
/*************	本地变量声明	**************/

unsigned long dog_count = 0;
unsigned long watchdog_of_b1300 = WatchDog_Of_B1300_S;
/*************	本地函数声明	**************/



/*************  延时函数 *****************/

void  delay_ms(unsigned long ms)
{
     unsigned int i,j=0,k=0;
	 i = MAIN_Fosc / 21000UL;
	 for(k=0;k<ms;k++)
        for(j=0;j<i;j++);
}

/****************复位函数**************************/
void reset_b1300(void)
{
		reset = 0;
		delay_ms(250);
		reset = 1;
}
/******************** IO配置函数 **************************/
void GPIO_config(void)
{
	P3M1 &= ~4;
	P3M0 |=  4;	 //p32推挽输出
	P3M1 &= ~8;
	P3M0 |=  8;	 //p33推挽输出
	P3M1 |=  32;
	P3M0 &= ~32;  //p35浮空输入
	reset = 1;
}

/******************* 定时器0中断配置 **********************/
void time0_interrupt_init(void)
{
	Timer0_12T(); //定时器12分频
	Timer0_16bitAutoReload();//16位自动装载
	Timer0_AsTimer();//时器0用做定时器
	Timer0_Load(Timer0_Reload);
	Timer0_InterruptEnable();
	Timer0_Run();
}

/******************* 外部中断1配置 *********************/
//void ext_interrupt1_init(void)
//{
//	IT1 = 1;
//	EX1 = 1;
//	PX1 = 1;//外部中断1最高优先级
//}
/******************** 主函数 **************************/
void main(void)
{	
	
	GPIO_config();//GPIO初始化
	//WDT_reset(D_WDT_SCALE_128);//看门狗初始化
	time0_interrupt_init();//定时器初始化
	//ext_interrupt1_init();//外部中断初始化
	
	INT_CLKO = 0x20;//外部中断4使能'
	EA  = 1;//使能总中断	
	delay_ms(5000);
	while(1)
	{
		//WDT_reset(D_WDT_SCALE_128);//喂狗（本芯片）
				
		feeddog_to_sgm706 = 1;
		delay_ms(300);
		feeddog_to_sgm706 = 0;
		delay_ms(300);
	}
}


/********************** INT0 外部中断函数 ************************/
void exint3 (void) interrupt 11		//进中断时已经清除标志
{
 	
	dog_count = 0;
	INT_CLKO &= 0xdf;//手动清零 
	INT_CLKO |= 0x20;//开启中断4

}


/********************** Timer0 1ms中断函数 ************************/
void timer0 (void) interrupt 1
{
	if(dog_count > 500){
		reset_b1300();
		dog_count = 0;
	}
	else
		dog_count++;
}

