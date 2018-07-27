#ifndef __WATERSENSOR_H
#define __WATERSENSOR_H	
#include "sys.h"

void WaterSensor_Init(void);
u16 Get_WaterSensor_Adc(u8 ch); 
u16 Get_WaterSensor_Adc_Average(u8 ch,u8 times); 
 
#endif 
