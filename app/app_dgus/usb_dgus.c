/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : usb_dgus.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年7月8日
功能描述   : USB接口相关的的DGUS应用程序实现
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/

#include "usb_dgus.h"
#include "app/app_usb/app_interface.h"
#include "app/app_usb/file_sys.h"
#include "driver/dgus/dgus.h"
#include "driver/uart/uart.h"
#include "string.h"
#include "stdio.h"

/********************************对内函数声明*********************************/

static UINT8 AckDiskInit(void);
static void AckReadOrWriteFile(void);
static void AckGetOrSetPath(void);
static void AckSystemUp(void);
static void SystemUpDriver(PUINT8 pFileList, UINT8 FileType, UINT16 FileNumber, PUINT16 pTimes) reentrant;
static UINT8 SysUpGetDWINFile(PUINT8 pMatchList);

/********************************宏定义***************************************/
/* USB DGUS寄存器地址 */
#define DGUS_ADDR_GET_OR_SET_PATH		(0x5C0)		/* 获取或者设置文件或目录属性 */
#define DGUS_ADDR_CREATE_OR_DEL_PATH	(0x5C4)		/* 创建或者删除文件或目录 */
#define DGUS_ADDR_READ_OR_WRITE_FILE	(0x5C8)		/* 读取或者写入文件 */
#define DGUS_ADDR_SEARCH_FILE			(0x5D0)		/* 查找文件 */
#define DGUS_ADDR_SYSTEM_UP				(0x5D4)		/* 系统升级 */
#define DGUS_ADDR_DISK_STATUS			(0x5D8)		/* 芯片或者USB状态 */
/* USB 操作动作定义 */
#define ACK_GET_OR_SET_PATH				(0x01)		/* 获取或者设置文件或目录属性 */
#define ACK_CREATE_OR_DEL_PATH			(0x02)		/* 创建或者删除文件或目录 */
#define ACK_READ_OR_WRITE_FILE			(0x03)		/* 读取或者写入文件 */
#define ACK_SEARCH_FILE					(0x04)		/* 查找文件 */
#define ACK_SYSTEM_UP					(0x05)		/* 系统升级 */
#define ACK_DISK_INIT					(0x06)		/* 芯片或者USB初始化 */
/* 标志位定义 */
#define FLAG_START						(0x5A)		/* 开始标志 */
#define FLAG_END						(0x00)		/* 结束标志 */
#define FLAG_READ						(0x5A)		/* 读取标志 */
#define FLAG_WRITE						(0xA5)		/* 写入标志 */
#define FLAG_CREATE						(0x5A)		/* 创建标志 */
#define FLAG_DELETE						(0xA5)		/* 删除标志 */
#define TYPE_FILE						(0x55)		/* 类型为文件 */
#define TYPE_DIR 						(0xAA)		/* 类型为目录 */
#define DISK_IC_ERROR					(0x00)		/* 芯片错误或者连接错误 */
#define DISK_NO_CONNECT					(0x00)		/* 磁盘没有连接 */
#define DISK_NO_INIT					(0x00)		/* 磁盘没有初始化 */

#define MATCH_LIST_NUM					(0x28)		/* 匹配列表的数量 */
#define MATCH_LIST_LEN					(0x320)		/* 匹配列表的总字节长 */
#define MATCH_STRING_LEN				(0x10)		/* 匹配字符串长度 */
#define PATH_LENGTH						(0x80)		/* 路径长度 */
#ifndef BUF_SIZE
#define BUF_SIZE						(0x1000)	/* BUF缓冲区大小 */
#endif

/********************************函数定义开始*********************************/

