#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
//#define KEY1 PAin(11)
//#define KEY2 PAin(10)  
//#define KEY3 PAin(9)  
//#define KEY4 PAin(8)  
//#define KEY5 PAin(0)   
 

#define KEY1  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_10)//读取按键2
#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_9)//读取按键3
#define KEY4  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)//读取按键4
#define KEY5  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键5
 
#define KEY_NOPRES 0  //没有按下键盘
#define KEY1_PRES	1		//KEY1值
#define KEY2_PRES	2		//KEY2
#define KEY3_PRES	3		//KEY3
#define KEY4_PRES	4		//KEY4
#define KEY5_PRES	5		//KEY5

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8 mode);  	//按键扫描函数
void keyTest(void);         //键盘测试函数
#endif
