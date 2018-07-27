#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"  
//外部中断0服务程序
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

    KEY_Init();	 //	按键端口初始化

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

    //GPIOA.11 中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource11);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line11;	//KEY1
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

   //GPIOA.10	  中断线以及中断初始化配置 下降沿触发 //KEY2
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource10);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line10;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

   //GPIOA.9	  中断线以及中断初始化配置  下降沿触发	//KEY3
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource9);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line9;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	//GPIOA.8	  中断线以及中断初始化配置  下降沿触发	//KEY4
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource8);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line8;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
  	
   //GPIOA.0	  中断线以及中断初始化配置 上升沿触发 PA0  WK_UP KEY5
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0); 

  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_Init(&EXTI_InitStructure);		//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//使能按键KEY5所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 

    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键KEY3 KEY4所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键KEY1 KEY2所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
  	
}



