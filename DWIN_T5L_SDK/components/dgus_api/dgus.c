/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : dgus.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 提供单片机访问DGUS变量接口 文件升级相关的接口定义
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "include/dgus.h"

/********************************对内函数声明*********************************/

/********************************函数定义开始*********************************/

/*****************************************************************************
 函 数 名  : ReadDGUS
 功能描述  : 读DGUS寄存器
 输入参数  : UINT32 Addr  DGUS寄存器地址
             PUINT8 pBuf  接收缓冲区
			 UINT16 Len   读取数据字节长度	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void ReadDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset = 0;
	UINT8 EndLen = 0;
	Offset = Addr & 0x01;
	Addr = Addr >> 1;
	ADR_H = (UINT8)(Addr >> 16);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_L = (UINT8)(Addr);
	ADR_INC = 1;
	RAMMODE = 0xAF;
	while (!APP_ACK);
	if (Offset == 1)			/* 地址偏移修正 */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA1;
		if (Len == 1) return;
        *pBuf++ = DATA0;
		Len -= 2;
	}
	EndLen = Len & 0x03;		/* 尾部长度 0-3 */
	Len = Len >> 2;
	while (Len--)
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		*pBuf++ = DATA2;
		*pBuf++ = DATA1;
		*pBuf++ = DATA0;
	}
	if (EndLen)					/* 尾部有数据 根据数据长度选择读取次数 */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		if (EndLen & 2) *pBuf++ = DATA2;
		if (EndLen & 3) *pBuf++ = DATA1;
	}
	RAMMODE = 0x00;	 
}

/*****************************************************************************
 函 数 名  : WriteDGUS
 功能描述  : 写DGUS寄存器
 输入参数  : UINT32 Addr  DGUS寄存器地址
             PUINT8 pBuf  发送缓冲区
			 UINT16 Len   发送数据字节长度	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void WriteDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset;
	UINT8 EndLen;
	Offset = Addr & 1;
	Addr >>= 1;
	ADR_L = (UINT8)(Addr);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_H = (UINT8)(Addr >> 16);
	ADR_INC = 1;
	RAMMODE = 0x8F;
	while (!APP_ACK);
	if (Offset == 1)			/* 地址偏移修正 */
	{
		ADR_INC = 0;
        RAMMODE = 0xAF;
        APP_EN = 1;
        while (APP_EN);			/* 读写执行结束	*/

        ADR_INC = 0x01; 
        RAMMODE = 0x8F;     
        DATA1 = *pBuf++;
		if (Len == 1) return;
        DATA0 = *pBuf++;
        APP_EN = 1;
        while (APP_EN);			/* 读写执行结束	*/
        Len -= 2;
	}
	EndLen = Len & 0x03;		/* 尾部长度 0-3 */
	Len >>=2;
	while (Len--)
	{
		DATA3 = *pBuf++;
		DATA2 = *pBuf++;
		DATA1 = *pBuf++;
		DATA0 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	if (EndLen)					/* 尾部有数据 根据数据长度选择写入次数 */
	{
		DATA3 = *pBuf++;
		if (EndLen & 2) DATA2 = *pBuf++;
		if (EndLen & 3) DATA1 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	RAMMODE = 0x00;
}