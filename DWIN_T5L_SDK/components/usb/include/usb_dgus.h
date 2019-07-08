/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : usb_dgus.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年7月8日
功能描述   : USB接口相关的的DGUS应用程序实现
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#ifndef _USB_DGUS_H_
#define _USB_DGUS_H_

#include "../../usb/include/app_interface.h"
#include "../../dgus_api/include/dgus.h"
#include "../../uart/include/uart.h"
#include "../../system/include/sys.h"
#include "string.h"
#include "stdio.h"

/********************************对外函数声明*********************************/

void USBModule(void);			/* USB模块初始化 */
void MesseageShow(void);		/* 文件属性DGUS显示 */

#endif