/*****************************************************************************
 函 数 名  : USBModule
 功能描述  : 唯一对外接口 自动扫描DGUS命令标志并执行相应操作
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/

void USBModule(void)
{
	UINT8 ACK = 0, Flag = 0;
	/* (1) 固定对USB进行连接查询 */
	if (DWIN_OK != AckDiskInit()) return;
	/* (2) 扫描DGUS变量标志位 确定相应应答标志 后扫描的寄存器 执行优先级高 */
	ReadDGUS(DGUS_ADDR_GET_OR_SET_PATH, &Flag, 1);
	if (Flag == FLAG_READ || Flag == FLAG_WRITE)
		ACK = ACK_GET_OR_SET_PATH;
	
	ReadDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, &Flag, 1);
	if (Flag == FLAG_CREATE || Flag == FLAG_DELETE)
		ACK = ACK_CREATE_OR_DEL_PATH;
	
	ReadDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, &Flag, 1);
	if (Flag == FLAG_READ || Flag == FLAG_WRITE)
		ACK = ACK_READ_OR_WRITE_FILE;
	
	ReadDGUS(DGUS_ADDR_SEARCH_FILE, &Flag, 1);
	if (Flag == FLAG_START)
		ACK = ACK_SEARCH_FILE;
	
	ReadDGUS(DGUS_ADDR_SYSTEM_UP, &Flag, 1);
	if (Flag == FILE_ALL || Flag == FILE_T5L51_BIN || Flag == FILE_DWINOS_BIN || 
		Flag == FILE_XXX_LIB || Flag == FILE_XXX_BIN || Flag == FILE_XXX_ICL)
		ACK = ACK_SYSTEM_UP;
		
	
	/* (3) 应答响应 */
	if (ACK == ACK_SYSTEM_UP) AckSystemUp();
	else if (ACK == ACK_SEARCH_FILE) AckSearchFile();
	else if (ACK == ACK_READ_OR_WRITE_FILE) AckReadOrWriteFile();
	else if (ACK == ACK_CREATE_OR_DEL_PATH) AckCreateOrDelPath();
	else if (ACK == ACK_GET_OR_SET_PATH) AckGetOrSetPath();
}

/*****************************************************************************
 函 数 名  : ReadDgusClientString
 功能描述  : 读取DGUS客户端的输入字符 返回去掉DGUS结束标志的字符串和字符长度 
 输入参数  : UINT32 AddrDgus	DGUS寄存器地址
 			 PUINT8 pData		DGUS数据的接收缓冲区
			 PUINT16 pDataLen	读取前：读取长度 读取后：DGUS数据的实际长度 	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
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
 日    期  : 2019年7月8日
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

/*****************************************************************************
 函 数 名  : AckCreateOrDelPath
 功能描述  : 响应：创建或者删除 文件或者目录
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void AckCreateOrDelPath(void)
{
	UINT8 Cmd[8];
	UINT8 xdata Path[PATH_LENGTH];
	UINT8 Mod = 0, FileType = 0;
	UINT16 PathLength = 0, AddrDgusPath = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(Path, 0, PATH_LENGTH);
	/* (1) 读取控制字 */
	ReadDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
	Mod = Cmd[0];
	FileType = Cmd[2];
	AddrDgusPath = ((UINT16)Cmd[3] << 8) | Cmd[4];
	PathLength = PATH_LENGTH;
	/* (2) 读取路径名 */
	ReadDgusClientString(AddrDgusPath, Path, &PathLength);
	Path[PathLength] = 0;
	/* (3) 根据控制字执行创建/删除 文件/目录的操作 */
	if (Mod == FLAG_CREATE)
	{
		Cmd[1] = CreateFileOrDir(Path, FileType);
	}
	if (Mod == FLAG_DELETE)
	{
		Cmd[1] = RmFileOrDir(Path);
	}
	/* (4) 写入结果 */
	Cmd[0] = 0;			/* 清除标志位 */
	WriteDGUS(DGUS_ADDR_CREATE_OR_DEL_PATH, Cmd, sizeof(Cmd));
}

