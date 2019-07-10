/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : sys.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 系统寄存器配置相关
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/

#ifndef _SYS_H_
#define _SYS_H_

/********************************宏定义***************************************/
#ifndef UINT8
typedef unsigned char	UINT8;
#endif
#ifndef UINT16
typedef unsigned int	UINT16;
#endif
#ifndef UINT32
typedef unsigned long int	UINT32;
#endif
#ifndef INT8
typedef char            INT8;
#endif
#ifndef INT16
typedef int           	INT16;
#endif
#ifndef INT32
typedef long int		INT32;
#endif
#ifndef PUINT8
typedef unsigned char	*PUINT8;
#endif
#ifndef PUINT16
typedef unsigned int	*PUINT16;
#endif
#ifndef PUINT32
typedef unsigned long int	*PUINT32;
#endif
#ifndef UINT8V
typedef unsigned char volatile	UINT8V;
#endif
#ifndef PUINT8V
typedef unsigned char volatile	*PUINT8V;
#endif

#define DWIN_OK							(0x00)
#define DWIN_ERROR						(0xFF)
#define DWIN_NULL_POINT					(0x02)				/* 空指针 */
#define DWIN_PARAM_ERROR				(0x03)
#define NULL							((void *)0)			/* 数据NULL */
#define FOSC							(206438400UL)		/* T5L主频 */
#define T1MS    						(65536-FOSC/12/1000)/* 1MS定时器 */

/********************************对外函数声明*********************************/

void INIT_CPU(void);	/* 寄存器配置初始化 */

#endif