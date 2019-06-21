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

#include "t5los8051.h"
#include "system/sys.h"
#ifndef _PARA_PORT_H_
#define _PARA_PORT_H_	

void CH376_PORT_INIT(void);			/* 初始化并口IO */
UINT8 xReadCH376Status(void);		/* 从CH376读状态 */
void xWriteCH376Cmd(UINT8 mCmd);  	/* 向CH376写命令 */
void xWriteCH376Data(UINT8 mData);	/* 向CH376写数据 */
UINT8 xReadCH376Data(void);			/* 从CH376读数据 */

#endif
