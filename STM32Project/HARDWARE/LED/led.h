#ifndef __LED_H
#define __LED_H	 
#include "sys.h"
#include "key.h"
////////////////////////////////////////////////////////////////////////////////// 
#define LED1 PAout(12)	// PA12   LED1 LED2 低电平亮 高电平灭
#define LED2 PAout(15)  //PA15
#define LEDR PBout(5)  //红黄蓝三色灯高电平亮 低电平灭
#define LEDB PBout(3)
#define LEDG PBout(4)
void LED_Init(void);//初始化函数

void LED_Flicker(u8 count);//led闪烁程序
void LED1_Open(void);
void LED1_Close(void);
void LED2_Open(void);
void LED2_Close(void);
void LEDR_Open(void); //点亮LEDR
void LEDB_Open(void);	//点亮LEDB
void LEDG_Open(void);	//点亮LEDG
void LED_RBGAllClose(void);
#endif
