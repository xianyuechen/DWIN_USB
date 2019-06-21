/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : app_interface.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月4日
功能描述   : CH376应用程序接口
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "para_port.h"
#include "ch376.h"
#include "file_sys.h"
#include "string.h"
#include "system/uart.h"

#ifndef _APP_INTERFACE_H_
#define _APP_INTERFACE_H_

/********************************宏定义***************************************/
#define	USB_DEVICE_OFF					(0x00)	/* 未启用USB设备方式 */
#define	USB_DEVICE_ON_OUTSIDE_FIRMWARE	(0x01)	/* 已启用USB设备方式,外部固件模式(串口连接方式不支持) */
#define	USB_DEVICE_ON_INSIDE_FIRMWARE	(0x02)	/* 已启动USB设备方式 ,内置固件模式 */
#define SD_HOST							(0x03)	/* SD卡主机模式 */
#define USB_HOST_OFF					(0x04)	/* 未启用USB主机方式 */
#define USB_HOST_ON_NO_SOF				(0x05)	/* 已启用USB主机方式,不自动产生SOF包 */
#define USB_HOST_ON_SOF					(0x06)	/* 已启用USB主机方式,自动产生SOF包 */
#define USB_HOST_ON_RESET_USB			(0x07)	/* 已启用USB主机方式,复位USB总线 */
#define BUF_SIZE						(0x1000)/* 缓冲区BUF包大小 越大扇区读写越快 由单片机RAM大小决定 */
#define	WRITE_FROM_HEAD					(0x00)	/* 扇区写方式可选项：从文件头部写 */
#define WRITE_FROM_END					(0x01)	/* 扇区写方式可选项：从文件尾部写 */ 
#define FILE_T5L51_BIN					(0x01)	/* 升级文件选择 T5L51.BIN */
#define FILE_DWINOS_BIN					(0x02)	/* 升级文件选择 DWINOS.BIN */
#define FILE_XXX_LIB					(0x04)	/* 升级文件选择 XXX.BIN */
#define FILE_ALL						(FILE_T5L51_BIN | FILE_DWINOS_BIN |FILE_XXX_LIB)	/* 整体系统升级 */
#define DWIN_DIR						("/DWIN_SET")	/* DWIN存放升级文件的文件夹 */
#define MATCH_LIST_SIZE					(10)	/* 匹配文件列表的文件数目 */
/********************************对外函数声明*********************************/

UINT8 CH376HostInit(void);

UINT8 Query376Interrupt(void);

UINT8 CH376TouchNewFile(PUINT8 PathName);

UINT8 CH376TouchDir(PUINT8 PathName);

UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize);

UINT8 CH376WriteFile(PUINT8 pPathName, PUINT8 pBuf, UINT8 Flag);

UINT8 FindDWINFile(PUINT8 MatchString);

#endif