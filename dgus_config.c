/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : dgus_config.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年7月9日
功能描述   : USB DEMO的DGUS应用程序配置
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "dgus_config.h"
#include "string.h"
#include "stdio.h"
#include "driver/system/sys.h"
#include "app/app_usb/app_interface.h"
#include "app/app_usb/usb_dgus.h"
#include "app/app_usb/file_sys.h"
#include "driver/dgus/dgus.h"

/********************************宏定义***************************************/
#define FILE			(0x14)
#define DIR				(0x41)
#define PAGE_UP			(0x5A)
#define PAGE_DOWN		(0xA5)

/********************************对内函数声明*********************************/

static void PageDriver(PUINT8 pNumber, UINT8 Type);
static void ClickPathUpdate(UINT8 Number);
static UINT8 GetPathAttr(PUINT8 pPath);
static void DelPath(void);
static void MesseageShow(void);
static void PageClickAck(void);
static void BackToPreviousAck(void);

/********************************函数定义开始*********************************/

/*****************************************************************************
 函 数 名  : DGUSDemoInit
 功能描述  : DGUS Demo相关初始化
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void DGUSDemoInit(void)
{
	MesseageShow();
	PageClickAck();
	BackToPreviousAck();
}

/*****************************************************************************
 函 数 名  : DgusRegConfig
 功能描述  : DGUS Demo 寄存器配置
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void DgusRegConfig(void)
{
	UINT8 xdata Cmd[0x10];
	UINT8 xdata MatchString[0x04];
	memset(MatchString, 0, sizeof(MatchString));
	memset(Cmd, 0, sizeof(Cmd));
	/* 所有配置前两个字节都不配置，通过DGUS按钮触发 */
	//配置：创建或者删除文件
	Cmd[0] = 0x55;	/* 类型为文件 */
	Cmd[1] = 0xE0;	/* 路径保存DGUS地址 */
	Cmd[2] = 0x00;
	WriteDGUS(0x5C5, Cmd, 4);
	//配置：获取文件列表
	MatchString[0] = '*';
	WriteDGUS(0xE040, MatchString, 3);
	Cmd[0] = 0xC0;	/* 实际操作路径保存DGUS地址 */
	Cmd[1] = 0x00;
	Cmd[2] = 0xE0;	/* 路径保存DGUS地址 */
	Cmd[3] = 0x40;
	Cmd[4] = 0xE0;	/* 匹配列表保存DGUS地址 */
	Cmd[5] = 0x48;
	WriteDGUS(0x5D1, Cmd, 6);
	//配置：读写文件
	Cmd[0] = 0xE0;	/* 路径保存DGUS地址 */
	Cmd[1] = 0x00;
	Cmd[2] = 0xE2;	/* 文件信息保存DGUS地址 */
	Cmd[3] = 0x00;	
	Cmd[4] = 0x00;	/* 扇区偏移为0 */
	Cmd[5] = 0x00;
	Cmd[6] = 0x00;
	Cmd[7] = 0x10;	/* 4096个字节 */
	Cmd[8] = 0x00;
	WriteDGUS(0x5C9, Cmd, 10);
	//配置：文件属性获取或设置
	Cmd[0] = 0xE0;	/* 路径保存DGUS地址 */
	Cmd[1] = 0x00;
	Cmd[2] = 0xE1;	/* 文件属性保存DGUS地址 */
	Cmd[3] = 0x88;
	WriteDGUS(0x5C1, Cmd, 4);
	//配置：系统升级
	Cmd[0] = 0xFF;	/* 整体升级 */
	Cmd[1] = 0x5A;	/* 升级完毕自动复位 */
	WriteDGUS(0x5D5, Cmd, 2);
}

