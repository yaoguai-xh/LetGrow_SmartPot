#include "led.h"
#include "WS2812B.h" 
#include "wifi.h"
#include "key.h"
#include "exti.h"
#include "rtc.h"
#include "pwm.h"
#include "beep.h"
#include "pump.h"
#include "sht11.h"
#include "sht2x.h"
#include "dht11.h"
#include "waterSensor.h"
#include "battery.h"
#include "iwdg.h"
#include "bh1750.h"

#include "delay.h"
#include "usart.h"
#include "sys.h"
#include<string.h>
//关于LCD的头文件
#include "Lcd_Driver.h"
#include "QDTFT_demo.h" 
#include "GUI.h"
/************************************************************************************************/
/****************************************全局变量************************************************/ 
#define TRUE 1
#define FALSE 0
#define BufLength 15
#define InstructionsCount 9 			 //指令数量
#define InstructionsMaxLength 40 		 //指令最大长度
const char Instructions[InstructionsCount][InstructionsMaxLength]={"PUMP_Open","ServoRollBack",
	"setServoAngle=","setSoilHumidityThreshold=","setIlluminationThreshold=","setWS2812BColor=","openLED",
	"closeLED","setTime="};//存放指令

int SoilHumidityThreshold=70;		//土壤湿度门限阈值 默认值80  可通过wifi修改
int illuminationThreshold=4000;		//光照强度门限值 默认值4000lux 可通过wifi修改
int ServoAngle=0;					//存放舵机角度数值 默认为0    范围0~180
	
unsigned char ReceiveBuf[20]={0};
unsigned char SendBuf[20]={0};
unsigned char USARTReceiveLength;		//存放串口接收数据的长度
char colorstr[10]={0};				//用于存储接收的颜色字符串
char *colorStr=colorstr;
unsigned long colorNum=0xffffff;		//用于存储转换过后的颜色值
unsigned long oldColorNum=0xffffff;		//用于存储转换过后的颜色值  渐变色控制的两个颜色
char *timestrbuf;                       //用于存储转换后的时间字符串
char *tempbuf;                          //用于存储转换后的  SoilHumidityThreshold  illuminationThreshold  ServoAngle
char tempStr[10];                        //用于显示RTC的临时转换字符串

unsigned char isPhoneControl=FALSE;		//是否手机控制
unsigned char wifiIsConnected=FALSE;

typedef struct{
	float soilTemperature;
	float soilHumidity;
	unsigned char airTemperature;
	unsigned char airHumidity;
	int waterADC;
	float illumination_1;
	float illumination_2;
}SENSOR_STATE; 

typedef struct{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
}RTC_TIME;

SENSOR_STATE sensorState;
RTC_TIME RTCTime,OldTime;
/****************************************全局变量************************************************/
/************************************************************************************************/

//舵机控制
void setServoAngle(int servoAngle)
{
	char PWMStr[4];
	char num[3];
	//servoAngle  舵机角度
	int pwmValue=0;//pwm 定时器初值
	//pwmValue=(int)(2000*(1-( ( 2.5-(servoAngle/90.0) )/20.0 )  ));
	pwmValue=(int)((100.0/180.0)*servoAngle+100);
	sprintf(PWMStr,"%4d",pwmValue);
	sprintf(num,"%d",servoAngle);
	
	Gui_DrawFont_GBK16(0,80,GREEN,BLACK,"         ");
	//Gui_DrawFont_GBK16(0,80,GREEN,BLACK,PWMStr);//显示占空比value 
	Gui_DrawFont_GBK16(0,80,GREEN,BLACK,"舵机:");
	Gui_DrawFont_GBK16(48,80,GREEN,BLACK,(unsigned char *)num);//显示舵机角度
	Gui_DrawFont_GBK16(72,80,GREEN,BLACK,"度");
	
	TIM_SetCompare1(TIM3,pwmValue);
	delay_ms(500);
}

