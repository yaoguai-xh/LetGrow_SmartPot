#include "WS2812B.h"
#include "sys.h"
//ws2812B 的寄存器格式是 G R B
/* 显存 */                //G R B
//unsigned long WsDat[nWs]={0x110011,0x0C000C,0x030003,
//                          0x000000,0x000000,0x000000,
//                          0x001111,0x000C0C,0x000303,
//                          0x000000,0x000000,0x000000};

/**************************************************************************************
* IO初始化（移植时请修改）
**************************************************************************************/
void WS_Init()
{
	GPIO_InitTypeDef  GPIO_InitStructure;	
	//端口时钟，使能
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );	 

	// 端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				// PIN
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		// 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		// IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					// 根据设定参数初始化 
}

/**************************
* 内部延时
***************************/
void delay2us()
{
	unsigned char i;
	i=3;
	while(i)
	{
		i--;
	}
	
}
void delay05us()
{
	unsigned char i;
	i=1;
	while(i)
	{
		i--;
	}
}

/***************************
* 发送一比特
****************************/
void TX0()  	{ PBout(5) = 1; delay05us(); PBout(5) = 0; delay2us(); } // 发送0
void TX1()  	{ PBout(5) = 1; delay2us();  PBout(5) = 0; delay05us(); } // 发送1
void WS_Reset() { PBout(5) = 0; delay_us(60);PBout(5) = 1; PBout(5) = 0;   }

/**************************************************************************************
* 发送一字节
**************************************************************************************/
void WS_Set1(unsigned long dat)
{
	unsigned char i;
	
	for(i=0; i<24; i++)
	{
		if(0x800000 == (dat & 0x800000) )	
			TX1();
		else						
			TX0();
		dat<<=1;							//左移一位
	}
}

/**************************************************************************************
* 发送所有字节
**************************************************************************************/
void WS_SetAll(unsigned long color)
{
	static uint8_t k;
	unsigned char j;
	unsigned long temp;
	//R G B   --->   G R B
	color=(color&0xff0000)/0x100 + (color&0x00ff00)*0x100 + (color&0x0000ff);
	for(j=0; j<nWs; j++)
	{
		WS_Set1(color);  // j / 0
	}
	WS_Reset();
}
/********************************************
* 求绝对值
********************************************/
unsigned char abs0(int num)
{
	if(num>0) return num;
	
	num = -num;
	return (unsigned char) num;
}

/***********************************************************************************
* 颜色渐变算法
* 误差 <= 2
************************************************************************************/
u32 ColorToColor(unsigned long color0, unsigned long color1)
{
	unsigned char Red0, Green0, Blue0;  // 起始三原色
	unsigned char Red1, Green1, Blue1;  // 结果三原色
	int			  RedMinus, GreenMinus, BlueMinus;	// 颜色差（color1 - color0）
	unsigned char NStep; 							// 需要几步
	float		  RedStep, GreenStep, BlueStep;		// 各色步进值
	unsigned long color;							// 结果色
	unsigned char i,j;
	static uint32_t WsDatTemp; 
	// 绿 红 蓝 三原色分解
	Red0   = color0>>16; 
	Green0 = color0>>8;  
	Blue0  = color0;      
	
	Red1   = color1>>16;
	Green1 = color1>>8;
	Blue1  = color1;
	
	// 计算需要多少步（取差值的最大值）
	RedMinus   = (Red1 - Red0); 
	GreenMinus = (Green1 - Green0); 
	BlueMinus  = (Blue1 - Blue0);
	
	NStep = ( abs0(RedMinus) > abs0(GreenMinus) ) ? abs0(RedMinus):abs0(GreenMinus);
	NStep = ( NStep > abs0(BlueMinus) ) ? NStep:abs0(BlueMinus);
	
	// 计算出各色步进值
	RedStep   = (float)RedMinus   / NStep;
	GreenStep = (float)GreenMinus / NStep;
	BlueStep  = (float)BlueMinus  / NStep;

	// 渐变开始
	for(i=0; i<NStep; i++)
	{
		Red1   = Red0   + (int)(RedStep   * i);
		Green1 = Green0 + (int)(GreenStep * i);
		Blue1  = Blue0  + (int)(BlueStep  * i);
		
		color  = Green1<<16 | Red1<<8 | Blue1; 	// 合成  绿红蓝
		WsDatTemp = color;
		for(j=0; j<nWs; j++)
	{
		WS_Set1(WsDatTemp);  // j / 0
	}
	  WS_Reset();
		delay_ms(10);						// 渐变速度
	}
	// 渐变结束
	
	return color;
}





