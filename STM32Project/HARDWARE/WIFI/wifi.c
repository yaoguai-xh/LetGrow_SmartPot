#include "wifi.h"

void WiFi_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB,ENABLE);	 //使能PA PB端口时钟

	/*
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //wifi state 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 			 //上拉输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB 1
	*/
	 
	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //wifi en 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOA 4
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void WiFi_Enable()
{
	GPIO_SetBits(GPIOA,GPIO_Pin_4);
}

void WiFi_Disable()
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_4);
}

void ReadWiFiState(unsigned char *state)
{
	*state=WiFiState;
}
