/******************************************************************************
*																			  *
*                  版权所有 (C), 2019, 北京迪文科技有限公司					  *
*																			  *
*******************************************************************************
* 文 件 名   : uart.c													      *
* 版 本 号   : V1.0															  *
* 作    者   : chenxianyue													  *
* 生成日期   : 2019年6月4日													  *
* 功能描述   : 串口5中断打印字符串			  *
* 修改历史   :																  *
* 日    期   : 																  *
* 作    者   : 																  *
* 修改内容   : 																  *
******************************************************************************/

#include "t5los8051.h"
#include "sys.h"

#ifndef _UART_H_
#define _UART_H_

void UART5_Init(void);

void UART5_Sendbyte(UINT8 dat);

void UART5_SendString(PUINT8 String);

UINT8 UART5_Recivebyte(void);

void SendString(PUINT8 String, UINT32 BUFFSIZE);

#endif