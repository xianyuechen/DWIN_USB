/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : uart.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月4日
功能描述   : 串口5中断打印字符串
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "uart.h"

void UART5_Init(void)
{
    SCON3T=0x80;
    SCON3R=0x80;
    BODE3_DIV_H=0x00;
    BODE3_DIV_L=0xE0;
}

void UART5_Sendbyte(UINT8 dat)
{	
	SBUF3_TX = dat;    
	while((SCON3T & 0x01) == 0);
	SCON3T = 0x80;    
}

void UART5_SendString(PUINT8 String)
{
	while(*String != '\0')
	{
		UART5_Sendbyte(*String++);	
	}
}

void SendString(PUINT8 String, UINT32 BUFFSIZE)
{
	UINT32 i = 0;
	for (i = 0; i < BUFFSIZE; i++)
	{
		UART5_Sendbyte(*String++);	
	}
}

UINT8 UART5_Recivebyte(void)
{
	UINT8 dat, i;
	dat = SBUF3_RX;
	while((SCON3R & 0x01) == 0)
	{
		i++;
		if (i == 200) break;
	}
	SCON3R &= 0xFE;
	return dat;	
}