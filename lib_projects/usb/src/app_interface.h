/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : app_interface.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : CH376应用程序接口
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#ifndef _APP_INTERFACE_H_
#define _APP_INTERFACE_H_

#include "../../dwin_51/src/sys.h"

#include "string.h"
#include "stdio.h"

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
#define FILE_XXX_LIB					(0x03)	/* 升级文件选择 XXX.LIB */
#define FILE_XXX_BIN					(0x04)	/* 升级文件选择 XXX.BIN */
#define FILE_XXX_ICL					(0x05)	/* 升级文件选择 XXX.ICL */
#define FILE_ALL						(0x5A)	/* 整体系统升级 */
#define FLAG_ALL						(0x0FFF)	/*  */
#define DWIN_DIR						("/DWIN_SET")	/* DWIN存放升级文件的文件夹 */
#define MATCH_LIST_SIZE					(40)	/* 匹配文件列表的文件数目 */
/* 文件Flash地址计算 */
#define ADDR_T5L51_BIN		(UINT32)(0x10000);							/* T5L51*.BIN 文件地址 */
#define ADDR_DWIN_OS		(UINT32)(0x18000);							/* DWINOS* 文件地址 */
#define LIB(x)				(UINT32)((0x30 + x) << 12)			/* *.LIB 库文件地址 */
#define FONT(x)				(UINT32)(x << 18)	/* XX*.BIN 字库文件地址 */
#define ICL(x)				(UINT32)(x << 18)	/* XX*.ICL 图片文件地址 */
/* 升级使能标记 */
#define FLAG_EN				(0x5A)
#define FLAG_NO_EN			(0x00)
/* 升级空间 */
#define SPACE_1				(0x00)
#define SPACE_2				(0x01)
#define SPACE_3				(0x02)
#define SPACE_4				(0x03)
/* 升级操作相关地址 */
#define ADDR_UP_EN			(0x438)
#define ADDR_UP_TIME		(0x439)
#define ADDR_UP_SPANCE1		(0x43A)
#define ADDR_UP_SPANCE2		(0x43E)
#define ADDR_UP_SPANCE3		(0x442)
#define ADDR_UP_SPANCE4		(0x446)
#define ADDR_UP_CONFIG		(0x44A)
/* 升级包大小定义 控制+数据 = 521B + 4096B */
#ifndef BUF_SIZE
#define BUF_SIZE			(0x1000)
#endif
#ifndef CONTROL_SIZE
#define CONTROL_SIZE		(0x200)
#endif

#define PATH_FILE			(0x55)
#define PATH_DIR			(0xAA)

typedef struct _DIR_TYPE
{
	UINT8  DIR_Attr;
	UINT16 DIR_CrtTime;
	UINT16 DIR_CrtDate;
	UINT16 DIR_WrtTime;
	UINT16 DIR_WrtDate;
	UINT32 DIR_FileSize;
}DIR_TYPE, *P_DIR_TYPE;	
/********************************对外函数声明*********************************/

UINT8 USBInit(void);												/* 检测CH376通讯、设置USB工作模式、磁盘初始化 */
UINT8 CheckIC(void);
UINT8 CheckConnect(void);
UINT8 CheckDiskInit(void);
UINT8 CH376CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath);
UINT8 CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath);			/* 创建新文件或者目录 */
UINT8 RmFileOrDir(PUINT8 pPathName);								/* 删除文件或者目录 */
UINT8 ReadFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset);	/* 读取文件信息 */
UINT8 WriteFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset);	/* 写入文件、不存在则新建 */
UINT8 MatchFile(PUINT8 pDir,PUINT8 pMatchString, PUINT8 pBuf);
UINT8 SystemUpdate(UINT8 FileType, UINT16 FileNumber);				/* 系统升级 */
UINT8 GetFileMessage(PUINT8 pFilePath, PUINT8 pBuf);
UINT8 SetFileMessage(PUINT8 pFilePath, PUINT8 pBuf);

void SysUpGetFileMesg(UINT8 FileType, UINT16 FileNumber, PUINT8 pUpSpace, PUINT32 FileAddr, PUINT8 String);
UINT8 SysUpGetDWINFile(PUINT8 pMatchList);
void NumberStringMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pCount);
UINT8 SysUpFileMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pResult, PUINT32 pFileSize);
void SendUpPackToDGUS(UINT32 AddrDgusHead, UINT32 AddrDgusMesg, PUINT8 BufHead, PUINT8 BufMesg, UINT16 MesgSize);
void SysUpPcakSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);
void SysUpFileSend(PUINT8 pPath, UINT8 UpSpace, UINT32 AddrDgusPck,UINT32 AddrFileSave, UINT32 FileSize);
void SysUpWaitOsFinishRead(UINT32 AddrDgus);

#endif