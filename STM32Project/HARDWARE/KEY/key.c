#include "key.h"
#include "led.h"
#include "delay.h"
//按键初始化函数 
//PA11 PA10 PA9 PA8 PA0 设置成输入
void KEY_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;//PA11
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA11
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_10;//PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA10
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9;//PA9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA9
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_8;//PA8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA8
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;//PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA0
	
} 
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY1_PRES，KEY1按下
//KEY2_PRES，KEY2按下
//KEY3_PRES，KEY3按下
//KEY4_PRES，KEY4按下
//KEY5_PRES，KEY5按下
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY1==0||KEY2==0||KEY3==0||KEY4==0||KEY5==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;
		else if(KEY3==0)return KEY3_PRES;
		else if(KEY4==0)return KEY4_PRES;
		else if(KEY5==1)return KEY5_PRES;
	}else if(KEY1==1&&KEY2==1&&KEY3==1&&KEY4==1&&KEY5==0)key_up=1; 	     
	return 0;// 无按键按下
}

void keyTest()
{
		u8 mode=0;
		u8 keyValue=0;
		keyValue=KEY_Scan(mode);
		switch(keyValue)
		{
			case KEY1_PRES:
				LED_Flicker(KEY1_PRES);	break;
			case KEY2_PRES:
				LED_Flicker(KEY2_PRES);	break;
			case KEY3_PRES:
				LED_Flicker(KEY3_PRES);	break;
			case KEY4_PRES:
				LED_Flicker(KEY4_PRES);	break;
			case KEY5_PRES:
				LED_Flicker(KEY5_PRES);	break;
			default:
				delay_ms(10);	break;
		}
	
}
