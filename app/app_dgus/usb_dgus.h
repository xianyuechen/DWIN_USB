/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : usb_dgus.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : USB接口相关的的DGUS应用程序实现
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#ifndef _USB_DGUS_H_
#define _USB_DGUS_H_

#include "app/app_usb/app_interface.h"
#include "driver/dgus/dgus.h"
#include "driver/uart/uart.h"
#include "driver/system/sys.h"
#include "string.h"
#include "stdio.h"

void USBModule(void);
UINT8 CompareDgusRegValue(UINT32 AddrDgus, UINT8 Value);
void ReadDgusClientString(UINT32 AddrDgus, PUINT8 pData, PUINT16 pDataLen);
void WriteDgusClientString(UINT32 AddrDgus, PUINT8 pData, UINT16 DataLen);
void AckCreateOrDelPath(void);
void AckSearchFile(void);
void AckReadOrWriteFile(void);
void AckGetOrSetPath(void);
void AckSystemUp(void);
void AckDiskInit(void);
void MesseageShow(void);

#endif