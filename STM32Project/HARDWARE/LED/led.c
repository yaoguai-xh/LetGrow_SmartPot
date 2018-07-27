#include "led.h"
#include "delay.h"
#include "key.h"
//LED IO初始化
#define GPIO_Remap_SWJ_JTAGDisable  ((uint32_t)0x00300200)  /*!< JTAG-DP Disabled and SW-DP Enabled */


void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
 GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//使能SW 禁用JTAG 即启用JTDI(PA15)和JTDO（PB3）端口
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 //LED1-->PA12 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA12
 GPIO_SetBits(GPIOA,GPIO_Pin_12);						 //PA.12 输出高
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 //LED2-->PA15 端口配置
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA15
 GPIO_SetBits(GPIOA,GPIO_Pin_15);						 //PA.15 输出高

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;				 //LEDB-->PB3 端口配置
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOPB3
 GPIO_ResetBits(GPIOB,GPIO_Pin_3);						 //PB3 输出高
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //LEDG-->PB4 端口配置
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOPB4
 GPIO_ResetBits(GPIOB,GPIO_Pin_4);						 //PB4 输出高
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //LEDR-->PB5 端口配置
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOPB5
 GPIO_ResetBits(GPIOB,GPIO_Pin_5);						 //PB5 输出高
 
}
void LED_Flicker(u8 count)   ////led闪烁程序
{
	u8 i;
	LED1=1;
	for(i=0;i<count;i++)
	{
		LED1=!LED1;
		delay_ms(500);
		LED1=!LED1;
		delay_ms(500);
	}
}
void LED1_Open()
{
	LED1=0;
}
void LED1_Close()
{
	LED1=1;
}
void LED2_Open()
{
	LED2=0;
}
void LED2_Close()
{
	LED2=1;
}
void LEDR_Open()
{
	LEDR=1;
	LEDB=0;
	LEDG=0;
}
void LEDB_Open()
{
	LEDR=0;
	LEDB=1;
	LEDG=0;
}
void LEDG_Open()
{
	LEDR=0;
	LEDB=0;
	LEDG=1;
}
void LED_RBGAllClose()
{
	LEDR=0;
	LEDB=0;
	LEDG=0;
}