//舵机反转函数
void ServoRollback()
{
	if(ServoAngle==0)
	{
		ServoAngle=180;
		setServoAngle(180);
	}
	else if(ServoAngle==180)
	{
		ServoAngle=0;
		setServoAngle(0);
	}
	delay_ms(500);
}

void LCD_Display()//LCD显示
{
	Lcd_Clear(BLACK);
	Gui_DrawFont_GBK16(16,120,GREEN,BLACK,"欢迎使用LetGrow智能花盆");			
}

//串口接收程序 ---------------wifi
void usartReceive()
{
	char t;
	if(USART_RX_STA&0x8000) 
	{					   
		USARTReceiveLength=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
		
		printf("\r\n您发送的消息为:\r\n");
		for(t=0;t<USARTReceiveLength;t++)
		{
			USART2->DR=USART_RX_BUF[t];
			ReceiveBuf[t]=USART_RX_BUF[t];
			while((USART2->SR&0X40)==0);//等待发送结束
			USART_RX_STA=0;
		}
		/*
		Gui_DrawFont_GBK16(0,160,GREEN,BLACK, "             ");
		Gui_DrawFont_GBK16(0,160,GREEN,BLACK,SendBuf);
		Gui_DrawFont_GBK16(0,140,GREEN,BLACK,"Rec:          ");
		Gui_DrawFont_GBK16(32,140,GREEN,BLACK,ReceiveBuf);
		for(i=0;i<BufLength;i++)//显示后就销毁
		{
			SendBuf[i]=0;
			ReceiveBuf[i]=0;
		}
		*/
	}
}

//读取土壤湿度温度状态并显示，作出响应 sht11
void SoilState()
{
	value humi_val, temp_val;
	unsigned char error,checksum;
	char str[4];//用来转换的字符串
	float temperature=0,humidity=0;
	error=0;	
	error+=SHT11_Measure((unsigned char*) &humi_val.i,&checksum,HUMI);  //测量湿度
	error+=SHT11_Measure((unsigned char*) &temp_val.i,&checksum,TEMP);  //测量温度 测量值放入value.i
	if(error!=0)	 
	{
		SHT11_Reset();         		//in case of an error: connection reset
	}
	else
	{ 
		humi_val.f=(float)humi_val.i;        //
		temp_val.f=(float)temp_val.i;        //转换类型 将value.i中的值强制转换为浮点型并放入value.f
		SHT11_Convert(&humi_val.f,&temp_val.f);
		temperature = temp_val.f; 	//温度
		humidity = humi_val.f;		//湿度
		sensorState.soilTemperature=temperature;//保存到结构体中
		sensorState.soilHumidity=humidity;
		//显示土壤温度和湿度
		Gui_DrawFont_GBK16(0,0,GREEN,BLACK, "T:   ");
		sprintf(str,"%3.1f",temperature);
		Gui_DrawFont_GBK16(16,0,GREEN,BLACK,(unsigned char *)str);
		Gui_DrawFont_GBK16(56,0,GREEN,BLACK, "H:   ");
		sprintf(str,"%3.1f",humidity);
		Gui_DrawFont_GBK16(72,0,GREEN,BLACK,(unsigned char *)str);
		//湿度低于阈值就会浇水
		if(humidity<SoilHumidityThreshold)
		{
			PUMP_Open();
			Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵开    ");
		}	  
		else
		{
			PUMP_Close();
			Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵关    ");
		}					
	}
}


