/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : usb_dgus.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月27日
功能描述   : USB接口相关的的DGUS应用程序实现
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/

#include "usb_dgus.h"
#include "app/app_usb/app_interface.h"
#include "driver/dgus/dgus.h"
#include "driver/uart/uart.h"
#include "string.h"


/********************************宏定义***************************************/
/* USB DGUS寄存器地址 */
#define DGUS_ADDR_GET_OR_SET_PATH		(0x5C0)
#define DGUS_ADDR_CREATE_OR_DEL_PATH	(0x5C4)
#define DGUS_ADDR_READ_OR_WRITE_FILE	(0x5C8)
#define DGUS_ADDR_SEARCH_FILE			(0x5D0)
#define DGUS_ADDR_SYSTEM_UP				(0x5D4)
/* USB 操作动作定义 */
#define ACK_GET_OR_SET_PATH				(0x05)
#define ACK_CREATE_OR_DEL_PATH			(0x04)
#define ACK_READ_OR_WRITE_FILE			(0x03)
#define ACK_SEARCH_FILE					(0x02)
#define ACK_SYSTEM_UP					(0x01)	
/* 标志位定义 */
#define FLAG_START						(0x5A)
#define FLAG_END						(0x00)
#define FLAG_READ						(0x5A)
#define FLAG_WRITE						(0xA5)
#define FLAG_CREATE						(0x5A)
#define FLAG_DELETE						(0xA5)
#define TYPE_FILE						(0x55)
#define TYPE_DIR 						(0xAA)

#define MATCH_LIST_NUM					(0x28)
#define MATCH_LIST_LEN					(0x280)
#define MATCH_STRING_LEN				(0x10)
#define PATH_LENGTH						(0x80)
#ifndef BUF_SIZE
#define BUF_SIZE						(0x1000)
#endif				
/*****************************************************************************
 函 数 名  : USBModule
 功能描述  : 唯一对外接口 自动扫描DGUS命令标志并执行相应操作
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月28日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void USBModule(void)
{
	UINT8 ACK = 0;
	/* (1) 扫描DGUS变量标志位 确定相应应答标志 后扫描的寄存器 执行优先级高 */
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_GET_OR_SET_PATH, FLAG_READ) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_GET_OR_SET_PATH, FLAG_WRITE))
		ACK = ACK_GET_OR_SET_PATH;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_CREATE_OR_DEL_PATH, FLAG_CREATE) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_CREATE_OR_DEL_PATH, FLAG_DELETE))
		ACK = ACK_CREATE_OR_DEL_PATH;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_READ_OR_WRITE_FILE, FLAG_READ) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_READ_OR_WRITE_FILE, FLAG_WRITE))
		ACK = ACK_READ_OR_WRITE_FILE;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SEARCH_FILE, FLAG_START))
		ACK = ACK_SEARCH_FILE;
	if (DWIN_OK == CompareDgusRegValue(ACK_SYSTEM_UP, FLAG_START))
		ACK = ACK_SYSTEM_UP;

	/* (2) 应答响应 */
	switch (ACK)
	{
		case ACK_SYSTEM_UP:
		{
			//AckSystemUp();
			break;
		}
		case ACK_SEARCH_FILE:
		{
			//AckSearchFile();
			break;
		}
		case ACK_READ_OR_WRITE_FILE:
		{
			//AckReadOrWriteFile();
			break;
		}
		case ACK_CREATE_OR_DEL_PATH:
		{
			//AckCreateOrDelPath();
			break;
		}
		case ACK_GET_OR_SET_PATH:
		{
			//AckGetOrSetPath();
			break;
		}
		default:
			break;
	}
}
/*****************************************************************************
 函 数 名  : CompareDgusRegValue
 功能描述  : 比较DGUS寄存器首字节的数字是否和Value相等
 输入参数  : UINT32 AddrDgus	DGUS寄存器地址
 			 UINT8 Value		待比较的字节 	 
 输出参数  : DWIN_OK    数值相等
 			 DWIN_ERROR 数值不等
 修改历史  :
 日    期  : 2019年6月28日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CompareDgusRegValue(UINT32 AddrDgus, UINT8 Value)
{
	UINT8 DgusValue = 0;
	ReadDGUS(AddrDgus, &DgusValue, sizeof(DgusValue));
	if (DgusValue == Value) return DWIN_OK;
	else return DWIN_ERROR;
}
/*****************************************************************************
 函 数 名  : ReadDgusClientString
 功能描述  : 读取DGUS客户端的输入字符 返回去掉DGUS结束标志的字符串和字符长度 
 输入参数  : UINT32 AddrDgus	DGUS寄存器地址
 			 PUINT8 pData		DGUS数据的接收缓冲区
			 PUINT16 pDataLen	读取前：读取长度 读取后：DGUS数据的实际长度 	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月28日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void ReadDgusClientString(UINT32 AddrDgus, PUINT8 pData, PUINT16 pDataLen)
{
	UINT8 i = 0, Data1 = 0, Data2 = 0;
	ReadDGUS(AddrDgus, pData, *pDataLen);
	for (i = *pDataLen - 1; i != 0; i--)
	{
		if (pData[i] == 0xFF && pData[i - 1] == 0xFF)	/* 逆序查找 找到标志位标记位置 */
		{
			*pDataLen = i - 1;
			break;	
		}
			
	}
	if (i == 0) *pDataLen = strlen(pData);
}
/*****************************************************************************
 函 数 名  : WriteDgusClientString
 功能描述  : 向DGUS客户端写入数据 写入后的数据会带结束标志 写入的实际长度会+2
 输入参数  : UINT32 AddrDgus	DGUS寄存器地址
 			 PUINT8 pData		DGUS数据的写入缓冲区
			 UINT16 DataLen		写入长度 	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月28日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void WriteDgusClientString(UINT32 AddrDgus, PUINT8 pData, UINT16 DataLen)
{
	/* 写入0x00 0x00 结束标志来适配DGUS客户端显示 */
	pData[DataLen++] = 0x00;
	pData[DataLen++] = 0x00;
	WriteDGUS(AddrDgus, pData, DataLen);
	
}