/*****************************************************************************
 函 数 名  : AckSearchFile
 功能描述  : 响应：查找文件
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
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

/*****************************************************************************
 函 数 名  : AckReadOrWriteFile
 功能描述  : 响应：读写文件
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void AckReadOrWriteFile(void)
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
	if (Mod == FLAG_READ)
	{
		Status = ReadFile(Path, Buf, ByteSize, SectorOffset);
		WriteDgusClientString(AddrDgusFileMsg, Buf, ByteSize);
		if (Status == DWIN_OK) Cmd[1] = 0x00;
		else Cmd[1] = 0xFF;
	}
	else if (Mod == FLAG_WRITE)
	{
		ReadDgusClientString(AddrDgusFileMsg, Buf, &ByteSize);
		Status = WriteFile(Path, Buf, ByteSize, SectorOffset);
		if (Status == DWIN_OK) Cmd[1] = 0x00;
		else Cmd[1] = 0xFF;
	}
	else Cmd[1] = 0xFF;
	/* (6) 发送执行结果 */
	Cmd[0] = 0x00;		/* 清除标志位 */
	WriteDGUS(DGUS_ADDR_READ_OR_WRITE_FILE, Cmd, sizeof(Cmd));
}

/*****************************************************************************
 函 数 名  : AckGetOrSetPath
 功能描述  : 响应：获取或者设置文件或者目录属性
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void AckGetOrSetPath(void)
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
	if (Mod == FLAG_READ)
	{
		Status = GetFileMessage(Path, DirName);
		WriteDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
	}
	else if (Mod == FLAG_WRITE)
	{
		ReadDGUS(AddrDgusDirAttr, DirName, sizeof(DirName));
		Status = SetFileMessage(Path, DirName);
	}
	else Status = DWIN_ERROR;
	/* (6) 发送执行结果 */
	Cmd[0] = 0x00;		/* 清除标志位 */
	Cmd[1] = Status;
	WriteDGUS(DGUS_ADDR_GET_OR_SET_PATH, Cmd, sizeof(Cmd));
}

/*****************************************************************************
 函 数 名  : AckSystemUp
 功能描述  : 响应：系统升级
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void AckSystemUp(void)
{
	UINT8 xdata Cmd[8];
	UINT8 xdata FileList[0x320];		/* 40 * 20 */
	UINT8 xdata Reset[4] = {0x55, 0xAA, 0x5A, 0xA5};
	UINT8 FileType = 0;
	UINT16 FileNumber = 0, Times = 0;
	memset(Cmd, 0, sizeof(Cmd));
	memset(FileList, 0, sizeof(FileList));
	ReadDGUS(DGUS_ADDR_SYSTEM_UP, Cmd, sizeof(Cmd));
	FileType = Cmd[0];
	FileNumber = ((UINT16)Cmd[1] << 8) | Cmd[2];
	SysUpGetDWINFile(FileList);			/* 获取DWIN_SET目录下的所有文件 */
	//SendString(FileList, 100);
	SystemUpDriver(FileList, FileType, FileNumber, &Times);
	Cmd[0] = 0x00;
	Cmd[1] = 0x00;
	Cmd[2] = 0x00;
	Cmd[3] = Times >> 8;
	Cmd[4] = (UINT8)Times;
	WriteDGUS(DGUS_ADDR_SYSTEM_UP, Cmd, sizeof(Cmd));
	WriteDGUS(0x04, Reset, 4);
}

