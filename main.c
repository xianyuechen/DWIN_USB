/******************************************************************************
*
*                  版权所有 (C), 2019, 北京迪文科技有限公司
*
*******************************************************************************
* 文 件 名   : main.c
* 版 本 号   : V1.0
* 作    者   : chenxianyue
* 生成日期   : 2019年6月4日
* 功能描述   : 主函数，外设和参数初始化，主循环中主要功能函数入口。
* 修改历史   :
* 日    期   : 
* 作    者   :
* 修改内容   : 
******************************************************************************/
#include "t5los8051.h"
#include "driver/system/sys.h"
#include "app/app_dgus/usb_dgus.h"
#include "driver/usb/para_port.h"
#include "app/app_usb/file_sys.h"
#include "driver/dgus/dgus.h"
#include "driver/uart/uart.h"
#include "dgus_config.h"
#include "string.h"
#include "stdio.h"
void Delay(void);
UINT16 TIMES = 0;
void main()
{
	UINT8 xdata FileName[64] = {0};
	UINT8 xdata cmd[4] = {0};
	UINT16 NameLen = 64;
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	DgusRegConfig();
	USBModule();
	MesseageShow();
	PageClickAck();
	BackToPreviousAck();/*
	ReadDGUS(0x5C4, cmd, 4);
	if (cmd[0] == 0x5A || cmd[0] == 0xA5)
	{
		UART5_SendString("11111111");
		AckCreateOrDelPath();
		
		ReadDgusClientString(0xE000, FileName, &NameLen);
		FileName[NameLen] = 0;
		UART5_SendString(FileName);
		CreateFileOrDir(FileName, 0x55);
		cmd[0] = 0;
		WriteDGUS(0x5C4, cmd, 4);
		
	}*/
}

void T0_ISR_PC(void)	interrupt 1
{
	EA = 0;
	TIMES++;
	TH0 = T1MS >> 8;
    TL0 = T1MS;
	if (TIMES == 100)
	{
		TIMES = 0;
	}		
	EA = 1;
}