
/*---------------------------------------------------------------------*/

#include    <reg52.h>
/*************	����˵��	**************

GL-S1300Ӳ�����Ź�

******************************************/

/*************	���س�������	**************/
sfr	AUXR = 0x8E;
sfr P3M1  = 0xB1;	//P3M1.n,P3M0.n 	=00--->Standard,	01--->push-pull
sfr P3M0  = 0xB2;	//					=10--->pure input,	11--->open drai

sbit reset = P3^5;
sbit feeddog_to_sgm706 = P3^2;
sbit feeddog_by_b1300 = P3^3;

#define     MAIN_Fosc		22118400L	//������ʱ��
#define		Timer0_Reload	(65536UL -(MAIN_Fosc / (12*50)))		//Timer 0 �ж�Ƶ��, 50��/��
#define 	Timer0_12T()				AUXR &= ~(1<<7)	//Timer0 clodk = fo/12	12��Ƶ,	default
#define		Timer0_16bitAutoReload()	TMOD &= ~0x03					//16λ�Զ���װ
#define		Timer0_AsTimer()			TMOD &= ~4		//ʱ��0������ʱ��
#define		Timer0_Load(n)		TH0 = (n) / 256,	TL0 = (n) % 256
#define 	Timer0_InterruptEnable()	ET0 = 1				//����Timer1�ж�.
#define 	Timer0_Run()	 			TR0 = 1				//����ʱ��0����

#define   watchdog_of_b1300_s    1650 //ʱ�䵥λ 30*50 ÿ����50��Ϊһ��

/*************	���ر�������	**************/

//unsigned int dog_time = watchdog_of_b1300_first_s;
unsigned int dog_time ;//= watchdog_of_b1300_s;
unsigned int dog_count;// = 0;
unsigned char int_number; //= 0;
unsigned char dog_flags; //= 0;
unsigned char reset_flags;//= 0; //��λ��־�������ط�������
/*************	���غ�������	**************/



/*************  ��ʱ���� *****************/

void  delay_ms(unsigned int ms)
{
     unsigned int i,j=0,k=0;
	 i = MAIN_Fosc / 21000UL;
	 for(k=0;k<ms;k++)
        for(j=0;j<i;j++);
}

/******************** ������ **************************/
void main(void)
{	
 dog_time = watchdog_of_b1300_s;
 dog_count= 0;
 int_number= 0;
 dog_flags= 0;
 reset_flags= 0; //��λ��־�������ط�������
	/******************** IO���ú��� **************************/
	P3M1 &= ~4;
	P3M0 |=  4;	 //�������
	P3M1 |=  8;
	P3M0 &= ~8;	 //��������
	P3M1 &= ~32;
	P3M0 |=  32;	 //�������
	reset = 1;
	/******************* ��ʱ��0�ж����� **********************/
	Timer0_12T(); //��ʱ��12��Ƶ
	Timer0_16bitAutoReload();//16λ�Զ�װ��
	Timer0_AsTimer();//ʱ��0������ʱ��
	Timer0_Load(Timer0_Reload);
	Timer0_InterruptEnable();
	Timer0_Run();

	/******************* �ⲿ�ж�1���� *********************/
	IT1 = 1;
	EX1 = 1;
	PX1 = 1;//�ⲿ�ж�1������ȼ�
	/*********************************************************/
	EA  = 1;//ʹ�����ж�
	
	
	while(1)
	{
		feeddog_to_sgm706 = 1;
		delay_ms(250);
		feeddog_to_sgm706 = 0;
		delay_ms(250);	
	}
}


/********************** INT0 �ⲿ�жϺ��� ************************/
void INT1_int (void) interrupt 2		//���ж�ʱ�Ѿ������־
{
  dog_count = 0;
	int_number++;
}


/********************** Timer0 20ms�жϺ��� ************************/
void timer0 (void) interrupt 1
{	
	static i;
	dog_count++;
	if(reset_flags >= 2)   //mcu�Ѿ���λ�����أ�������ι�������Ѿ�����
	{	
		if(i > 10)
		{
			feeddog_by_b1300 = 0;
		}
		if(i > 20)
		{
			feeddog_by_b1300 = 1;
			i = 0;
		}
		if(dog_count > 400)
		{
			reset_flags = 0;  //�˳����ͼ�¼����
			dog_flags = 0;  //�رճ�ʱ��λ����
			P3M1 |=  8;
		  P3M0 &= ~8;	 //��������
			EX1 = 1;
			dog_count = 0;
		}
		i++;
	}
	
	if(dog_count > dog_time){
		if(dog_flags == 1){
			reset = 0;
			delay_ms(100);  //��λ����
			reset = 1;
			reset_flags = 1; //mcu�Ѿ���λ������
			dog_flags = 0;  //�رո�λ���ع���
		}
		else
		{
			dog_count = 0;  //�����������ͬʱ��ֹdog_count���
			P3M1 |=  8;
		  P3M0 &= ~8;	 //��������
			EX1 = 1;
			PX1 = 1;
		}
	} 
	
	if(dog_count > 50 && int_number !=0){     //��1��ʱ���жϴ������ж��ź�Ƶ����Ҫ����0.5Hz����3~7�����Ź���ʼ��ʱ��>10���Ź�ֹͣ��ʱ
		if(int_number >= 3 && int_number <= 7){
			dog_flags = 1;       //��һ�ο��������Ź�ʱ����Ϊ3min��֮��Ϊ30s
			dog_count = 0;
			if(reset_flags == 1) //������ر���λ������
			{
				reset_flags = 2;  //���������¼��λ����
				dog_flags = 0;    //��ֹ��λ����
				EX1 = 0;         //�ر��ж�
				P3M1 &=  ~8;
		    P3M0 |=  8;	 //�������
			}
		}
		if(int_number >= 10){
			dog_flags = 0;
			}
		int_number = 0;
	}
}

