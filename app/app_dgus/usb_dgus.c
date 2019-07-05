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


void SystemUpDriver(UINT8 FileType, UINT16 FileNumber, PUINT16 pTimes) reentrant;
/********************************宏定义***************************************/
/* USB DGUS寄存器地址 */
#define DGUS_ADDR_GET_OR_SET_PATH		(0x5C0)
#define DGUS_ADDR_CREATE_OR_DEL_PATH	(0x5C4)
#define DGUS_ADDR_READ_OR_WRITE_FILE	(0x5C8)
#define DGUS_ADDR_SEARCH_FILE			(0x5D0)
#define DGUS_ADDR_SYSTEM_UP				(0x5D4)
#define DGUS_ADDR_DISK_STATUS			(0x5D8)
/* USB 操作动作定义 */
#define ACK_GET_OR_SET_PATH				(0x01)
#define ACK_CREATE_OR_DEL_PATH			(0x02)
#define ACK_READ_OR_WRITE_FILE			(0x03)
#define ACK_SEARCH_FILE					(0x04)
#define ACK_SYSTEM_UP					(0x05)
#define ACK_DISK_INIT					(0x06)
/* 标志位定义 */
#define FLAG_START						(0x5A)
#define FLAG_END						(0x00)
#define FLAG_READ						(0x5A)
#define FLAG_WRITE						(0xA5)
#define FLAG_CREATE						(0x5A)
#define FLAG_DELETE						(0xA5)
#define TYPE_FILE						(0x55)
#define TYPE_DIR 						(0xAA)
#define DISK_IC_ERROR					(0x00)
#define DISK_NO_CONNECT					(0x00)
#define DISK_NO_INIT					(0x00)

