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
UINT8 res = 0x00;
UINT32 DiskFree;
UINT32 NewSize;
UINT32 PRINT_EN = 0;
UINT16 TIMES = 0;
FAT_NAME MatchLish[DIR_FILE_MAX] = {0};
int main()
{
	UINT8 j = 0;
	UINT32 i = 0;
	UINT8 COUNT = 0;
	UINT32 Addr = 0;
	UINT16 Size = 0;
	
	UINT32 Buf_Size = 0;
	UINT8 xdata filename[64];
	UINT8 xdata Buf[BUF_SIZE];	
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	for (i = 0; i < DIR_FILE_MAX; i++)
		memset(MatchLish[i].NAME, 0, sizeof(MatchLish[i].NAME));
	memset(filename, 0, sizeof(filename));
	memset(Buf, 0, sizeof(Buf));
	strcpy(filename, "/dwin_set/t5l51.bin");
	/*strcpy(Buf, "11111111111111111111222222222222222222222222\n\
22222232323232323232333333333333333333232323\n\
22222222222222222222222222222222222222222222\n\
33333333333333333333333333333333333333333333\n\
44444444444444444444444444444444444444444444\n\
55555555555555555555555555555555555555555555\n\
66666666666666666666666666666666666666666666\n\
77777777777777777777777777777777777777777777\n\
22222232323232323232333333333333333333232323\n\
22222222222222222222222222222222222222222222\n\
33333333333333333333333333333333333333333333\n\
44444444444444444444444444444444444444444444\n\
55555555555555555555555555555555555555555555\n\
66666666666666666666666666666666666666666666\n\
77777777777777777777777777777777777777777777\n");*/
	/*strcpy(filename2, "XXXXXXXXXXXX.C");
	for ( j = 0; filename2[j] != 0; j++ ) {
			Buf[j*2] = filename2[j];
			Buf[j*2+1] = 0x00;
		} */
	//ReadDGUS(0x1000, filename, 2);
	
	//UART5_Sendbyte(filename[1]);
	//res = CH376USBInit();
	//SystemUpdate(FILE_T5L51_BIN);
	USBInit();

	//UART5_SendString("Start.\n");
	//CH376ReadFile(filename, Buf, 1024, 6);
	//CH376WriteFile(filename, Buf, strlen(Buf), WRITE_FROM_END);
	
	//UART5_SendString("End.\n");
	/*
	CH376CreateLongName(filename, Buf);
	CH376MatchFile("*", "/DWIN_SET", MatchLish);
	for (i = 0; i < DIR_FILE_MAX; i++)
	{
		if(MatchLish[i].NAME[0] == 0) break;
		UART5_SendString(MatchLish[i].NAME);
		UART5_Sendbyte('\n');
	} */
	//CH376TouchDir(filename); 
	/*
	if (1) CreateFileOrDir(filename, 0x55);
	else {
		UART5_Sendbyte(CH376FileOpen("/XXXXX"));
		UART5_Sendbyte(CH376FileOpen("FDAFDDS"));
		CH376CloseFile(0);
	}
	UART5_SendString("Over\n");*/
	//FindDWINFile(filename, "BIN");
	//CH376ReadFile(filename, Buf, &Buf_Size, 2);
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
		PRINT_EN++;
		TIMES = 0;
	}		
	EA = 1;
}