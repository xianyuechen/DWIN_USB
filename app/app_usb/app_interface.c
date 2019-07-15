/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : app_interface.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : CH376应用程序接口
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/

#include "app_interface.h"
#include "driver/usb/para_port.h"
#include "driver/usb/ch376.h"
#include "driver/uart/uart.h"
#include "driver/dgus/dgus.h"
#include "app/app_usb/file_sys.h"
#include "string.h"
#include "stdio.h"

/********************************对内函数声明*********************************/

static void Delay(void);
static void SysUpGetFileMesg(UINT8 FileType, UINT16 FileNumber, PUINT8 pUpSpace, PUINT32 FileAddr, PUINT8 String);
static void NumberStringMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pCount);
static UINT8 SysUpFileMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pResult, PUINT32 pFileSize);
static void SendUpPackToDGUS(UINT32 AddrDgusHead, UINT32 AddrDgusMesg, PUINT8 BufHead, PUINT8 BufMesg, UINT16 MesgSize);
static void SysUpPcakSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);
static void SysUpFileSend(PUINT8 pPath, UINT8 UpSpace, UINT32 AddrDgusPck,UINT32 AddrFileSave, UINT32 FileSize);
static void SysUpWaitOsFinishRead(UINT32 AddrDgus);

/********************************函数定义开始*********************************/

/*****************************************************************************
 函 数 名  : Delay	软件延时约 0.6ms
 功能描述  : 延时函数
 输入参数  : 无	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void Delay(void)
{
	UINT8 i, j;
	for(i = 0; i < 100; i++)
		for(j = 0; j < 100; j++);
}

/*****************************************************************************
 函 数 名  : USBInit
 功能描述  : 检测CH376通讯、设置USB工作模式、磁盘初始化
 输入参数  : 无	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 USBInit(void)
{
	UINT8 i = 0, Status = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST); 
	xWriteCH376Data(0x65);

	if (xReadCH376Data() != 0x9A)  return ERR_USB_UNKNOWN;

	xWriteCH376Cmd(CMD11_SET_USB_MODE);
	xWriteCH376Data(USB_HOST_ON_NO_SOF);
	for (i=100; i!=0; i--) 
		if (xReadCH376Data() == CMD_RET_SUCCESS) break;
	if (0 == i) return ERR_USB_UNKNOWN;

	for (i = 0; i < 100; i ++)						   
	{
		Delay();
		Status = CH376DiskMount();
		if (USB_INT_SUCCESS == Status) break;
		if (ERR_DISK_DISCON == Status) break;
		if (CH376GetDiskStatus() >= DEF_DISK_MOUNTED && i >= 5) break;
	}
	return USB_INT_SUCCESS;
}

/*****************************************************************************
 函 数 名  : CheckIC
 功能描述  : 检查芯片状态
 输入参数  : 无 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CheckIC(void)
{
	UINT8 i = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST);  	/* 测试单片机与CH376之间的通讯接口 */
	xWriteCH376Data(0x65);
	/* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */ 
	if (xReadCH376Data() != 0x9A)  return 0;
	else 
	{
		xWriteCH376Cmd(CMD11_SET_USB_MODE);		/* 设备USB工作模式 */
		xWriteCH376Data(USB_HOST_ON_NO_SOF);
		for (i=100; i!=0; i--) 
			if (xReadCH376Data() == CMD_RET_SUCCESS) break;
		return 0x5A;
	}
}