//读取土壤湿度温度状态并显示，作出响应 sht20
void SoilState_sht20()
{
	char str[10];//用来转换的字符串
	float temperature=0.0,humidity=0.0;
	//temperature=SHT2x_MeasureTempHM();
	//humidity=SHT2x_MeasureHumiHM();
	temperature=SHT2x_MeasureTempPoll();//测量温度
	humidity=SHT2x_MeasureHumiPoll();//测量湿度
	humidity-=20.0;
	sensorState.soilTemperature=temperature;//保存到结构体中
	sensorState.soilHumidity=humidity;
	//显示土壤温度和湿度
	Gui_DrawFont_GBK16(0,0,GREEN,BLACK, "T:   ");
	sprintf(str,"%3.1f",temperature);
	Gui_DrawFont_GBK16(16,0,GREEN,BLACK,(unsigned char *)str);
	Gui_DrawFont_GBK16(56,0,GREEN,BLACK, "H:   ");
	sprintf(str,"%3.1f",humidity);
	Gui_DrawFont_GBK16(72,0,GREEN,BLACK,(unsigned char *)str);
	//湿度低于阈值就会浇水
	if(humidity<SoilHumidityThreshold)
	{
		PUMP_Open();
		Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵开    ");
	}	  
	else
	{	
		PUMP_Close();
		Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵关    ");
	}					
	
}

//读取空气温湿度状态并显示
void AirState()
{
	unsigned char temperature=0,humidity=0;
	char str[5];		//用来转换的字符串
	DHT11_Read_Data(&temperature,&humidity);//读取温湿度
	sensorState.airTemperature=temperature;//保存到结构体中
	sensorState.airHumidity=humidity;
	//显示空气温度和湿度
	Gui_DrawFont_GBK16(0,20,GREEN,BLACK, "T:   ");
	sprintf(str,"%d",temperature);
	Gui_DrawFont_GBK16(16,20,GREEN,BLACK,(unsigned char *)str);
	Gui_DrawFont_GBK16(56,20,GREEN,BLACK, "H:   ");
	sprintf(str,"%d",humidity);
	Gui_DrawFont_GBK16(72,20,GREEN,BLACK,(unsigned char *)str);
}

//获取水位传感器的值
void WaterSensorADC()
{
	char str[5];
	float adcVoltage;
	int adcValue=0;
	
	adcValue=Get_WaterSensor_Adc_Average(ADC_Channel_5,10);//PA5 对于ADC通道5
	sprintf(str,"%d",adcValue);
	Gui_DrawFont_GBK16(0,40,GREEN,BLACK,"     ");//显示水位传感器的值
	Gui_DrawFont_GBK16(0,40,GREEN,BLACK,(unsigned char *)str);
	
	sensorState.waterADC=adcValue;//保存到结构体中
	
	adcVoltage=((float)adcValue/4096)*3.3;//转换为电压
	sprintf(str,"%3.2f",adcVoltage);
	Gui_DrawFont_GBK16(56,40,GREEN,BLACK,"    V");//显示ADC转换后的电压
	Gui_DrawFont_GBK16(56,40,GREEN,BLACK,(unsigned char *)str);
	
	Gui_DrawFont_GBK16(112,40,GREEN,BLACK,"水位  ");
	//判断水位
	if(adcVoltage>=1.6)
	{
		Gui_DrawFont_GBK16(144,40,GREEN,BLACK,"满");
	}
	else if(adcVoltage>=1.5 && adcVoltage<1.6)
	{
		Gui_DrawFont_GBK16(144,40,GREEN,BLACK,"高");
	}
	else if(adcVoltage>=1.4 && adcVoltage<1.5)
	{
		Gui_DrawFont_GBK16(144,40,GREEN,BLACK,"中");
	}
	else if(adcVoltage<1.4)
	{
		Gui_DrawFont_GBK16(144,40,BLUE,BLACK,"低");
	}
}