/*****************************************************************************
 函 数 名  : MesseageShow
 功能描述  : 文件属性DGUS显示
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void MesseageShow(void)
{
	UINT8 xdata Ms[13], String[18];
	UINT16 DIR_CrtTime = 0, DIR_CrtDate = 0, DIR_WrtTime = 0, DIR_WrtDate = 0;
	UINT16 Hour = 0, Min = 0, Sec = 0, Year = 0, Month = 0, Day = 0;
	UINT32 DIR_FileSize = 0;
	
	memset(Ms, 0, sizeof(Ms));
	memset(String, 0, sizeof(String));
	ReadDGUS(0xE188, Ms, sizeof(Ms));
	DIR_CrtTime  = (Ms[1] << 8) | Ms[2];
	DIR_CrtDate  = (Ms[3] << 8) | Ms[4];
	DIR_WrtTime  = (Ms[5] << 8) | Ms[6];
	DIR_WrtDate  = (Ms[7] << 8) | Ms[8];
	/* 默认小端对齐，改为大端对齐 */
	DIR_FileSize = ((UINT32)Ms[9] << 24) | ((UINT32)Ms[10] << 16) | (UINT16)(Ms[11] << 8) | (Ms[12]);
	
	Hour = (DIR_CrtTime & 0xF800) >> 11;
	Min  = (DIR_CrtTime & 0x07E0) >> 5;
	Sec  = (DIR_CrtTime & 0x001F) << 1;
	sprintf(String, "%2d : %2d : %2d", Hour, Min, Sec);
	WriteDGUS(0xE1B0, String, sizeof(String));
	memset(String, 0, sizeof(String));
	
	Year  = ((DIR_CrtDate & 0xFE00) >> 9) + 1980;
	Month = (DIR_CrtDate & 0x01E0) >> 5;
	Day   = DIR_CrtDate & 0x001F;
	sprintf(String, "%4d - %2d - %2d", Year, Month, Day);
	WriteDGUS(0xE1A0, String, sizeof(String));
	memset(String, 0, sizeof(String));
	
	Hour = (DIR_WrtTime & 0xF800) >> 11;
	Min  = (DIR_WrtTime & 0x07E0) >> 5;
	Sec  = (DIR_WrtTime & 0x001F) << 1;
	sprintf(String, "%2d : %2d : %2d", Hour, Min, Sec);
	WriteDGUS(0xE1D0, String, sizeof(String));
	memset(String, 0, sizeof(String));
	
	Year  = ((DIR_WrtDate & 0xFE00) >> 9) + 1980;
	Month = (DIR_WrtDate & 0x01E0) >> 5;
	Day   = DIR_WrtDate & 0x001F;
	sprintf(String, "%4d - %2d - %2d", Year, Month, Day);
	WriteDGUS(0xE1C0, String, sizeof(String));
	memset(String, 0, sizeof(String));
	
	sprintf(String, "%lu Byte", DIR_FileSize);
	WriteDGUS(0xE1E0, String, sizeof(String));
}

/*****************************************************************************
 函 数 名  : PageClickAck
 功能描述  : 翻页操作
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void PageClickAck(void)
{
	UINT8 xdata Cmd[10];
	UINT8 xdata DgusShowPath[64];
	UINT8 SearchAckFilePath[2];
	UINT16 DgusShowLen = 0, SearchAckFileLen = 0; 
	UINT8 PageNumber = 0, Number = 0;
	
	DgusShowLen =  sizeof(DgusShowPath);
	SearchAckFileLen = sizeof(SearchAckFileLen);
	memset(Cmd, 0, sizeof(Cmd));
	memset(DgusShowPath, 0, DgusShowLen);
	memset(SearchAckFilePath, 0, SearchAckFileLen);
	
	ReadDGUS(0x5DA, Cmd, sizeof(Cmd));
	ReadDgusClientString(0xE000, DgusShowPath, &DgusShowLen);
	ReadDgusClientString(0xC000, SearchAckFilePath, &SearchAckFileLen);
	PageNumber = Cmd[9];
	if (Cmd[0] == 0x5A) PageDriver(&PageNumber, PAGE_UP);
	if (Cmd[1] == 0x5A) PageDriver(&PageNumber, PAGE_DOWN);
	Cmd[9] = PageNumber;
	if (DgusShowPath[0] != '/') 			/* 加根目录处理 */
	{
		DgusShowPath[0] = '/';
		WriteDgusClientString(0xE000, DgusShowPath, 1);
	}
	if (SearchAckFilePath[0] == 0)
	{										/* 第一次会把显示路径设置成工作路径 */
		WriteDgusClientString(0xC000, DgusShowPath, sizeof(DgusShowPath));
	}
	for (Number = 2; Number < 10; Number++) /* 与DGUS按钮响应对应, 0-1字节是上下翻页, 2-9字节是点击文件名 */
	{
		if (Cmd[Number] == 0x5A) ClickPathUpdate(Number - 2);
	}
	memset(Cmd, 0, sizeof(Cmd)-1);			/* 最后一个字符不写入，作为翻页记录 */
	WriteDGUS(0x5DA, Cmd, sizeof(Cmd));
}

/*****************************************************************************
 函 数 名  : PageDriver
 功能描述  : 翻页处理驱动：原理是重新获取文件列表 根据文件页码重新写入制定区域
 输入参数  : PUINT8 pNumber	：页码号
			 UINT8 Type		：翻页类型 上一页/下一页
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void PageDriver(PUINT8 pNumber, UINT8 Type)
{
	UINT8 xdata FileList[0x320];
	UINT8 xdata FileShow[0x320];
	PUINT8 pFileList = FileList;
	UINT16 Offset = 0;
	UINT8 i = 0;
	memset(FileList, 0, sizeof(FileList));
	memset(FileShow, 0, sizeof(FileShow));
	AckSearchFile();				/* 重新获取文件列表 */
	ReadDGUS(0xE048, FileList, sizeof(FileList));
	while(FileList[(i++) * 140] != 0) if (i == 6) break;
	i -= 2;
	if (Type == PAGE_DOWN) (*pNumber)++;
	if (Type == PAGE_UP) (*pNumber)--;
	if (*pNumber > i) *pNumber = 0;
	Offset = 20 * 7 * (*pNumber);
	pFileList += Offset;
	memcpy(FileShow, pFileList, sizeof(FileList) - Offset);
	WriteDGUS(0xE048, FileShow, sizeof(FileShow));
}