/*****************************************************************************
 函 数 名  : CheckConnect
 功能描述  : 检查磁盘连接状态
 输入参数  : 无 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CheckConnect(void)
{
	if (CH376DiskConnect() != USB_INT_SUCCESS) return 0;
	else return 0x5A;
}

/*****************************************************************************
 函 数 名  : CheckDiskInit
 功能描述  : 检查磁盘初始化状态
 输入参数  : 无 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CheckDiskInit(void)
{
	if (CH376DiskMount() != USB_INT_SUCCESS) return 0;
	else return 0x5A;
}

/*****************************************************************************
 函 数 名  : CreateFileOrDir
 功能描述  : 创建新文件或者目录,支持多级路径
 输入参数  : PUINT8 pPathName	文件绝对路径名
 			 UINT8 TypePath		路径属性	 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath)
{
	AlphabetTransfrom(pPathName);
	if (TypePath != PATH_FILE && TypePath != PATH_DIR) return DWIN_ERROR;
	return (CH376CreateFileOrDir(pPathName, TypePath));	
}

/*****************************************************************************
 函 数 名  : CH376CreateFileOrDir
 功能描述  : 创建新文件或者目录,支持多级路径
 输入参数  : PUINT8 pPathName	文件绝对路径名
 			 UINT8 TypePath		路径属性	 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376CreateFileOrDir(PUINT8 pPathName, UINT8 TypePath)
{
	UINT8 xdata NameBuf[PATH_NUMBER][12];
	UINT8 i = 0;
	UINT8 j = 0;
	UINT8 Status = 0;
	memset(NameBuf, 0, PATH_NUMBER * 12);
	/* (1) 把文件目录拆分成单独文件或者目录 */	
	for (i = 0; i < PATH_NUMBER; i++)		
	{
		for (j = 0; j < 12; j++)
		{
			NameBuf[i][j] =  *pPathName++;
			if (*pPathName == '/' || *pPathName == 0) 
			{
				pPathName++;
				break;	
			}
		}
		if (*pPathName == 0) break;
	}
	/* (2) 打开每一级目录或文件 不存在则新建 存在与之冲突的文件或目录则删除 */
	for (j = 0; j < i; j++)				
	{
		Status = CH376FileOpen(NameBuf[j]);
		if (j == (i - 1) && TypePath == PATH_FILE)	
		{
			if (Status == ERR_MISS_FILE)							/* 发现 文件或目录不存在 */
			{
				if (USB_INT_SUCCESS != CH376FileCreate(NameBuf[j])) return CH376Error();
				CH376FileOpen(NameBuf[j]);							/* 这里处理创建文件出现乱码头的问题 */
				NameBuf[j + 1][0] = 0;
				NameBuf[j + 1][1] = 0;
				CH376SectorWrite(NameBuf[j + 1], 2, NULL);
				CH376CloseFile(0);
			}
			if (Status == ERR_OPEN_DIR)	return CH376Error();		/* 发现是目录 */
		}
		else
		{
			if (Status == ERR_MISS_FILE)							/* 发现 目录不存在 */
			{
				if (USB_INT_SUCCESS != CH376DirCreate(NameBuf[j])) return CH376Error();
				CH376FileOpen(NameBuf[j]);
			}
			if (Status == USB_INT_SUCCESS) return CH376Error();		/* 发现 文件 */
		}
	}
	return DWIN_OK;
}

/*****************************************************************************
 函 数 名  : RmFileOrDir
 功能描述  : 删除文件或者目录,支持多级路径
 输入参数  : PUINT8 pPathName 文件绝对路径名	 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 RmFileOrDir(PUINT8 pPathName)
{
	UINT8 i = 0;
	AlphabetTransfrom(pPathName);
	for (i = 20; i != 0; i--)
		if (USB_INT_SUCCESS == CH376FileDeletePath(pPathName)) return DWIN_OK;
	return DWIN_ERROR;
}

/*****************************************************************************
 函 数 名  : ReadFile
 功能描述  : 读取文件信息	   (读取大小比文件总大小大时，只会读取文件扇区偏移后总大小)
 输入参数  : PUINT8 pPathName  文件绝对路径名
             PUINT8 pBuf       缓冲区数据长度
			 UINT16 DataLen    读取数据的长度
			 UINT32 SectorOffset 读取的起始扇区位置	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 ReadFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset)	/* 读取文件信息 */	
{	/* 字符存储缓冲区pBuf 4096字节 = 8个扇区 */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;
	UINT32 FileSize = 0;
	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	//memset(pData, 0, BUF_SIZE);
	/* (1) 检测文件存在与否,获取文件长度和可读取扇区数 */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	FileSize = CH376GetFileSize();
	if (FileSize == 0)	return CH376Error();	/* 空文件 */
	/* (2) 检测将读取的字节数位置是否大于文件总字节数 大于则出错直接返回 */
	if ((SectorOffset != 0) &&(((FileSize >> 9)) < (SectorOffset - 1))) return CH376Error();
	/* (3) 检测将读取的字节数是否大于文件偏移后的字节数 大于则直接取剩余的文件字节数作为读取数 */
	if ((FileSize + 512) < ((SectorOffset << 9) + DataLen)) FileSize = FileSize - (SectorOffset << 9);
	else  FileSize = (UINT32)DataLen;
	CH376SecLocate(SectorOffset);				/* 设置扇区偏移 */
	while (FileSize)
	{
	/* (4) 设置读取扇区数 */
		/* 是否存在尾部零头数据 存在则多读取一个扇区 */
		if (FileSize % DEF_SECTOR_SIZE) SectorCount = (FileSize >> 9) + 1;	
		else SectorCount = (FileSize >> 9);
		/* 一次只能读取8个扇区 大于则取8 */
		if (SectorCount > (BUF_SIZE / DEF_SECTOR_SIZE))
		{
			Count = BUF_SIZE / DEF_SECTOR_SIZE;
			FileSize -= BUF_SIZE;
		} 
		else 
		{
			Count = SectorCount;
			FileSize = 0;
		}
	/* (5) 文件数据读取 */	
		Status = CH376SectorRead(pData, (UINT8)Count, NULL);	
		if (Status != USB_INT_SUCCESS) return CH376Error();
	}
	CH376CloseFile(0);
	return DWIN_OK;
}