/*****************************************************************************
 函 数 名  : AckDiskInit
 功能描述  : 响应：芯片和磁盘初始化
 输入参数  : 无	 
 输出参数  : DWIN_OK 初始化成功
			 其他失败
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static UINT8 AckDiskInit(void)
{
	UINT8 xdata Cmd[4];
	UINT8 Status = DWIN_OK;
	memset(Cmd, 0, sizeof(Cmd));
	
	ReadDGUS(DGUS_ADDR_DISK_STATUS, Cmd, sizeof(Cmd));
	
	if (Cmd[0] == DISK_IC_ERROR)	/* 如果IC未检测，自动检测IC，设置磁盘为未连接和未初始化状态 */
	{
		Cmd[0] = CheckIC();
		if (Cmd[0] == DISK_IC_ERROR)
		{
			Cmd[1] = DISK_NO_CONNECT;
			Cmd[2] = DISK_NO_INIT;
			Status = DWIN_ERROR;
			goto END;
		}
	}
	Cmd[1] = CheckConnect();		/* 断开重连 */
	if (Cmd[1] == DISK_NO_CONNECT)	/* 如果磁盘为未连接，设置磁盘未初始化状态 */
	{
		Cmd[2] = DISK_NO_INIT;
		Status = DWIN_ERROR;
		goto END;
	}
	if (Cmd[2] == DISK_NO_INIT)		/* 如果磁盘为未初始化，进行磁盘初始化 */
	{
		Cmd[2] = CheckDiskInit();
		if (Cmd[2] == DISK_NO_INIT) Status = DWIN_ERROR;
	}
END:
	WriteDGUS(DGUS_ADDR_DISK_STATUS, Cmd, sizeof(Cmd));
	return Status;
}

/*****************************************************************************
 函 数 名  : SystemUpDriver
 功能描述  : 系统升级驱动
 输入参数  : UINT8 FileType		：文件类型
			 UINT16 FileNumber	：文件编号
			 PUINT16 pTimes		：升级成功数
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SystemUpDriver(PUINT8 pFileList, UINT8 FileType, UINT16 FileNumber, PUINT16 pTimes) reentrant
{
	UINT16 xdata Num = 0;
	if (FileNumber != FLAG_ALL)		/* 如果是单个文件 直接升级 */ 
	{
		if (DWIN_OK == SystemUpdate(pFileList, FileType, FileNumber)) (*pTimes)++;
		goto END;
	}
	/* 对于某一类文件升级采取分成单个文件进行递归调用 */
	if (FileType == FILE_ALL)
	{
		SystemUpDriver(pFileList, FILE_T5L51_BIN, 0x00FF, pTimes);
		SystemUpDriver(pFileList, FILE_DWINOS_BIN, 0x00FF, pTimes);
		SystemUpDriver(pFileList, FILE_XXX_LIB, FLAG_ALL, pTimes);
		SystemUpDriver(pFileList, FILE_XXX_BIN, FLAG_ALL, pTimes);
		SystemUpDriver(pFileList, FILE_XXX_ICL, FLAG_ALL, pTimes);
	}
	else if (FileType == FILE_XXX_LIB)
	{
		for (Num = 0; Num < 1000; Num++)
		{
			SystemUpDriver(pFileList, FILE_XXX_LIB, Num, pTimes);
		}
	}
	else if (FileType == FILE_XXX_BIN)
	{
		for (Num = 0; Num < 32; Num++)	/* 0-31存放字库文件 */
		{
			SystemUpDriver(pFileList, FILE_XXX_BIN, Num, pTimes);
		}
	}
	else if (FileType == FILE_XXX_ICL)	/* 32-999存放ICL文件 */
	{
		for (Num = 32; Num < 1000; Num++)
		{
			SystemUpDriver(pFileList, FILE_XXX_ICL, Num, pTimes);
		}
	}
END:{}
}

/*****************************************************************************
 函 数 名  : SysUpGetDWINFile
 功能描述  : 从DWIN_SET路径下列出所有文件
 输入参数  : PUINT8 pMatchList	：匹配列表接收字符串
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static UINT8 SysUpGetDWINFile(PUINT8 pMatchList)
{
	UINT8 Status = 0;
	UINT8 i = 0;
	Status = CH376MatchFile("*", DWIN_DIR, (P_FAT_NAME)pMatchList);
	if (Status == ERR_MISS_FILE) return DWIN_OK; 
	else return DWIN_ERROR;
}