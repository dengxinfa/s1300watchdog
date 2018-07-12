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
/*************	功能说明	**************

程序使用P4.7 P4.6 P1.6 P1.7 来演示跑马灯，输出低驱动。

******************************************/

/*************	本地常量声明	**************/
sfr	AUXR = 0x8E;
sfr P3M1  = 0xB1;	//P3M1.n,P3M0.n 	=00--->Standard,	01--->push-pull
sfr P3M0  = 0xB2;	//					=10--->pure input,	11--->open drai

sbit reset = P3^5;
sbit feeddog_to_sgm706 = P3^2;
sbit feeddog_by_b1300 = P3^3;

#define     MAIN_Fosc		22118400L	//定义主时钟
#define		Timer0_Reload	(65536UL -(MAIN_Fosc / (12*50)))		//Timer 0 中断频率, 50次/秒
#define 	Timer0_12T()				AUXR &= ~(1<<7)	//Timer0 clodk = fo/12	12分频,	default
#define		Timer0_16bitAutoReload()	TMOD &= ~0x03					//16位自动重装
#define		Timer0_AsTimer()			TMOD &= ~4		//时器0用做定时器
#define		Timer0_Load(n)		TH0 = (n) / 256,	TL0 = (n) % 256
#define 	Timer0_InterruptEnable()	ET0 = 1				//允许Timer1中断.
#define 	Timer0_Run()	 			TR0 = 1				//允许定时器0计数

#define		watchdog_of_b1300_first_s  180 //第一次启动喂狗时间
#define     watchdog_of_b1300_s    30 //时间单位s

/*************	本地变量声明	**************/

unsigned int dog_time = watchdog_of_b1300_first_s;
unsigned int dog_count = 0;
int int_number = 0;
int dog_flags = 0;
/*************	本地函数声明	**************/



/*************  延时函数 *****************/

void  delay_ms(unsigned int ms)
{
     unsigned int i,j=0,k=0;
	 i = MAIN_Fosc / 21000UL;
	 for(k=0;k<ms;k++)
        for(j=0;j<i;j++);
}

/****************复位函数**************************
void reset_b1300(void)
{
		reset = 0;
		delay_ms(250);
		reset = 1;
}*/

/******************** IO配置函数 **************************
void GPIO_config(void)
{
	P3M1 &= ~4;
	P3M0 |=  4;	 //推挽输出
	P3M1 |=  8;
	P3M0 &=  8;	 //浮空输入
	P3M1 &= ~32;
	P3M0 |=  32;	 //推挽输出
	reset = 1;
}*/

/******************* 定时器0中断配置 **********************
void time0_interrupt_init(void)
{
	Timer0_12T(); //定时器12分频
	Timer0_16bitAutoReload();//16位自动装载
	Timer0_AsTimer();//时器0用做定时器
	Timer0_Load(Timer0_Reload);
	Timer0_InterruptEnable();
	Timer0_Run();
}*/

/******************* 外部中断1配置 *********************
void ext_interrupt1_init(void)
{
	IT1 = 1;
	EX1 = 1;
	PX1 = 1;//外部中断1最高优先级
}*/
/******************** 主函数 **************************/
void main(void)
{	
	
	//GPIO_config();//GPIO初始化
	//time0_interrupt_init();//定时器初始化
	//ext_interrupt1_init();//外部中断初始化
	/******************** IO配置函数 **************************/
	P3M1 &= ~4;
	P3M0 |=  4;	 //推挽输出
	P3M1 |=  8;
	P3M0 &=  8;	 //浮空输入
	P3M1 &= ~32;
	P3M0 |=  32;	 //推挽输出
	reset = 1;
	/******************* 定时器0中断配置 **********************/
	Timer0_12T(); //定时器12分频
	Timer0_16bitAutoReload();//16位自动装载
	Timer0_AsTimer();//时器0用做定时器
	Timer0_Load(Timer0_Reload);
	Timer0_InterruptEnable();
	Timer0_Run();
	
	/******************* 外部中断1配置 *********************/
	IT1 = 1;
	EX1 = 1;
	PX1 = 1;//外部中断1最高优先级
	/*********************************************************/
	EA  = 1;//使能总中断
	
	
	while(1)
	{
		//WDT_reset(D_WDT_SCALE_256);//喂狗（本芯片）
		feeddog_to_sgm706 = 1;
		delay_ms(300);
		feeddog_to_sgm706 = 0;
		delay_ms(300);	
	}
}


/********************** INT0 外部中断函数 ************************/
void INT1_int (void) interrupt 2		//进中断时已经清除标志
{
    dog_count = 0;
	int_number++;
}


/********************** Timer0 20ms中断函数 ************************/
void timer0 (void) interrupt 1
{	
	dog_count++;
	
	if(dog_count > dog_time){
		if(dog_flags == 1){
		reset = 0;
		delay_ms(100);
		reset = 1;
		}
		dog_count = 0;  //清零计数器，同时防止dog_count溢出
	} 
	
	if(dog_count == 5){     //在1秒时，中断次数（中断信号频率需要大于0.5Hz）；3~7：看门狗开始计时；>10看门狗停止计时
		if(int_number >= 3 && int_number <= 7){
			if(dog_flags == 1)   //模式标志位：0 ：关闭状态； 1：开启状态；第一次开启
				dog_time = watchdog_of_b1300_s;
			dog_flags = 1;       //第一次开启，看门狗时间间隔为3min，之后为30s
			dog_count = 0;
		}
		if(int_number >= 10){
			dog_flags = 0;
			dog_time = watchdog_of_b1300_first_s;
			}
		int_number = 0;
	}
}