#define MATCH_LIST_NUM					(0x28)
#define MATCH_LIST_LEN					(0x320)
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
	AckDiskInit();
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
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_ALL) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_T5L51_BIN) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_DWINOS_BIN) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_XXX_LIB) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_XXX_BIN) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_SYSTEM_UP, FILE_XXX_ICL))
		ACK = ACK_SYSTEM_UP;
	if (DWIN_OK == CompareDgusRegValue(DGUS_ADDR_DISK_STATUS, DISK_NO_CONNECT) ||
		DWIN_OK == CompareDgusRegValue(DGUS_ADDR_DISK_STATUS, DISK_NO_INIT))
		ACK = ACK_DISK_INIT;
	/* (2) 应答响应 */
	switch (ACK)
	{
		case ACK_DISK_INIT:
		{
			AckDiskInit();
			break;
		}
		case ACK_SYSTEM_UP:
		{
			AckSystemUp();
			break;
		}
		case ACK_SEARCH_FILE:
		{
			AckSearchFile();
			break;
		}
		case ACK_READ_OR_WRITE_FILE:
		{
			AckReadOrWriteFile();
			break;
		}
		case ACK_CREATE_OR_DEL_PATH:
		{
			AckCreateOrDelPath();
			break;
		}
		case ACK_GET_OR_SET_PATH:
		{
			AckGetOrSetPath();
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
	ReadDGUS(AddrDgus, &DgusValue, 1);
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
	UINT16 Len = 0;
	Len = *pDataLen;
	ReadDGUS(AddrDgus, pData, *pDataLen);
	for (i = 0; i < *pDataLen; i++)	/* 此处只能用正序查找 逆序会有问题 DGUS存储数据不会自动擦除 */
	{
		if (pData[i] == 0xFF && pData[i + 1] == 0xFF)
		{
			*pDataLen = i;
		}
	}
	if (*pDataLen == Len) *pDataLen = strlen(pData);
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

void AckCreateOrDelPath(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 Mod = 0, FileType = 0;
	UINT16 PathLength = 0, AddrDgusPath = 0;
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
	Path[PathLength + 1] = 0;
	/* (3) 根据控制字执行创建/删除 文件/目录的操作 */
	switch (Mod)
	{
		case FLAG_CREATE:
		{
			Cmd[1] = CreateFileOrDir(Path, FileType);
			break;
		}
		case FLAG_DELETE:
		{
			Cmd[1] = RmFileOrDir(Path);
			break;
		}
		default:
			break;
	}
	/* (4) 写入结果 */
	Cmd[0] = 0;			/* 清除标志位 */
	WriteDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
}

void AckSearchFile(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata AimString[MATCH_STRING_LEN];
	UINT8 xdata MatchLish[MATCH_LIST_LEN];
	PUINT8 pMatch = NULL;
	UINT8 Mod = 0, Status = 0, MatchNumber = 0, i = 0;
	UINT16 PathLength = 0, AimStringLen = 0;
	UINT16 AddrDgusPath = 0, AddrDgusAimString = 0, AddrDgusMatchResult = 0;

	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(AimString, 0, MATCH_STRING_LEN);
	memset(MatchLish, 0, MATCH_LIST_LEN);
	PathLength = PATH_LENGTH;
	AimStringLen = MATCH_STRING_LEN;
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusAimString = (Cmd[4] << 8) | Cmd[5];
	AddrDgusMatchResult = (Cmd[6] << 8) | Cmd[7];
	WriteDGUS(AddrDgusMatchResult, MatchLish, MATCH_LIST_LEN);
	/* (2) 读取路径名 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
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
	Cmd[0] = 0;			/* 清除标志位 */
	Cmd[1] = MatchNumber;
	WriteDGUS(DGUS_ADDR_SEARCH_FILE, Cmd, sizeof(Cmd));
	/* 由于写入缓冲区已经进行了清零初始化 不用再写结束标志0x00 0x00来适配DGUS客户端显示 */
	WriteDGUS(AddrDgusMatchResult, MatchLish, (MatchNumber * (MATCH_LIST_LEN / MATCH_LIST_NUM)));
}

void AckReadOrWriteFile(void)
{
	UINT8 xdata Cmd[16];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata Buf[BUF_SIZE];
	UINT8 Mod = 0, Status = 0;
	UINT16 PathLength = 0, ByteSize = 0;
	UINT16 AddrDgusPath = 0, AddrDgusFileMsg = 0, SectorOffset = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(Buf, 0, BUF_SIZE);
	PathLength = PATH_LENGTH;
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath 	= (Cmd[2] << 8) | Cmd[3];
	AddrDgusFileMsg = (Cmd[4] << 8) | Cmd[5];
	SectorOffset 	= ((UINT32)Cmd[6] << 16) | (Cmd[7] << 8) | Cmd[8];
	ByteSize 		= (Cmd[9] << 8) | Cmd[10];
	/* (2) 读取文件路径 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) 根据标志位进行文件读取或者文件写入 */
	switch (Mod)
	{
		case FLAG_READ:
		{
			Status = ReadFile(Path, Buf, ByteSize, SectorOffset);
			UART5_SendString(Buf);
			WriteDgusClientString(AddrDgusFileMsg, Buf, ByteSize);
			if (Status == DWIN_OK) Cmd[1] = 0x00;
			else Cmd[1] = 0xFF;
			break;
		}
		case FLAG_WRITE:
		{
			ReadDgusClientString(AddrDgusFileMsg, Buf, &ByteSize);
			Status = WriteFile(Path, Buf, ByteSize, SectorOffset);
			if (Status == DWIN_OK) Cmd[1] = 0x00;
			else Cmd[1] = 0xFF;
			break;
		}
		default:
		{
			Cmd[1] = 0xFF;
			break;
		}
	}
	/* (6) 发送执行结果 */
	Cmd[0] = 0x00;		/* 清除标志位 */
	WriteDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, Cmd, sizeof(Cmd));
}

void AckGetOrSetPath(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 xdata DirName[sizeof(DIR_TYPE)];
	UINT8 Mod = 0, Status = 0;
	UINT16 PathLength = 0;
	UINT16 AddrDgusPath = 0, AddrDgusDirAttr = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	memset(DirName, 0, sizeof(DirName));
	PathLength = PATH_LENGTH;
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_GET_OR_SET_PATH, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	AddrDgusPath = (Cmd[2] << 8) | Cmd[3];
	AddrDgusDirAttr = (Cmd[4] << 8) | Cmd[5];
	/* (2) 读取文件路径 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	Path[PathLength + 1] = 0;
	/* (3) 根据标志位进行文件属性读取或者属性写入 */
	switch(Mod)
	{
		case FLAG_READ:
		{
			Status = GetFileMessage(Path, DirName);
			WriteDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
			break;
		}
		case FLAG_WRITE:
		{
			ReadDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
			Status = SetFileMessage(Path, DirName);
			break;
		}
		default:
		{
			Status = DWIN_ERROR;
			break;
		}
	}
	/* (6) 发送执行结果 */
	Cmd[0] = 0x00;		/* 清除标志位 */
	Cmd[1] = Status;
	WriteDGUS(DGUS_ADDR_GET_OR_SET_PATH, Cmd, sizeof(Cmd));
}

void AckSystemUp(void)
{
	UINT8 xdata Cmd[8];
	UINT8 FileType = 0;
	UINT16 FileNumber = 0, Times = 0;
	memset(Cmd, 0, sizeof(Cmd));
	UART5_SendString("Up Up Up\n");
	ReadDGUS(DGUS_ADDR_SYSTEM_UP, Cmd, sizeof(Cmd));
	FileType = Cmd[0];
	FileNumber = ((UINT16)Cmd[1] << 8) | Cmd[2];
	SystemUpDriver(FileType, FileNumber, &Times);
	Cmd[0] = 0x00;
	Cmd[1] = 0x00;
	Cmd[2] = 0x00;
	Cmd[3] = Times >> 8;
	Cmd[4] = (UINT8)Times;
	WriteDGUS(DGUS_ADDR_SYSTEM_UP, Cmd, sizeof(Cmd));
}

void AckDiskInit(void)
{
	UINT8 xdata Cmd[4];
	memset(Cmd, 0, sizeof(Cmd));
	
	ReadDGUS(DGUS_ADDR_DISK_STATUS, Cmd, sizeof(Cmd));
	
	if (Cmd[0] == DISK_IC_ERROR)
	{
		Cmd[0] = CheckIC();
		if (Cmd[0] == DISK_IC_ERROR)
		{
			Cmd[1] = DISK_NO_CONNECT;
			Cmd[2] = DISK_NO_INIT;
			goto END;
		}
	}
	Cmd[1] = CheckConnect();
	if (Cmd[1] == DISK_NO_CONNECT)
	{
		Cmd[2] = DISK_NO_INIT;
		goto END;
	}
	if (Cmd[2] == DISK_NO_INIT)
		Cmd[2] = CheckDiskInit();
END:
	WriteDGUS(DGUS_ADDR_DISK_STATUS, Cmd, sizeof(Cmd));	
}

void SystemUpDriver(UINT8 FileType, UINT16 FileNumber, PUINT16 pTimes) reentrant
{
	UINT16 xdata Num = 0;
	UINT8 xdata String[20];
	if (FileNumber != FLAG_ALL) 
	{
		if (DWIN_OK == SystemUpdate(FileType, FileNumber)) (*pTimes)++;
		goto END;
	}
	memset(String, 0, 20);
	switch(FileType)
	{
		case FILE_ALL:
		{
			SystemUpDriver(FILE_T5L51_BIN, 	0x0FFF, pTimes);
			SystemUpDriver(FILE_DWINOS_BIN, 0x0FFF, pTimes);
			SystemUpDriver(FILE_XXX_LIB, FLAG_ALL, pTimes);
			SystemUpDriver(FILE_XXX_BIN, FLAG_ALL, pTimes);
			SystemUpDriver(FILE_XXX_ICL, FLAG_ALL, pTimes);
			break;
		}
		case FILE_T5L51_BIN:
			break;
		case FILE_DWINOS_BIN:
			break;
		case FILE_XXX_LIB:
		{
			for (Num = 0; Num < 1000; Num++)
			{
				SystemUpDriver(FILE_XXX_LIB, Num, pTimes);
			}
			break;
		}
		case FILE_XXX_BIN:
		{
			for (Num = 0; Num < 32; Num++)
			{
				sprintf(String, "Num = %d.\n", (UINT16)Num);
				UART5_SendString(String);
				SystemUpDriver(FILE_XXX_BIN, Num, pTimes);
			}
			break;
		}
		case FILE_XXX_ICL:
		{
			for (Num = 32; Num < 1000; Num++)
			{
				sprintf(String, "Num = %d.\n", (UINT16)Num);
				UART5_SendString(String);
				SystemUpDriver(FILE_XXX_ICL, Num, pTimes);
			}
			break;
		}
	}
END:{}
}

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
	DIR_FileSize = ((UINT32)Ms[9] << 24) | ((UINT32)Ms[10] << 16) | (Ms[11] << 8) | (Ms[12]);
	
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