//获取电池电压
void getBatteryADC()
{
	u8 i;
	char str[5];
	float adcVoltage;
	int adcValue=0;
	
	adcValue=Get_Battery_Adc_Average(ADC_Channel_8,10);//PB0 对于ADC通道8
	sprintf(str,"%d",adcValue);
	/*
	Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"     ");//显示电池AD的值
	Gui_DrawFont_GBK16(0,160,GREEN,BLACK,(unsigned char *)str);
	*/
	adcVoltage=((float)adcValue/4096)*3.3*2;//转换为电压   电路中电池的AD采样为1/2电池电压
	sprintf(str,"%3.2f",adcVoltage);
	
	Gui_DrawFont_GBK16(56,160,GREEN,BLACK,"    V");//显示ADC转换后的电压
	Gui_DrawFont_GBK16(56,160,GREEN,BLACK,(unsigned char *)str);
	
	i=(int)((adcVoltage-3.0)/((4.2-3.0)/13.0));
	//14级电量显示
	switch(i)
	{
		case 0:
			Gui_DrawFont_GBK16(0,160,RED,BLACK,"〇");
			Gui_DrawFont_GBK16(16,160,RED,BLACK,"  0%");//显示电量百分比
			break;
		case 1:
			Gui_DrawFont_GBK16(0,160,RED,BLACK,"①");
			Gui_DrawFont_GBK16(16,160,RED,BLACK," 14%");//显示电量百分比
			break;
		case 2:
			Gui_DrawFont_GBK16(0,160,RED,BLACK,"②");
			Gui_DrawFont_GBK16(16,160,RED,BLACK," 21%");//显示电量百分比
			break;
		case 3:
			Gui_DrawFont_GBK16(0,160,RED,BLACK,"③");
			Gui_DrawFont_GBK16(16,160,RED,BLACK," 28%");//显示电量百分比
			break;
		case 4:
			Gui_DrawFont_GBK16(0,160,YELLOW,BLACK,"④");
			Gui_DrawFont_GBK16(16,160,YELLOW,BLACK," 35%");//显示电量百分比
			break;
		case 5:
			Gui_DrawFont_GBK16(0,160,YELLOW,BLACK,"⑤");
			Gui_DrawFont_GBK16(16,160,YELLOW,BLACK," 42%");//显示电量百分比
			break;
		case 6:
			Gui_DrawFont_GBK16(0,160,YELLOW,BLACK,"⑥");
			Gui_DrawFont_GBK16(16,160,YELLOW,BLACK," 50%");//显示电量百分比
			break;
		case 7:
			Gui_DrawFont_GBK16(0,160,YELLOW,BLACK,"⑦");
			Gui_DrawFont_GBK16(16,160,YELLOW,BLACK," 57%");//显示电量百分比
			break;
		case 8:
			Gui_DrawFont_GBK16(0,160,YELLOW,BLACK,"⑧");
			Gui_DrawFont_GBK16(16,160,YELLOW,BLACK," 64%");//显示电量百分比
			break;
		case 9:
			Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"⑨");
			Gui_DrawFont_GBK16(16,160,GREEN,BLACK," 71%");//显示电量百分比
			break;
		case 10:
			Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"⑩");
			Gui_DrawFont_GBK16(16,160,GREEN,BLACK," 78%");//显示电量百分比
			break;
		case 11:
			Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"⑾");
			Gui_DrawFont_GBK16(16,160,GREEN,BLACK," 78%");//显示电量百分比
			break;
		case 12:
			Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"⑿");
			Gui_DrawFont_GBK16(16,160,GREEN,BLACK," 85%");//显示电量百分比
			break;
		case 13:
			Gui_DrawFont_GBK16(0,160,GREEN,BLACK,"⒀");
			Gui_DrawFont_GBK16(16,160,GREEN,BLACK,"100%");//显示电量百分比
			break;
		default:
			break;
	}
}

