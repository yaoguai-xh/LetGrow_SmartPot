#ifndef __SHT11_H
#define __SHT11_H
#include "stm32f10x.h"

#define SCK		GPIO_Pin_10
#define DATA 	GPIO_Pin_11
#define PORT	GPIOB
#define CLK		RCC_APB2Periph_GPIOB

#define ACK		1
#define noACK	0

#define REG_WRITE		0x06	//Ð´×´Ì¬¼Ä´æÆ÷
#define REG_READ 		0x07	//¶Á×´Ì¬¼Ä´æÆ÷
#define TEMP_MEASURE	0x03	//ÎÂ¶È²âÁ¿
#define HUMI_MEASURE	0x05	//Êª¶È²âÁ¿
#define RESET			0x1e	//Èí¸´Î»

typedef union
{
	uint16_t i;
	float f;
}value;

enum{TEMP, HUMI};				//Ã¶¾Ù£ºÎ´¸¶ÖµÄ¬ÈÏÎª´Ó0,1,2......


void SHT11_GPIO_Config(void);
void DATA_Mode_Out_PP(void);
void DATA_Mode_IPU(void);
void SHT11_SCK_OUT(uint8_t a);
void SHT11_DATA_OUT(uint8_t a);
uint8_t SHT11_DATA_Read(void);
void SHT11_Start(void);
void SHT11_Reset(void);
char SHT11_SoftReset(void);

char SHT11_WriteByte(unsigned char value);
char SHT11_ReadByte(unsigned char ack);
char SHT11_Read_StatusReg(unsigned char *p_value, unsigned char *p_checksum);
char SHT11_Write_StatusReg(unsigned char *p_value);

char SHT11_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode);
void SHT11_Convert(float *p_humidity ,float *p_temperature);

float SHT11_Convert_Dewpoint(float h,float t);

#endif
