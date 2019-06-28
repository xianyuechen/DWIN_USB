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
#include "app/app_usb/app_interface.h"
#include "app/app_dgus/usb_dgus.h"
#include "driver/uart/uart.h"
#include "driver/usb/ch376.h"
#include "driver/usb/para_port.h"
#include "app/app_usb/file_sys.h"
#include "driver/dgus/dgus.h"
#include "string.h"
#include "stdio.h"

void Delay(void);
UINT16 TIMES = 0;
int main()
{
	UINT8 j = 0;
	UINT32 i = 0;
	UINT8 COUNT = 0;
	UINT32 Addr = 0;
	UINT16 Size = 0;
	
	UINT32 Buf_Size = 0;
	UINT8 xdata filename[128];
	UINT8 xdata Buf[BUF_SIZE];	
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	memset(filename, 0, sizeof(filename));
	memset(Buf, 0, sizeof(Buf));
	strcpy(filename, "/dwin_set");
	USBInit();
	AckSearchFile();
	/*
	ReadDGUS(0x5C4, Buf, 1);
	if (Buf[0] == 0x5A)
	{
		ReadDgusClientString(0xC000, filename, &Size);
		SendString(filename, Size);
		UART5_SendString("\n");
	}*/  
	/*
	ReadDgusClientString(0xC000, filename, &Size);
	if (filename[0] != 0)
	{
		
	}*/
	while(1);
	return 0;
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