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
void main()
{
	UINT8 j = 0;
	UINT32 i = 0;
	PUINT8 pCOUNT = NULL;
	UINT32 Addr = 0;
	UINT16 Size = 0;
	
	UINT32 Buf_Size = 0;
	UINT8 xdata filename[0x1000];
	UINT8 xdata Cmd[16];	
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	memset(filename, 0, sizeof(filename));
	memset(Cmd, 0, sizeof(Cmd));
	//配置：创建或者删除文件
	Cmd[0] = 0x55;
	Cmd[1] = 0xE0;
	Cmd[2] = 0x00;
	WriteDGUS(0x5C5, Cmd, 4);
	//配置：获取文件列表
	filename[0] = '*';
	WriteDGUS(0xE040, filename, 3);
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE0;
	Cmd[3] = 0x40;
	Cmd[4] = 0xE0;
	Cmd[5] = 0x48;
	WriteDGUS(0x5D1, Cmd, 6);
	//配置：读写文件
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE2;
	Cmd[3] = 0x00;
	Cmd[4] = 0x00;
	Cmd[5] = 0x00;
	Cmd[6] = 0x00;
	Cmd[7] = 0x10;
	Cmd[8] = 0x00;
	WriteDGUS(0x5C9, Cmd, 10);
	//配置：文件属性获取或设置
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE1;
	Cmd[3] = 0x88;
	WriteDGUS(0x5C1, Cmd, 4);
	//SystemUP
	Cmd[0] = 0xFF;
	Cmd[1] = 0x00;
	WriteDGUS(0x5D5, Cmd, 2);
	
	USBModule();
	MesseageShow();
	//SystemUpdate(5, 32);
	//while(1);
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