void AckReadOrWriteFile(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 Mod = 0, FileType = 0;
	UINT16 PathLength = 0;
	UINT32 AddrDgusPath = 0;

	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	FileType = Cmd[2];
	AddrDgusPath = (Cmd[3] << 8) | Cmd[4];
	PathLength = PATH_LENGTH;
	/* (2) 读取路径名 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	/* (3) 根据控制字执行创建/删除 文件/目录的操作 */
	switch (Mod)
	{
		case FLAG_CREATE:
		{
			SendString(Path, PathLength);
			Cmd[1] = CreateFileOrDir(Path, FileType);
			break;
		}
		case FLAG_DELETE:
		{
			Cmd[1] = RmFileOrDir(Path);
		}
		default:
			break;
	}
	/* (4) 写入结果 */
	WriteDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
}

void AckSearchFile(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata AimString[MATCH_STRING_LEN];
	UINT8 xdata MatchLish[MATCH_LIST_LEN] = {0};
	PUINT8 pMatch = NULL;
	UINT8 Mod = 0, Status = 0, MatchNumber = 0, i = 0;
	UINT16 PathLength = 0, AimStringLen = 0;
	UINT32 AddrDgusPath = 0, AddrDgusAimString = 0, AddrDgusMatchResult = 0;

	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(AimString, 0, MATCH_STRING_LEN);
	//memset(MatchLish, 0, MATCH_LIST_LEN);
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusAimString = (Cmd[4] << 8) | Cmd[5];
	AddrDgusMatchResult = (Cmd[6] << 8) | Cmd[7];
	/* (2) 读取路径名 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	/* (3) 读取匹配字符 */
	ReadDgusClientString(AddrDgusAimString, AimString, &AimStringLen);
	AimString[AimStringLen] = 0;
	/* (4) 开始匹配 */
	Status = MatchFile(Path, AimString, MatchLish);
	/* (5) 获取匹配数量 */
	pMatch = MatchLish;
	for (i = 0; i < MATCH_LIST_NUM; i++)
	{
		if (*pMatch == 0) break;
		pMatch += MATCH_LIST_LEN / MATCH_LIST_NUM;
		MatchNumber++;
	}
	/* (6) 发送匹配结果 */
	Cmd[1] = MatchNumber;
	WriteDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	/* 由于写入缓冲区已经进行了清零初始化 不用再写结束标志0x00 0x00来适配DGUS客户端显示 */
	WriteDGUS(AddrDgusMatchResult, MatchLish, (MatchNumber * (MATCH_LIST_LEN / MATCH_LIST_NUM)));
}