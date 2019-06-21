/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : para_port.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月4日
功能描述   : CH376并口初始化以及与T5L并口通讯实现
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "T5LOS8051.H"
#include "para_port.h"
#include "intrins.h"
#define CH376_DATA_DAT_OUT(data)	{P2 = data;}		/* 向并口输出数据 */
#define CH376_DATA_DAT_IN()			(P2)				/* 从并口输入数据 */
#define	CH376_DATA_DIR_OUT()		{P2MDOUT |= 0xFF;}	/* 设置并口方向为输出 */
#define	CH376_DATA_DIR_IN()			{P2MDOUT &= 0x00;}	/* 设置并口方向为输入 */

void mDelay0_5uS()
{
	UINT8 i;
	for(i = 0; i < 9; i++) _nop_();
}

void CH376_PORT_INIT(void)			/* 由于使用通用I/O模拟并口读写时序,所以进行初始化 */
{
	P2MDOUT |= 0x00;
	P1MDOUT |= 0x0F;
	CH376_CS = 1;
	CH376_WR = 1;
	CH376_RD = 1;
	CH376_A0 = 0;
	CH376_DATA_DIR_IN();			/* 设置并口输出 */
}

UINT8 xReadCH376Status(void)		/* 从CH376读状态 */
{
	UINT8 mData = 0;
	CH376_DATA_DIR_IN();
	CH376_WR = 1;
	CH376_RD = 0;
	CH376_A0 = 1;
	CH376_CS = 0;
	mData = CH376_DATA_DAT_IN();	/* 从CH376的并口输入数据 */
	CH376_WR = 1;
	CH376_RD = 1;
	CH376_A0 = 0;
	CH376_CS = 1;
	return mData;
}

void xWriteCH376Cmd(UINT8 mCmd)		/* 向CH376写命令 */
{
	CH376_DATA_DAT_OUT(mCmd);  		/* 向CH376的并口输出数据 */
	CH376_DATA_DIR_OUT();  			/* 设置并口方向为输出 */
	CH376_A0 = 1;
	CH376_CS = 0;
	CH376_WR = 0;  					/* 输出有效写控制信号, 写CH376芯片的命令端口 */
	CH376_CS = 0;  					/* 该操作无意义,仅作延时,CH376要求读写脉冲宽度大于40nS */
	CH376_WR = 1;  					/* 输出无效的控制信号, 完成操作CH376芯片 */
	CH376_CS = 1;
	CH376_A0 = 0;
	CH376_DATA_DIR_IN();  			/* 禁止数据输出 */
	mDelay0_5uS(); mDelay0_5uS(); mDelay0_5uS();  /* 延时1.5uS确保读写周期大于1.5uS,或者用状态查询代替 */
}

void xWriteCH376Data(UINT8 mData)	/* 向CH376写数据 */
{
	CH376_DATA_DAT_OUT(mData);  	/* 向CH376的并口输出数据 */
	CH376_DATA_DIR_OUT();  			/* 设置并口方向为输出 */
	CH376_A0 = 0;
	CH376_CS = 0;
	CH376_WR = 0;  					/* 输出有效写控制信号, 写CH376芯片的数据端口 */
	CH376_CS = 0;  					/* 该操作无意义,仅作延时,CH376要求读写脉冲宽度大于40nS */
	CH376_WR = 1;  					/* 输出无效的控制信号, 完成操作CH376芯片 */
	CH376_CS = 1;
	CH376_DATA_DIR_IN();  			/* 禁止数据输出 */
	mDelay0_5uS();  				/* 确保读写周期大于0.6uS */
}

UINT8 xReadCH376Data(void)			/* 从CH376读数据 */
{
	UINT8 mData;
	mDelay0_5uS();  /* 确保读写周期大于0.6uS */
	CH376_DATA_DIR_IN();  /* 设置并口方向为输入 */
	CH376_A0 = 0;
	CH376_CS = 0;
	CH376_RD = 0;  /* 输出有效读控制信号, 读CH376芯片的数据端口 */
	CH376_CS = 0;
	CH376_CS = 0;
	CH376_CS = 0;
	CH376_CS = 0;  /* 该操作无意义,仅作延时,CH376要求读写脉冲宽度大于40nS,强烈建议此处执行一条空指令延时以确保并口有足够时间输入数据 */
	mData = CH376_DATA_DAT_IN();  /* 从CH376的并口输入数据 */
	CH376_RD = 1;  /* 输出无效的控制信号, 完成操作CH376芯片 */
	CH376_CS = 1;
	return mData;
}