/*****************************************************************************
 函 数 名  : WriteFile
 功能描述  : 入文件、不存在则新建
 输入参数  : PUINT8 pPathName  文件绝对路径名
             PUINT8 pBuf       缓冲区数据长度
			 UINT8 Flag        写入标志位: 从文件头部写入/从文件尾部写入	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 WriteFile(PUINT8 pPathName, PUINT8 pData, UINT16 DataLen, UINT32 SectorOffset)	/* 写入文件、不存在则新建 */
{
	UINT8 xdata Buf[BUF_SIZE];					/* 字符存储缓冲区 4096字节 = 8个扇区 */
	UINT8 xdata EndBuf[DEF_SECTOR_SIZE];
	UINT8 Status = 0, SectorCount = 0, Flag = 0;
	PUINT8 pMid = NULL; 
	UINT16 BufFreeLen = 0, BufSourceLen = 0, EndBufSize = 0;
	UINT32 FileSize = 0;

	memset(Buf, 0, BUF_SIZE);
	memset(EndBuf, 0, DEF_SECTOR_SIZE);
	if ((NULL == pPathName) || (NULL == pData)) return DWIN_ERROR;
	AlphabetTransfrom(pPathName);
	if (SectorOffset == 0) Flag = WRITE_FROM_HEAD;
	else Flag = WRITE_FROM_END;
	
	/* (1) 检测文件存在与否 不存在则新建文件 */
	Status = CH376FileOpenPath(pPathName);
	if (Status != USB_INT_SUCCESS) 
	{
		CH376CreateFileOrDir(pPathName, PATH_FILE);
		Status = CH376FileOpenPath(pPathName);
		if (Status != USB_INT_SUCCESS) return CH376Error();
		Flag = 	WRITE_FROM_HEAD;
	}
	/* (2) 根据标志变量选择写方式 从文件开始/从文件结尾 */
	if (Flag == WRITE_FROM_HEAD) CH376SecLocate(0);
	else if (Flag == WRITE_FROM_END)	/* 若存在尾部数据需要先重新拼接写入 */
	{
		FileSize = CH376GetFileSize();
		EndBufSize = FileSize % DEF_SECTOR_SIZE;
		if (EndBufSize)		/* 是否存在尾部零头数据 根据能否整除512判断 */ 
		{
			Status = CH376SectorRead(EndBuf, 1, NULL);
		}
		CH376SecLocate(0xFFFFFFFF);
	}
	else return CH376Error();
	FileSize += DataLen;
	if (EndBufSize != 0)		/* 有零头数据 先与pData组合成数据包 写一次不大于4K的数据 */ 
	{
		strncpy(Buf, EndBuf, EndBufSize);			
		BufFreeLen = BUF_SIZE - EndBufSize;
		pMid = Buf;
		pMid += EndBufSize;
		if (DataLen > BufFreeLen)
		{
			strncpy(pMid, pData, BufFreeLen);
			SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
			DataLen -= BufFreeLen;
			pData += BufFreeLen;	
		}
		else
		{
			strncpy(pMid, pData, DataLen);
			SectorCount = (EndBufSize + DataLen) / DEF_SECTOR_SIZE;			/* 扇区数 如果不是扇区整数倍则扇区数+1 */
			if ((EndBufSize + DataLen) % DEF_SECTOR_SIZE) SectorCount++;	/* 上面计算向零取整 这里计算补偿 扇区数+1 */
			DataLen = 0;
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		memset(Buf, 0, BUF_SIZE);	/* 写完将51缓冲区清零 */
	}
	/* (3) 循环写数据 */
	while(DataLen)
	{
		if (DataLen > BUF_SIZE)
		{
			strncpy(Buf, pData, BUF_SIZE);
			SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
			DataLen -= BUF_SIZE;
			pData += BUF_SIZE;	
		}
		else
		{
			strncpy(Buf, pData, DataLen);
			SectorCount = DataLen / DEF_SECTOR_SIZE;		/* 扇区数 如果不是扇区整数倍则扇区数+1 */
			if (DataLen % DEF_SECTOR_SIZE) SectorCount++;	/* 上面计算向零取整 这里计算补偿 扇区数+1 */
			DataLen = 0;
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		memset(Buf, 0, BUF_SIZE);				/* 缓存区清空 */
	}
	CH376WriteVar32(VAR_FILE_SIZE, FileSize);	/* 将正确的当前文件长度写入CH376内存 */
	Status = CH376SectorWrite(pData, 0, NULL);	/* 写0长度,实际是刷新文件长度 把缓冲区数据真正写入USB */
	CH376CloseFile(0);
	return DWIN_OK;
}

/*****************************************************************************
 函 数 名  : MatchFile
 功能描述  : 检查芯片状态
 输入参数  : PUINT8 pDir		：匹配路径 
			 PUINT8 pMatchString：匹配字符串 
			 PUINT8 pBuf		：存放匹配结果的缓冲BUF
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 MatchFile(PUINT8 pDir,PUINT8 pMatchString, PUINT8 pBuf)
{
	UINT8 Status = 0, i = 0;
	if(pBuf == NULL) return DWIN_ERROR;
	AlphabetTransfrom(pDir);
	AlphabetTransfrom(pMatchString);
	Status = CH376MatchFile(pMatchString, pDir, (P_FAT_NAME)pBuf);
	return (Status == USB_INT_SUCCESS ? DWIN_OK : DWIN_ERROR);
}

/*****************************************************************************
 函 数 名  : SysUpGetFileMesg
 功能描述  : 根据文件类型和编号获取相应的文件信息参数
 输入参数  : UINT8 FileType		：文件类型
			 UINT16 FileNumber	：文件编号
			 PUINT8 pUpSpace	：文件的升级空间
			 PUINT32 pFileAddr	：文件的物理地址
			 PUINT8 pString		：文件的匹配字符
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SysUpGetFileMesg(UINT8 FileType, UINT16 FileNumber, PUINT8 pUpSpace, PUINT32 pFileAddr, PUINT8 pString)
{
	if (FileType == FILE_T5L51_BIN)
	{
		strcpy(pString, "T5L51");
		pString += 6;
		strcpy(pString, ".BIN");
		*pFileAddr = ADDR_T5L51_BIN;
		*pUpSpace = SPACE_1;
	}
	else if (FileType == FILE_DWINOS_BIN)
	{
		strcpy(pString, "DWINOS");
		pString += 6;
		strcpy(pString, ".BIN");
		*pFileAddr = ADDR_DWIN_OS;
		*pUpSpace = SPACE_1;
	}
	else if (FileType == FILE_XXX_LIB)
	{
		sprintf(pString, "%d", FileNumber);
		pString += 6;
		strcpy(pString, ".LIB");
		*pFileAddr = LIB((UINT32)FileNumber);
		*pUpSpace = SPACE_1;
	}
	else if (FileType == FILE_XXX_BIN)
	{
		sprintf(pString, "%d", FileNumber);
		pString += 6;
		strcpy(pString, ".BIN");
		pString += 6;
		strcpy(pString, ".DZK");
		pString += 6;
		strcpy(pString, ".HZK");
		*pFileAddr = FONT((UINT32)FileNumber);
		*pUpSpace = SPACE_2;
	}
	else if (FileType == FILE_XXX_ICL)
	{
		sprintf(pString, "%d", FileNumber);
		pString += 6;
		strcpy(pString, ".ICL");
		*pFileAddr = ICL((UINT32)FileNumber);
		*pUpSpace = SPACE_2;
	}
}

/*****************************************************************************
 函 数 名  : NumberStringMatch
 功能描述  : 数字编号的文件匹配，其他开头的文件会直接默认成功
 输入参数  : PUINT8 pSource		：被匹配的文件字符串
			 PUINT8 pDest		：匹配字符串
			 PUINT8 pCount		：匹配结果计数
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void NumberStringMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pCount)
{	
	if (*pDest > '9' || *pDest < '0')
	{
		(*pCount)++;
		return;
	}		
	do
	{
		if (*pDest != *pSource) return;
		pDest++;
		pSource++;
	}while(*pSource >= '0' && *pSource <= '9');
		
	(*pCount)++;
}

/*****************************************************************************
 函 数 名  : SysUpFileMatch
 功能描述  : 根据多组匹配字符串，从文件列表中查找出目标文件
 输入参数  : PUINT8 pSource		：被匹配的文件列表
			 PUINT8 pDest		：匹配的多组特征字符串
			 PUINT8 pResult		：匹配成功的第一个文件
			 PUINT32 pFileSize	：匹配成功的第一个文件的大小
 输出参数  : DWIN_OK	成功
			 其他		失败
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static UINT8 SysUpFileMatch(PUINT8 pSource, PUINT8 pDest, PUINT8 pResult, PUINT32 pFileSize)
{
	UINT8 i = 0;
	PUINT8 pNow = pDest;
	PUINT8 pStatus = NULL;
	for (; *pSource != 0; pSource += sizeof(FAT_NAME))
	{
		i = 0;
		for (pNow = pDest; *pNow != 0; pNow += 6)
		{
			pStatus = strstr(pSource, pNow);
			if (pStatus != NULL) NumberStringMatch(pSource, pNow, &i);
			if (i == 0 && pStatus == NULL) break;
			if (i == 2)
			{
				sprintf(pResult, "%s/%s", DWIN_DIR, pSource);
				pSource += sizeof(((FAT_NAME *)0) -> NAME);
				*pFileSize = *pFileSize | *pSource++;
				*pFileSize = *pFileSize | ((UINT16)(*pSource++) << 8);
				*pFileSize = *pFileSize | ((UINT32)(*pSource++) << 16);
				*pFileSize = *pFileSize | ((UINT32)(*pSource) << 24);	
				return DWIN_OK;
			}
		}
	}
	return DWIN_ERROR;
}

/*****************************************************************************
 函 数 名  : SysUpWaitOsFinishRead
 功能描述  : 等待OS完成数据读取
 输入参数  : UINT32 AddrDgus	：标志位DGUS地址
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SysUpWaitOsFinishRead(UINT32 AddrDgus)
{
	UINT8 Flag = 0;
	do
	{
		ReadDGUS(AddrDgus, &Flag, 1);
	}
	while(Flag == FLAG_EN);
}

/*****************************************************************************
 函 数 名  : SysUpPcakSet
 功能描述  : 系统升级包 512字节包头设置
 输入参数  : PUINT8 pBuf		：匹配包指针
			 UINT8 Flag_EN		：包有效使能
			 UINT8 UpSpace		：升级空间
			 UINT32 UpAddr		：升级文件地址
			 UINT16 FileSize	：后4K包实际字节大小
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SysUpPcakSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize)
{
	*pBuf++ = Flag_EN;					/* 升级标志 */
	*pBuf++ = UpSpace;					/* 升级空间选择 */
	*pBuf++ = (UINT8)(UpAddr >> 24);	/* 远程升级目标地址 */
	*pBuf++ = (UINT8)(UpAddr >> 16);
	*pBuf++ = (UINT8)(UpAddr >> 8);
	*pBuf++ = (UINT8)(UpAddr);
	*pBuf++ = (UINT8)(FileSize >> 8);	/* 数据字节长度 0x0001 - 0x0FFF */
	*pBuf++ = (UINT8)(FileSize); 		
	*pBuf++ = 0x00;						/* 默认不进行CRC校验 */
	*pBuf = 0x00;
}

/*****************************************************************************
 函 数 名  : SendUpPackToDGUS
 功能描述  : 根据多组匹配字符串，从文件列表中查找出目标文件
 输入参数  : UINT32 AddrDgusHead：头包DGUS地址
			 UINT32 AddrDgusMesg：尾包DGUS地址
			 PUINT8 BufHead		：头包指针
			 PUINT8 BufMesg		：尾包指针
			 UINT16 MesgSize	：4K尾包实际数据字节大小
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SendUpPackToDGUS(UINT32 AddrDgusHead, UINT32 AddrDgusMesg, PUINT8 BufHead, PUINT8 BufMesg, UINT16 MesgSize)
{
	SysUpWaitOsFinishRead(AddrDgusHead);
	WriteDGUS(AddrDgusHead, BufHead, 10);
	WriteDGUS(AddrDgusMesg, BufMesg, MesgSize);
	if (*BufHead != FLAG_EN)
	{
		*BufHead = FLAG_EN;
		SysUpWaitOsFinishRead(AddrDgusHead);
		WriteDGUS(AddrDgusHead, BufHead, 10);
	}
}

/*****************************************************************************
 函 数 名  : SysUpFileSend
 功能描述  : 根据多组匹配字符串，从文件列表中查找出目标文件
 输入参数  : PUINT8 pPath		：文件绝对路径
			 UINT8 UpSpace		：升级空间
			 UINT32 AddrDgusPck	：升级包缓冲区DGUS地址
			 UINT32 AddrFileSave：文件保存地址
			 UINT32 FileSize	：文件字节大小
 输出参数  : 无
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
static void SysUpFileSend(PUINT8 pPath, UINT8 UpSpace, UINT32 AddrDgusPck,UINT32 AddrFileSave, UINT32 FileSize)
{
	UINT8 xdata BufHead[CONTROL_SIZE];
	UINT8 xdata BufMesg[BUF_SIZE];
	UINT8 Count = 0, i;
	UINT16 PackSize = 0, FirstPackSize = 0;
	UINT32 SectorOffset = 0, FirstAddrFileSave = 0;
	UINT32 xdata AddrDgusPackHead[4];
	UINT32 xdata AddrDgusPackMesg[4];
	
	memset(BufHead, 0, sizeof(BufHead));
	memset(BufMesg, 0, sizeof(BufMesg));
	memset(AddrDgusPackHead, 0, sizeof(AddrDgusPackHead));
	memset(AddrDgusPackMesg, 0, sizeof(AddrDgusPackMesg));
	AddrDgusPackHead[0] = AddrDgusPck;
	AddrDgusPackMesg[0] = AddrDgusPck + (CONTROL_SIZE / 2);
	for (i = 1; i < 4; i++)
	{
		AddrDgusPackHead[i] = AddrDgusPackHead[i - 1] + BUF_SIZE / 2 + CONTROL_SIZE / 2;
		AddrDgusPackMesg[i] = AddrDgusPackMesg[i - 1] + BUF_SIZE / 2 + CONTROL_SIZE / 2;
	}
	i = 0;
	if(FileSize > BUF_SIZE) FirstPackSize = BUF_SIZE;
	else 
	{
		FirstPackSize = (UINT16)FileSize;
		FileSize = 0;
	}
	FirstAddrFileSave = AddrFileSave;
	/* Send Pack */
	while(FileSize)
	{
		if (FileSize > BUF_SIZE) 
		{
			PackSize = BUF_SIZE;
			FileSize -= BUF_SIZE;
		}
		else
		{
			PackSize = (UINT16)FileSize;
			FileSize = 0;
		}
		SysUpPcakSet(BufHead, FLAG_NO_EN, UpSpace, AddrFileSave, PackSize);
		ReadFile(pPath, BufMesg, PackSize, SectorOffset);
		if (Count == 0) 
		{
			memset(BufMesg, 0, 16); /* FirstPack Head Clean, Because of .ICL's scaning */
			Count = 1;
		}
		if (i == 4) i = 0;
		SendUpPackToDGUS(AddrDgusPackHead[i], AddrDgusPackMesg[i], BufHead, BufMesg, PackSize);
		i++;
		//memset(BufMesg, 0, sizeof(BufMesg));	/* */
		AddrFileSave += BUF_SIZE;		
		SectorOffset += BUF_SIZE / DEF_SECTOR_SIZE;
	}
	SysUpPcakSet(BufHead, FLAG_NO_EN, UpSpace, FirstAddrFileSave, FirstPackSize);
	ReadFile(pPath, BufMesg, FirstPackSize, 0);
	if (i == 4) i = 0;
	SendUpPackToDGUS(AddrDgusPackHead[i], AddrDgusPackMesg[i], BufHead, BufMesg, FirstPackSize);
}

