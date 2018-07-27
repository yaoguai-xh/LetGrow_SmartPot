#ifndef __BATTERY_H
#define __BATTERY_H	 
#include "sys.h"

void  BatteryADC_Init(void);
u16 Get_Battery_Adc(u8 ch);
u16 Get_Battery_Adc_Average(u8 ch,u8 times);
#endif
