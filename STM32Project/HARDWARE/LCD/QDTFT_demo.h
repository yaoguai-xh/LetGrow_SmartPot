#ifndef __QDTFT_DEMO_H
#define __QDTFT_DEMO_H 
#include "sys.h"


void Redraw_Mainmenu(void);
void Num_Test(void);
void Font_Test(void);
void Color_Test(void);
void showimage(const unsigned char *p); 
void QDTFT_Test_Demo(void);
void LCD_Test(void);
void xy_LCD_PutString(unsigned short x, unsigned short y, unsigned char *s, unsigned int fColor, unsigned int bColor,u8 bigFont);
#endif
