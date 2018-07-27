#include<beep.h>
#include<delay.h>
#include<sys.h>
void BEEP_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);//使能PC端口时钟,复用时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 //BEEP-->PC13 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;		 //IO口速度为2MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA12
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);						 //PC.13 输出低
	
}
void beeping_ms(int time_ms)  //蜂鸣器响 x 毫秒
{
	BEEP=1;
	delay_ms(time_ms);
	BEEP=0;
	delay_ms(time_ms);
}
