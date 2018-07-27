#ifndef __BH1750_H__
#define __BH1750_H__
#include "delay.h"
#include "sys.h"

#define uchar unsigned char
#define uint  unsigned int

#define sda   GPIO_Pin_9    //PB9
#define scl   GPIO_Pin_8    //PB8
#define BH1750_1 PBout(7)
#define BH1750_2 PBout(6)

#define	  SlaveAddress   0x46   //定义器件在IIC总线中的从地址,根据ALT  ADDRESS地址引脚不同修改
//ALT  ADDRESS引脚接地时地址为0xA6，接电源时地址为0x3A

void BH1750_Init(void);
void conversion(uint temp_data);
void Single_Write_BH1750(uchar REG_Address);//单个写入数据
unsigned char Single_Read_BH1750(uchar REG_Address);   //单个读取内部寄存器数据
void  mread(void);         //连续的读取内部寄存器数据
void Convert_BH1750(float *result_lx);
void GPIOConfig(void);
void BH1750_1_Open(void);
void BH1750_2_Open(void);

#endif

