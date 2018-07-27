#ifndef __WS2812B_H
#define	__WS2812B_H

#include "stm32f10x.h"
#include "delay.h"	

//RGB 模式
#define White       0xFFFFFF  // 白色
#define Black       0x000000  // 黑色
#define Red         0xff0000  // 红色
#define Green       0x00ff00  // 绿色
#define Blue        0x0000ff  // 蓝色


#define nWs 8		// 有多少颗WS2811级联

extern unsigned long WsDat[];

extern void WS_Init(void);
extern void WS_Reset(void);
extern void WS_SetAll(unsigned long);
extern u32 ColorToColor(unsigned long color0, unsigned long color1);
#endif /* __LED_H */
