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
#include "lib.h"
#include "string.h"

void PageDriver(PUINT8 pNumber, UINT8 Type);

void DgusRegConfig(void)
{
	UINT8 xdata Cmd[16];
	UINT8 xdata filename[0x100];
	memset(filename, 0, sizeof(filename));
	memset(Cmd, 0, sizeof(Cmd));
	//配置：创建或者删除文件
	Cmd[0] = 0x55;
	Cmd[1] = 0xE0;
	Cmd[2] = 0x00;
	WriteDGUS(0x5C5, Cmd, 4);
	//配置：获取文件列表
	filename[0] = '*';
	WriteDGUS(0xE040, filename, 3);
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE0;
	Cmd[3] = 0x40;
	Cmd[4] = 0xE0;
	Cmd[5] = 0x48;
	WriteDGUS(0x5D1, Cmd, 6);
	//配置：读写文件
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE2;
	Cmd[3] = 0x00;
	Cmd[4] = 0x00;
	Cmd[5] = 0x00;
	Cmd[6] = 0x00;
	Cmd[7] = 0x10;
	Cmd[8] = 0x00;
	WriteDGUS(0x5C9, Cmd, 10);
	//配置：文件属性获取或设置
	Cmd[0] = 0xE0;
	Cmd[1] = 0x00;
	Cmd[2] = 0xE1;
	Cmd[3] = 0x88;
	WriteDGUS(0x5C1, Cmd, 4);
	//SystemUP
	Cmd[0] = 0xFF;
	Cmd[1] = 0x00;
	WriteDGUS(0x5D5, Cmd, 2);
}

/*****************************************************************************
 函 数 名  : 文件属性DGUS显示
 功能描述  : 系统升级驱动
 输入参数  : 无
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void MesseageShow(void)
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

void PageAck(void)
{
	UINT8 xdata Cmd[10];
	UINT8 xdata Path[64];
	UINT8 PageNumber = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, sizeof(Path));
	
	ReadDGUS(0x5DA, Cmd, sizeof(Cmd));
	ReadDGUS(0xE000, Path, sizeof(Path));
	
	PageNumber = Cmd[9];
	if (Cmd[0] == 0x5A) PageDriver(&PageNumber, PAGE_UP);
	if (Cmd[1] == 0x5A) PageDriver(&PageNumber, PAGE_DOWN);
	Cmd[9] = PageNumber;
	if (Path[0] != '/') 
	{
		Path[0] = '/';
		WriteDGUS(0xE000, Path, 1);
	}
	memset(Cmd, 0, sizeof(Cmd)-1);
	WriteDGUS(0x5DA, Cmd, sizeof(Cmd));
}

void PageDriver(PUINT8 pNumber, UINT8 Type)
{
	UINT8 xdata FileList[0x320];
	UINT8 xdata FileShow[0x320];
	PUINT8 pFileList = FileList;
	UINT16 Offset = 0;
	UINT8 i = 0;
	memset(FileList, 0, sizeof(FileList));
	memset(FileShow, 0, sizeof(FileShow));
	AckSearchFile();
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