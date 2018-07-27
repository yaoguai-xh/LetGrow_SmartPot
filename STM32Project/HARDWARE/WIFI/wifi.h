#ifndef __WIFI_H
#define __WIFI_H	 
#include "sys.h"

#define WiFiState  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)//∂¡»°  PB1
#define WiFiEN PAout(4)

void WiFi_Init(void);
void WiFi_Enable(void);
void WiFi_Disable(void);
void ReadWiFiState(unsigned char *state);//∂¡»°wifi state

#endif