/*****************************************************************************
 函 数 名  : SystemUpdate
 功能描述  : 根据文件类型和文件编号进行文件升级，返回文件升级状态
 输入参数  : UINT8 FileType		：文件类型
			 UINT16 FileNumber	：文件编号
 输出参数  : DWIN_OK	成功
			 其他		失败
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 SystemUpdate(PUINT8 pFileList, UINT8 FileType, UINT16 FileNumber)
{
	UINT8 xdata String[24]; //4 * 6
	UINT8 xdata FilePath[22];
	UINT8 UpSpace = 0, Status = 0;
	UINT8 Protect = 0;
	UINT32 AddrFile = 0, FileSize = 0, AddrDgusPack = 0;
	memset(String, 0, sizeof(String));
	memset(FilePath, 0, sizeof(FilePath));
	/* (1) 读取控制字 */
	ReadDGUS(ADDR_UP_CONFIG, FilePath, 4);
	AddrDgusPack = ((UINT16)FilePath[3] << 8) | 0x00;
	/* (2) 根据文件类型和文件编号获取文件参数 */
	SysUpGetFileMesg(FileType, FileNumber, &UpSpace, &AddrFile, String);
	/* (3) 查找出目录文件 */
	Protect = UpSpace;
	Status = SysUpFileMatch(pFileList, String, FilePath, &FileSize);
	if (Status != DWIN_OK) return DWIN_ERROR;
	if (FileSize == 0) return DWIN_ERROR;
	//
	UART5_SendString(FilePath);
	UART5_SendString("\n");
	//
	UpSpace = Protect;
	/* (4) 把文件信息发送到升级空间 */
	SysUpFileSend(FilePath, UpSpace, AddrDgusPack, AddrFile, FileSize);
	return DWIN_OK;
}