//读取光强传感器数据
void illuminationState()
{
	char str[10];		//用来转换的字符串
	float illuminationValue=0;
	//读光强1
	BH1750_1_Open();
	BH1750_Init();
	Convert_BH1750(&illuminationValue);//转换BH1750的值
	sensorState.illumination_1=illuminationValue;//保存到结构体
	//控制舵机
	/*if(illuminationValue>=illuminationThreshold)
	{
		ServoAngle=0;
		setServoAngle(0);//光强超过阈值舵机反转
	}*/
	Gui_DrawFont_GBK16(0,60,GREEN,BLACK, "i1:      lux");//光强1
	sprintf(str,"%5.0f",illuminationValue);
	Gui_DrawFont_GBK16(24,60,GREEN,BLACK,(unsigned char *)str);
	
	//读光强2
	BH1750_2_Open();
	BH1750_Init();
	Convert_BH1750(&illuminationValue);
	sensorState.illumination_2=illuminationValue;//保存到结构体
	//控制舵机
	/*
	if(illuminationValue>=illuminationThreshold)
	{
		ServoAngle=180;
		setServoAngle(180);//光强超过阈值舵机反转
	} */
	Gui_DrawFont_GBK16(104,60,GREEN,BLACK, "i2:      lux");//光强2
	sprintf(str,"%5.0f",illuminationValue);
	Gui_DrawFont_GBK16(128,60,GREEN,BLACK,(unsigned char *)str);
	//控制光照
	if(isPhoneControl==FALSE)
	{
		if(sensorState.illumination_1>=1000||sensorState.illumination_2>=1000)
		{
			WS_SetAll(Black);//black
		}
		else
		{
			WS_SetAll(Red);//red
		}
	}
}
//RBG提示灯模块 需完善
void LED_RBGModle()
{
	float adcVoltage;
	
	LED_RBGAllClose();
	LEDG_Open();//正常 亮绿灯
	adcVoltage=((float)sensorState.waterADC/4096)*3.3;//转换为电压
	if(adcVoltage<1.4)
	{
		LED_RBGAllClose();
		LEDB_Open();//水位不足 亮蓝灯
	}
	if(sensorState.soilHumidity<SoilHumidityThreshold)
	{
		LED_RBGAllClose();
		LEDR_Open();//土壤湿度不够 亮红灯
	}
	if(sensorState.illumination_1>illuminationThreshold||sensorState.illumination_2>illuminationThreshold)
	{
		LED_RBGAllClose();
		LEDR_Open();
		LEDB_Open();//光线太强 红加蓝
	}
}

//连接测试函数  握手
int connectionTest()
{
	unsigned char i;
	//请求连接 失败重新请求 成功直接跳过 最多三次尝试
	for(i=0;i<4;i++)
	{
		printf("RequestConnection\r\n");
		delay_ms(50);
		if(USART_RX_STA&0x8000) //判断串口是否接收到数据
		{					   
			USARTReceiveLength=USART_RX_STA&0x3fff;//得到此次接收到的数据长度
			Gui_DrawFont_GBK16(0,100,GREEN,BLACK,"                        ");
			Gui_DrawFont_GBK16(0,100,GREEN,BLACK,USART_RX_BUF);
			if(USARTReceiveLength==3&&USART_RX_BUF[0]=='Y'&&USART_RX_BUF[1]=='e'&&USART_RX_BUF[2]=='s')
			{
				Gui_DrawFont_GBK16(16,140,GREEN,BLACK,"网络连接成功!");
				//USART_RX_STA=0;
				return 1;
			}
			else
			{
				//USART_RX_STA=0;
				continue;
			}
		}
		else
		{
			continue;
		}
	}
	Gui_DrawFont_GBK16(16,140,RED,BLACK,"网络连接失败!");
	return 0;
}

//读取所有传感器的值并通过WiFi发送
void sendSensorStateToWifi()
{
	if(1)
	{
		//SoilState();			//读取土壤湿度温度状态并显示，作出响应 sht11
		SoilState_sht20();		//读取土壤湿度温度状态并显示，作出响应 sht20
		AirState();				//读取空气温湿度状态并显示
		WaterSensorADC();		//获取水位传感器的值
		getBatteryADC();    	//获取电池电压
		illuminationState();	//读取光强传感器数据
		printf("DataBegin\r\n");
		printf("soilTemperature=%.2f\r\n",sensorState.soilTemperature);
		printf("soilHumidity=%.2f\r\n",sensorState.soilHumidity);
		printf("airTemperature=%d\r\n",sensorState.airTemperature);
		printf("airHumidity=%d\r\n",sensorState.airHumidity);
		printf("waterADC=%d\r\n",sensorState.waterADC);
		//
		printf("illumination_1=%.0f\r\n",sensorState.illumination_1);
		printf("illumination_2=%.0f\r\n",sensorState.illumination_2);
		printf("DataEnd\r\n");
	}
}

