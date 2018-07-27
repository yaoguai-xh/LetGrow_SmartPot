/**
  ******************************************************************************
  * @file    sht11.c
  * @author  
  * @version V1.0
  * @date    2015-xx-xx
  * @brief   传感器默认分辨率为温度14位，湿度12位
  ******************************************************************************
  */
#include "sht11.h"
#include "delay.h"
#include "math.h"

void SHT11_GPIO_Config(void)
{		
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(CLK, ENABLE); 														   
  	GPIO_InitStructure.GPIO_Pin = SCK | DATA;	
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
  	GPIO_Init(PORT, &GPIO_InitStructure);		  	 
}

void DATA_Mode_Out_PP(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//推挽输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT, &GPIO_InitStructure);
}

void DATA_Mode_IPU(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = DATA;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入模式
	GPIO_Init(PORT, &GPIO_InitStructure);
}

void SHT11_SCK_OUT(uint8_t a)
{
	if(a)
		GPIO_SetBits(PORT, SCK);
	else
		GPIO_ResetBits(PORT, SCK);
}

void SHT11_DATA_OUT(uint8_t a)
{
	DATA_Mode_Out_PP();
	if(a)
		GPIO_SetBits(PORT, DATA);
	else
		GPIO_ResetBits(PORT, DATA);
}

uint8_t SHT11_DATA_Read(void)
{
	DATA_Mode_IPU();
	return GPIO_ReadInputDataBit(PORT, DATA);
}

void SHT11_Start(void)
{  
	SHT11_DATA_OUT(1); 
	SHT11_SCK_OUT(0);      	//Initial state
	delay_us(2);
	SHT11_SCK_OUT(1);
	delay_us(2);
	SHT11_DATA_OUT(0);
	delay_us(2);
	SHT11_SCK_OUT(0);  
	delay_us(6);
	SHT11_SCK_OUT(1);
	delay_us(2);
	SHT11_DATA_OUT(1);		   
	delay_us(2);
	SHT11_SCK_OUT(0);		   
}

void SHT11_Reset(void)
{  
	unsigned char i; 
	
	SHT11_DATA_OUT(1); 
	SHT11_SCK_OUT(0);                   //Initial state
	for(i=0;i<9;i++)                  	//9 SCK cycles
	{ 
		SHT11_SCK_OUT(1);
		SHT11_SCK_OUT(0);
	}
	SHT11_Start();                   	//transmission start
}

char SHT11_SoftReset(void)
{
	unsigned char error;
	
	SHT11_Reset();
	error += SHT11_WriteByte(RESET);
	
	return error;
}

char SHT11_WriteByte(unsigned char value)
//----------------------------------------------------------------------------------
// writes a byte on the Sensibus and checks the acknowledge 
{
	unsigned char i,error=0;  

	for (i=0x80;i>0;i/=2)               //shift bit for masking
	{ 
		if (i & value)
			SHT11_DATA_OUT(1);          //masking value with i , write to SENSI-BUS
		else 
			SHT11_DATA_OUT(0);   

		delay_us(2);                    //observe setup time
		SHT11_SCK_OUT(1);               //clk for SENSI-BUS
		delay_us(6);        			//pulswith approx. 5 us  	
		SHT11_SCK_OUT(0);
		delay_us(2);                    //observe hold time
	}
	SHT11_DATA_OUT(1);                  //release DATA-line
	delay_us(2);                        //observe setup time
	SHT11_SCK_OUT(1);                   //clk #9 for ack 
	error=SHT11_DATA_Read();            //check ack (DATA will be pulled down by SHT11)
	SHT11_SCK_OUT(0);       
	
	return error;                  	    //error=1 in case of no acknowledge
}

char SHT11_ReadByte(unsigned char ack)
//----------------------------------------------------------------------------------
// reads a byte form the Sensibus and gives an acknowledge in case of "ack=1" 
{ 
	unsigned char i,val=0;
	
	SHT11_DATA_OUT(1);                          //release DATA-line
	for (i=0x80;i>0;i=i>>1)             		//shift bit for masking
	{ 
		SHT11_SCK_OUT(1);                       //clk for SENSI-BUS
		if (SHT11_DATA_Read()) val=(val | i);   //read bit  
		SHT11_SCK_OUT(0);  					 
	}
	
	SHT11_DATA_OUT(!ack);               //in case of "ack==1" pull down DATA-Line
	delay_us(2);                        //observe setup time
	SHT11_SCK_OUT(1);                   //clk #9 for ack
	delay_us(6);          				//pulswith approx. 5 us 
	SHT11_SCK_OUT(0);
	delay_us(2);                        //observe hold time						    
	SHT11_DATA_OUT(1);                  //release DATA-line
	
	return val;
}


