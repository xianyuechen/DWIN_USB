/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : dgus.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 提供单片机访问DGUS变量接口 文件升级相关的接口定义
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "t5los8051.h"
#include "system/sys.h"
#include "app_interface.h"
#include "system/uart.h" 
#ifndef _DGUS_H_
#define _DGUS_H_

/* 文件Flash地址计算 */
#define ADDR_T5L51_BIN		(UINT32)(0x10000);
#define ADDR_DWIN_OS		(UINT32)(0x18000);
#define LIB(x)				(UINT32)(0x30000 + (x * 4) << 10)
#define WORD_LIB(x)			(UINT32)(0x001 << 20 + (0x100 * x) << 10)
#define BMP(x)				(UINT32)(0x040 << 20 + (0x080 * x) << 10)

#define FLAG_EN				(0x5A)
#define FLAG_NO_EN			(0x00)
#define SPACE_1				(0x00)
#define SPACE_2				(0x01)
#define SPACE_3				(0x02)
#define SPACE_4				(0x03)

#define ADDR_UP_EN			(0x438)
#define ADDR_UP_TIME		(0x439)
#define ADDR_UP_SPANCE1		(0x43A)
#define ADDR_UP_SPANCE2		(0x43E)
#define ADDR_UP_SPANCE3		(0x442)
#define ADDR_UP_SPANCE4		(0x446)
#define ADDR_UP_SET			(0x44A)

#ifndef BUF_SIZE
#define BUF_SIZE			(0x1000)
#endif
#ifndef CONTROL_SIZE
#define CONTROL_SIZE		(0x200)
#endif

#define UPDATE_FAILED		(0xFF)

void ReadDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len);	/* 读DGUS Addr地址 Len数据字节长度 存入Buf */
void WriteDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len);	/* 写DGUS Addr地址 Len数据字节长度 发送Buf */
UINT8 SystemUpdate(UINT8 FileType);
#endif