//获取系统RTC时钟 calendar结构体的值到RTCTime
void getSystemRTCTime(RTC_TIME *RTCTime)
{
	RTCTime->year=calendar.w_year;
	RTCTime->month=calendar.w_month;
	RTCTime->day=calendar.w_date;
	RTCTime->hour=calendar.hour;
	RTCTime->minute=calendar.min;
	RTCTime->second=calendar.sec;
}

//复制RTC时间结构体
void copyTime(RTC_TIME *destTime,RTC_TIME *srcTime)
{
	destTime->year=srcTime->year;
	destTime->month=srcTime->month;
	destTime->day=srcTime->day;
	destTime->hour=srcTime->hour;
	destTime->minute=srcTime->minute;
	destTime->second=srcTime->second;
}

/*判断时间是否相等或则时间差是否大于指定时间  
**此函数运行一次平均耗时6.8us
**参数long second为指定时间
**时间差为 NowTime - OldTime**/
int JudgeTime(RTC_TIME *OldTime,RTC_TIME *NowTime,long second)
{
	long sum=0;
	if(second==0)
	{
		if(OldTime->year==NowTime->year && OldTime->month==NowTime->month && OldTime->day==NowTime->day 
			&& OldTime->hour==NowTime->hour && OldTime->minute==NowTime->minute && OldTime->second==NowTime->second)
		{
			return TRUE;
		}
	}
	else if(second>0)
	{
		sum=(NowTime->hour-OldTime->hour)*3600 + (NowTime->minute-OldTime->minute)*60 + (NowTime->second-OldTime->second);
		if(sum>=second)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//显示RTC时间
void showRTCTime()
{
	if(RTCTime.second!=calendar.sec)
	{
		Gui_DrawFont_GBK16(156,160,GREEN,BLACK,"  :  :  ");
		sprintf(tempStr,"%d",calendar.hour);
		if(calendar.hour<10)
		{
			Gui_DrawFont_GBK16(156,160,GREEN,BLACK,"0");
			Gui_DrawFont_GBK16(164,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		else {
			Gui_DrawFont_GBK16(156,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		sprintf(tempStr,"%d",calendar.min);
		if(calendar.min<10)
		{
			Gui_DrawFont_GBK16(178,160,GREEN,BLACK,"0");
			Gui_DrawFont_GBK16(186,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		else {
			Gui_DrawFont_GBK16(180,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		sprintf(tempStr,"%d",calendar.sec);
		if(calendar.sec<10)
		{
			Gui_DrawFont_GBK16(204,160,GREEN,BLACK,"0");
			Gui_DrawFont_GBK16(212,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		else {
			Gui_DrawFont_GBK16(204,160,GREEN,BLACK,(unsigned char*)tempStr);
		}
		getSystemRTCTime(&RTCTime);//更新RTCTime时间
	}
}

//指令接收模块 用于主机通过WiFi控制单片机
void instructionReceiving()
{
	unsigned char i;
	char InstructionsStr[InstructionsMaxLength];//用于复制此次接收成功的指令
	USARTReceiveLength=USART_RX_STA&0x3fff;//得到此次接收到的数据长度 除\r\n之外的长度
	strcpy(InstructionsStr,USART_RX_BUF);
	Gui_DrawFont_GBK16(0,100,GREEN,BLACK,"***************************");
	Gui_DrawFont_GBK16(16,100,GREEN,BLACK,(unsigned char*)InstructionsStr);
	if(USART_RX_STA&0x8000) //判断串口是否接收到数据
	{		
		for(i=0;i<InstructionsCount;i++)
		{
			if(strncmp(Instructions[i],InstructionsStr,strlen(Instructions[i]))==0)
				break;
		}
		if(i<InstructionsCount)//指令匹配成功
		{
			//USART_RX_STA=0;
			switch(i)
			{
				case 0:
					PUMP_Open();//指令代码0
					Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵开    ");
					delay_ms(1500);
					delay_ms(1000);
					Gui_DrawFont_GBK16(112,0,GREEN,BLACK, "水泵关    ");
					PUMP_Close();
					break;
				case 1:
					ServoRollback();//指令代码1 
					break;
				case 2:
					tempbuf = strtok(InstructionsStr, "#");
					tempbuf = strtok(NULL,"#");
					sscanf(tempbuf,"%d",&ServoAngle);
					setServoAngle(ServoAngle);
					Gui_DrawFont_GBK16(150,100,RED,BLACK,"SET OK");
					break;
				case 3:
					tempbuf = strtok(InstructionsStr, "#");
					tempbuf = strtok(NULL,"#");
					sscanf(tempbuf,"%d",&SoilHumidityThreshold);
					Gui_DrawFont_GBK16(150,100,RED,BLACK,"SET OK");
					break;
				case 4:
					tempbuf = strtok(InstructionsStr, "#");
					tempbuf = strtok(NULL,"#");
					sscanf(tempbuf,"%d",&illuminationThreshold);
					Gui_DrawFont_GBK16(150,100,RED,BLACK,"SET OK");
					break;
				case 5:
					colorStr=strtok(InstructionsStr,"#");
					colorStr=strtok(NULL,"#");
					sscanf(colorStr,"%x",&colorNum);
					//ColorToColor(oldColorNum,colorNum);
					WS_SetAll(colorNum);
					oldColorNum=colorNum;
					isPhoneControl=TRUE;
					break;
				case 6:
					WS_SetAll(White);
					isPhoneControl=TRUE;
					break;
				case 7:
					WS_SetAll(Black);
					isPhoneControl=TRUE;
					break;
				case 8: 
					timestrbuf = strtok(InstructionsStr, "#");
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.year);
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.month);
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.day);
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.hour);
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.minute);
					timestrbuf = strtok(NULL,"#");
					sscanf(timestrbuf,"%d",&RTCTime.second);
					RTC_Set(RTCTime.year,RTCTime.month,RTCTime.day,RTCTime.hour,RTCTime.minute,RTCTime.second);
					getSystemRTCTime(&RTCTime);     //获取系统RTC时钟 calendar结构体的值到RTCTime
					copyTime(&OldTime,&RTCTime);	//更新时间
					break;
				default:
					Gui_DrawFont_GBK16(16,100,GREEN,BLACK,"instruction error");
					break;
			}
		}	
	}
}


//把指令匹配写到中断里面，更稳定              //串口2中断服务程序
void USART2_IRQHandler(void)                	
{
	u8 Res;
	u8 i;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) ==SET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART2);	//读取接收到的数据
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else 
				{
					USART_RX_STA|=0x8000;	//接收完成了
					instructionReceiving();    //把指令匹配写到中断里面，更稳定
					for(i=0;i<InstructionsMaxLength;i++)  //清空串口显示字符串
					{
						USART_RX_BUF[i]='\0';
					}
					USART_RX_STA=0;	//清空串口接收标志
				} 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
	}
}
//关于按键的外部中断服务程序
//外部中断15_10服务程序 KEY1
void EXTI15_10_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY1==0)	 //按键KEY1
	{
		WS_SetAll(Black);//black
	}
	if(KEY2==0)	 //按键KEY2
	{
		WS_SetAll(White);//white 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line11);  //清除LINE11上的中断标志位
	EXTI_ClearITPendingBit(EXTI_Line10);  //清除LINE10上的中断标志位
}
//外部中断9_5服务程序 KEY3
void EXTI9_5_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY3==0)	 //按键KEY3
	{				 
		ColorToColor(Blue,Red);
	}
	if(KEY4==0)	  //按键KEY4
	{
		if(isPhoneControl==FALSE) isPhoneControl=TRUE;
		else if(isPhoneControl==TRUE) isPhoneControl=FALSE;
	}		 	
	EXTI_ClearITPendingBit(EXTI_Line9);  //清除LINE3上的中断标志位  
	EXTI_ClearITPendingBit(EXTI_Line8);  //清除LINE8上的中断标志位 
}

//外部中断0服务程序  KEY5
void EXTI0_IRQHandler(void)
{
	delay_ms(10);//消抖
	if(KEY5==1)	 	 //WK_UP按键
	{				 
		/*
		if(WiFiEN==1) WiFi_Disable();
		else if(WiFiEN==0) WiFi_Enable();
		*/
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位  
}
int main(void)
{	
/************************************************************************************************/
/**************************************硬件初始化区**********************************************/ 
	delay_init();	    	 					//延时函数初始化	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断分组2 
	RTC_Init();                                 //RTC初始化
	LED_Init();		  							//初始化与LED连接的硬件接口
	//WiFi_Init();                                //初始化wifi模块IO引脚
	WS_Init();									
	WS_Reset();									//WS2812B 初始化
	WS_SetAll(Black);							//初始为黑色 即关闭
	KEY_Init();       							//初始化键盘KEY1~KEY5
	EXTIX_Init();                               //外部中断初始化KEY1~KEY5采用外部中断方式
	BEEP_Init();								//初始化蜂鸣器硬件接口
	PUMP_Init();								//初始化水泵引脚
	DHT11_Init();								//初始化DHT11引脚
	WaterSensor_Init();							//水位传感器ADC初始化
	BatteryADC_Init();                          //电池电压AD转换初始化
	IWDG_Init(4,3125);    						//分频值4*2^pre=64  重载值3215  溢出时间5s
	SHT2x_Init();								//土壤温湿度传感器初始化 sht20
	SHT2x_SoftReset();
	//SHT11_GPIO_Config();  					//土壤温湿度传感器初始化 sht11
	//SHT11_Reset();
	BH1750_Init();								//光强传感器引脚和iic初始化
	//TIM3_PWM_Init(1999,719);	 				//舵机PWM频率=72000000/（2000*720）=50hz
	Lcd_Init();									//初始化LCD
	uart_init(9600);							//串口初始化 波特率9600  wifi模块内设置的9600bps
/**************************************硬件初始化区**********************************************/ 
/************************************************************************************************/
	 
	LCD_Display();
	beeping_ms(200);		//初始化成功 蜂鸣器短响
	//SoilState();			//读取土壤湿度温度状态并显示，作出响应
	SoilState_sht20(); 		//读取土壤湿度温度状态并显示，作出响应
	AirState();				//读取空气温湿度状态并显示
	WaterSensorADC();		//获取水位传感器的值
	getBatteryADC();        //获取电池电压
	illuminationState();	//读取光强传感器数据
	getSystemRTCTime(&RTCTime);     //获取系统RTC时钟 calendar结构体的值到RTCTime
	copyTime(&OldTime,&RTCTime);	//更新时间
	while(1)
	{
		
		if(JudgeTime(&OldTime,&RTCTime,2)==TRUE) //5s发送一次数据
		{
			sendSensorStateToWifi();	//读取所有传感器的值并通过WiFi发送数据
			copyTime(&OldTime,&RTCTime);	//更新时间
			//beeping_ms(100);
			LED2=!LED2;
		}
		
		
		//sendSensorStateToWifi();	//读取所有传感器的值并通过WiFi发送数据
		//instructionReceiving();	//指令接收 已经放到串口中断中接收
		//USART_RX_STA=0;
		//必须清空串口接收标志 且sendSensorStateToWifi()和instructionReceiving()的顺序不能乱 未知bug
		
		showRTCTime();              //显示RTC时钟
		//usartReceive();
		delay_ms(500);
		IWDG_ReloadCounter();//重置看门狗计数器 即喂狗
	}
}