char SHT11_Read_StatusReg(unsigned char *p_value, unsigned char *p_checksum)
//----------------------------------------------------------------------------------
// reads the status register with checksum (8-bit)
{ 
	unsigned char error = 0;

	SHT11_Start();                   		//transmission start
	error = SHT11_WriteByte(REG_READ); 		//send command to sensor
	*p_value = SHT11_ReadByte(ACK);        	//read status register (8-bit)
	*p_checksum = SHT11_ReadByte(noACK);   	//read checksum (8-bit)   
	
	return error;                     		//error=1 传感器响应失败
}

char SHT11_Write_StatusReg(unsigned char *p_value)
//----------------------------------------------------------------------------------
// writes the status register with checksum (8-bit)
{ 
	unsigned char error = 0;

	SHT11_Start();                  		//transmission start
	error += SHT11_WriteByte(REG_WRITE);	//send command to sensor
	error += SHT11_WriteByte(*p_value);    	//send value of status register
	
	return error;                     		//error>=1 in case of no response form the sensor
}


char SHT11_Measure(unsigned char *p_value, unsigned char *p_checksum, unsigned char mode)
//----------------------------------------------------------------------------------
// makes a measurement (humidity/temperature) with checksum
{ 
	unsigned char error=0;
	unsigned int i;

	SHT11_Start();                   	//transmission start
	switch(mode)
	{                     				//send command to sensor
		case TEMP	: error+=SHT11_WriteByte(TEMP_MEASURE); break;
		case HUMI	: error+=SHT11_WriteByte(HUMI_MEASURE); break;
		default     : break;	 
	}

	for (i=0;i<100000;i++)
	if(SHT11_DATA_Read()==0) break; 		//wait until sensor has finished the measurement
	if(SHT11_DATA_Read()) error+=1;         // or timeout (~2 sec.) is reached

	*(p_value+1) = SHT11_ReadByte(ACK);    	//read the first byte (MSB)
	*(p_value) = SHT11_ReadByte(ACK);
	*p_checksum = SHT11_ReadByte(noACK);  	//read checksum

	return error;
}

void SHT11_Convert(float *p_Humidity ,float *p_Temperature)
//----------------------------------------------------------------------------------------
// calculates temperature [°C] and humidity [%RH] 
// input :  humi [Ticks] (12 bit) 
//          temp [Ticks] (14 bit)
{
	const float C1 = -2.0468;           // for 12 Bit RH
	const float C2 = +0.0367;           // for 12 Bit RH
	const float C3 = -0.0000015955;     // for 12 Bit RH
	const float T1 = +0.01;             // for 12 Bit RH
	const float T2 = +0.00008;          // for 12 Bit RH	

	float humi = *p_Humidity;           // humi:      Humidity [Ticks] 12 Bit 
	float temp = *p_Temperature;        // temp:      Temperature [Ticks] 14 Bit
	float RH_line;                     	// RH_line:   Humidity linear
	float RH_true;                    	// RH_true:   Temperature compensated humidity
	float Temp_C;                       // Temp_C :   Temperature [°C]

	Temp_C = temp * 0.01 - 39.7;               	 	//实际温度 [°C] from 14 bit temp. ticks @ 3.5V
	RH_line = C3*humi*humi + C2*humi + C1;    	    //相对湿度即线性湿度 [%RH]
	RH_true = (Temp_C-25)*(T1+T2*humi)+RH_line;     //实际湿度：湿度信号的温度补偿 [%RH]
	
	
	Temp_C-=31.5;
	RH_true-=30;//温湿度补偿
	
	
	if(RH_true>100) RH_true=100;       				//cut if the value is outside of
	if(RH_true<0.1) RH_true=0.1;       				//the physical possible range
	*p_Temperature = Temp_C;               			//return temperature [°C]
	*p_Humidity = RH_true;              			//return humidity[%RH]
}

float SHT11_Convert_Dewpoint(float h, float t)
//--------------------------------------------------------------------
{ 
	float m = 17.62;								//above water, 0_50°C
	float Tn = 243.12;
	float k,dew_point ;
  
	k = (log10(h)-2)/0.4343 + (m*t)/(Tn+t);
	dew_point = Tn*k / (m-k);
	
	return dew_point;
}