/*****************************************************************************
 函 数 名  : ClickPathUpdate
 功能描述  : 点击文件名自动刷新页面, 文件夹则进入文件夹, 文件则不会进入, 自动
			 把名字加到路径上, 文件名为文件夹则会写入到工作路径, 否则只会写入
			 到显示路径
 输入参数  : UINT8 Number	：被点击的文件序号, 从上到下
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void ClickPathUpdate(UINT8 Number)
{
	UINT8 xdata Path[64];
	UINT8 xdata FirstPageList[140];
	UINT16 PathLength = 0;
	memset(Path, 0, sizeof(Path));
	memset(FirstPageList, 0, sizeof(FirstPageList));
	PathLength = sizeof(Path);
	ReadDgusClientString(0xC000, Path, &PathLength);	/* 存放刷新列表所在路径的地方 */
	ReadDGUS(0xE048, FirstPageList, sizeof(FirstPageList));
	if (Path[PathLength - 1] != '/') 
	{
		Path[PathLength] = '/';
		Path[PathLength + 1] = 0;
	}
	strcat(Path, &FirstPageList[Number * 20]);
	if (Path[strlen(Path) - 1] == '/')
	{
		Path[strlen(Path) - 1] = 0;
		Path[strlen(Path)] = 0;
	}
	if (DIR == GetPathAttr(Path))
	{
		WriteDgusClientString(0xC000, Path, strlen(Path));
		AckSearchFile();
	}
	WriteDgusClientString(0xE000, Path, strlen(Path));
}

/*****************************************************************************
 函 数 名  : GetPathAttr
 功能描述  : 获取路径属性 用以简单判断是文件还是文件夹
 输入参数  : PUINT8 pPath	：路径
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static UINT8 GetPathAttr(PUINT8 pPath)
{
	UINT8 Status = 0;
	AlphabetTransfrom(pPath);
	Status = CH376FileOpenPath(pPath);
	CH376CloseFile(0);
	return Status;
}

/*****************************************************************************
 函 数 名  : BackToPreviousAck
 功能描述  : 返回上一级路径响应, 直接跳到上一级文件夹
 输入参数  : PUINT8 pPath	：路径
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void BackToPreviousAck(void)
{
	UINT8 Cmd[4];
	memset(Cmd, 0, sizeof(Cmd));
	ReadDGUS(0x5E0, Cmd, sizeof(Cmd));
	if (Cmd[0] == 0x5A)
	{
		DelPath();
		Cmd[0] = 0x00;
		WriteDGUS(0x5E0, Cmd, sizeof(Cmd));
	}
}

/*****************************************************************************
 函 数 名  : DelPath
 功能描述  : 删除路径中的最后一级文件夹字符串 esp:/DWIN_SET/T5L51.BIN -> /
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月15日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void DelPath(void)
{
	UINT8 xdata DgusShowPath[64];
	UINT8 i = 0, PathType;
	UINT16 DgusShowLen = 0;
	
	DgusShowLen = sizeof(DgusShowPath);
	memset(DgusShowPath, 0, DgusShowLen);
	ReadDgusClientString(0xE000, DgusShowPath, &DgusShowLen);
	PathType = GetPathAttr(DgusShowPath);
	if (PathType == FILE)
	{
		for(i = DgusShowLen; i > 1; i--)
		{
			if (DgusShowPath[i - 1] == '/')
			{
				DgusShowPath[i - 1] = 0;
				DgusShowLen = i;
				break;
			}
			DgusShowPath[i - 1] = 0;
		}	
	}
	for(i = DgusShowLen; i > 1; i--)
	{
		if (DgusShowPath[i - 1] == '/')
		{
			DgusShowPath[i - 1] = 0;
			DgusShowLen = i;
			break;
		}
		DgusShowPath[i - 1] = 0;
	}
	WriteDgusClientString(0xE000, DgusShowPath, DgusShowLen);	/* 写入到路径栏 */
	WriteDgusClientString(0xC000, DgusShowPath, DgusShowLen);	/* 写入到实际操作路径栏 */
	AckSearchFile();	/* 刷新一次文件列表 */
}