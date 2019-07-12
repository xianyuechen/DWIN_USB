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
#include "app/app_dgus/usb_dgus.h"
#include "driver/usb/para_port.h"
#include "driver/uart/uart.h"
#include "dgus_config.h"

void Delay(void);
unsigned int  TIMES = 0;
void main()
{
	INIT_CPU(); 	
	CH376_PORT_INIT();
	UART5_Init();
	DgusRegConfig();	//以下三个函数初始化次序不能变 配置寄存器-USB扫描-DGUS演示DEMO
	USBModule();
	DGUSDemoInit();
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