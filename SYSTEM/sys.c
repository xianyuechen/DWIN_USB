#include "sys.h"

void INIT_CPU(void)
{
	EA			= 0;
	CKCON		= 0x00;
	T2CON		= 0x70;
	DPC			= 0x00;
	PAGESEL		= 0x01;
	D_PAGESEL	= 0x02;
	MUX_SEL		= 0x00;
	PORTDRV		= 0x01;		/* 驱动强度+/-8mA */
	RAMMODE		= 0x00;

	IEN0		= 0x00;     /* 关闭所有中断	*/
	IEN1		= 0x00;
	IEN2		= 0x00;
	IEN3		= 0x00;
	IP0			= 0x00;		/* 中断优先级默认 */
	IP1			= 0x00;	

	/* UART5配置8N1      115200	*/
	SCON3T = 0x80;
	SCON3R = 0x00;
	BODE3_DIV_H = 0x00;
	BODE3_DIV_L = 0xE0;


	/* 定时器0初始化 */
	TMOD |= 0x01;
	TH0 = T1MS >> 8;
	TL0 = T1MS;
	ET0=1;
	TR0=1;
}