/*****************************************************************************
 函 数 名  : GetFileMessage
 功能描述  : 获取文件或者目录属性
 输入参数  : PUINT8 pFilePath	：绝对路径
			 PUINT8 pDest		：接收缓冲区
 输出参数  : DWIN_OK	成功
			 其他		失败
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 GetFileMessage(PUINT8 pFilePath, PUINT8 pBuf)
{
	UINT8 Status = 0;
	P_DIR_TYPE pDir = NULL;
	UINT8 xdata FatDir[sizeof(FAT_DIR_INFO)];
	P_FAT_DIR_INFO pFatDir = (P_FAT_DIR_INFO)FatDir;

	memset(FatDir, 0, sizeof(FAT_DIR_INFO));
	AlphabetTransfrom(pFilePath);
	Status = CH376GetFileMessage(pFilePath, (P_FAT_DIR_INFO)FatDir);
	pDir = (P_DIR_TYPE)pBuf;
	
	pDir -> DIR_Attr     = pFatDir -> DIR_Attr;
	pDir -> DIR_CrtTime  = pFatDir -> DIR_CrtTime;
	pDir -> DIR_CrtDate  = pFatDir -> DIR_CrtDate;
	pDir -> DIR_WrtTime  = pFatDir -> DIR_WrtTime;
	pDir -> DIR_WrtDate  = pFatDir -> DIR_WrtDate;
	pDir -> DIR_FileSize = pFatDir -> DIR_FileSize;
	return Status;
}

/*****************************************************************************
 函 数 名  : GetFileMessage
 功能描述  : 设置文件或者目录属性
 输入参数  : PUINT8 pFilePath	：绝对路径
			 PUINT8 pDest		：发送缓冲区
 输出参数  : DWIN_OK	成功
			 其他		失败
 修改历史  :
 日    期  : 2019年7月8日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 SetFileMessage(PUINT8 pFilePath, PUINT8 pBuf)
{
	UINT8 Status = 0;
	P_DIR_TYPE pDir = NULL;
	UINT8 xdata FatDir[sizeof(FAT_DIR_INFO)];
	P_FAT_DIR_INFO pFatDir = (P_FAT_DIR_INFO)FatDir;

	memset(FatDir, 0, sizeof(FAT_DIR_INFO));
	AlphabetTransfrom(pFilePath);
	pDir = (P_DIR_TYPE)pBuf;
	pFatDir -> DIR_Attr     = pDir -> DIR_Attr;
	pFatDir -> DIR_CrtTime  = pDir -> DIR_CrtTime;
	pFatDir -> DIR_CrtDate  = pDir -> DIR_CrtDate;
	pFatDir -> DIR_WrtTime  = pDir -> DIR_WrtTime;
	pFatDir -> DIR_WrtDate  = pDir -> DIR_WrtDate;
	pFatDir -> DIR_FileSize = pDir -> DIR_FileSize;
	Status = CH376SetFileMessage(pFilePath, (P_FAT_DIR_INFO)FatDir);
	return Status;
}