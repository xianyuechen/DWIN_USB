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