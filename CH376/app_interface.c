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
/********************************对内函数声明*********************************/
UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize);
void Delay(void);

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
void Delay(void)
{
	UINT8 i, j;
	for(i = 0; i < 100; i++)
		for(j = 0; j < 100; j++);
}

/*****************************************************************************
 函 数 名  : CH376USBInit
 功能描述  : 检测CH376通讯、设置USB工作模式、磁盘初始化
 输入参数  : 无	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376USBInit(void)
{
	UINT8 i = 0, Status = 0;
	xWriteCH376Cmd(CMD11_CHECK_EXIST);  	/* 测试单片机与CH376之间的通讯接口 */
	xWriteCH376Data(0x65);
	/* 通讯接口不正常,可能原因有:接口连接异常,其它设备影响(片选不唯一),串口波特率,一直在复位,晶振不工作 */ 
	if (xReadCH376Data() != 0x9A) return ERR_USB_UNKNOWN;

	xWriteCH376Cmd(CMD11_SET_USB_MODE);		/* 设备USB工作模式 */
	xWriteCH376Data(USB_HOST_ON_NO_SOF);
	for (i=100; i!=0; i--) 
		if (xReadCH376Data() == CMD_RET_SUCCESS) break;
	if (0 == i) return ERR_USB_UNKNOWN;

	while ((Status = CH376DiskConnect()) != USB_INT_SUCCESS);

	for (i = 0; i < 100; i ++)				/* 检测磁盘状态 */								   
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
 函 数 名  : CH376TouchNewFile
 功能描述  : 创建新文件,支持多级路径
 输入参数  : PUINT8 pPathName 文件绝对路径名	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376TouchNewFile(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376FileCreatePath(pPathName));	
}
/*****************************************************************************
 函 数 名  : CH376RmFile
 功能描述  : 删除文件或者目录,支持多级路径
 输入参数  : PUINT8 pPathName 文件绝对路径名	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376RmFile(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376FileDeletePath(pPathName));
}
/*****************************************************************************
 函 数 名  : CH376TouchDir
 功能描述  : 创建工作目录
 输入参数  : PUINT8 pPathName 文件绝对路径名	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376TouchDir(PUINT8 pPathName)
{
	AlphabetTransfrom(pPathName);
	return (CH376DirCreate(pPathName));	
}
/*****************************************************************************
 函 数 名  : CH376ReadFile
 功能描述  : 读取文件信息
 输入参数  : PUINT8 pPathName  文件绝对路径名
             PUINT8 pBuf       缓冲区数据长度
			 PUINT32 pFileSize 将返回的文件长度	 
 输出参数  : USB_INT_SUCCESS 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize)	/* 读取文件信息 */	
{	/* 字符存储缓冲区pBuf 4096字节 = 8个扇区 */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;

	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	memset(pBuf, 0, BUF_SIZE);	
	/* (1) 检测文件存在与否,获取文件长度和可读取扇区数 */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	CH376SecLocate(0);
	*pFileSize = CH376GetFileSize();
	
	if (*pFileSize == 0)	return DWIN_ERROR;		/* 空文件 */
	if (*pFileSize % DEF_SECTOR_SIZE)				/* 是否存在尾部零头数据 根据能否整除512判断 */ 
	{
		SectorCount = (*pFileSize >> 9) + 1;		/* T5L是16位 不能做32位乘除法 这里用移位操作实现 */		
	}
	else
	{
		SectorCount = (*pFileSize >> 9);
	}
	/* (2) 文件数据读取和发送 */
	while(1)
	{
		if (SectorCount > BUF_SIZE / DEF_SECTOR_SIZE) 
		{
			Count = BUF_SIZE / DEF_SECTOR_SIZE;
			SectorCount -= BUF_SIZE / DEF_SECTOR_SIZE;
		}
		else
		{
			Count = SectorCount;
		}
		Status = CH376SectorRead(pBuf, (UINT8)Count, NULL);
		if (Status != USB_INT_SUCCESS) break;
		/*	
		if (*pFileSize > BUF_SIZE)
		{
			 Status = SenderBuf(pBuf, FILE_T5L51_BIN, BUF_SIZE);
			 *pFileSize -= BUF_SIZE;
		}
		else Status = SenderBuf(pBuf, FILE_T5L51_BIN, *pFileSize); */
		if (Count == SectorCount) break;		/* 读取完毕 */ 
		
	}
	CH376CloseFile(0);
	return Status;
}
/*****************************************************************************
 函 数 名  : CH376WriteFile
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
UINT8 CH376WriteFile(PUINT8 pPathName, PUINT8 pBuf, UINT8 Flag)			/* 写入文件、不存在则新建 */
{
	UINT8 xdata Buf[BUF_SIZE];					/* 字符存储缓冲区 4096字节 = 8个扇区 */
	UINT8 xdata EndBuf[DEF_SECTOR_SIZE];
	UINT8 Status, SectorCount;
	UINT16 BufFreeLen, BufSourceLen;
	UINT32 FileSize;

	if ((NULL == pPathName) || (NULL == pBuf)) return DWIN_ERROR;
	AlphabetTransfrom(pPathName);
	memset(Buf, 0, BUF_SIZE);
	memset(EndBuf, 0, DEF_SECTOR_SIZE);
	/* (1) 检测文件存在与否 不存在则新建文件 */
	Status = CH376FileOpenPath(pPathName);
	if (Status != USB_INT_SUCCESS) 
	{
		CH376FileCreatePath(pPathName); 
		Status = CH376FileOpenPath(pPathName);
		if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
		Flag = 	WRITE_FROM_HEAD;
	}
	/* (2) 根据标志变量选择写方式 从文件开始/从文件结尾 */
	switch (Flag)
	{
		case WRITE_FROM_HEAD:
		{
			CH376SecLocate(0);
			break;
		}
		case WRITE_FROM_END:					/* 若存在尾部数据需要先重新拼接写入 */
		{
			FileSize = CH376GetFileSize();
			if (FileSize % DEF_SECTOR_SIZE)		/* 是否存在尾部零头数据 根据能否整除512判断 */ 
			{
				Status = CH376SectorRead(EndBuf, 1, NULL);
			}
			CH376SecLocate(0xFFFFFFFF);
			break;
		}
		default:
			return DWIN_ERROR;
	}
	if (strlen(EndBuf) != 0) 
	{
		strncpy(Buf, EndBuf, strlen(EndBuf));	/* 零头数据先写入到缓冲区 */
	}
	FileSize = FileSize + strlen(pBuf);			/* 最终写入的文件大小 */
	/* (2) 循环写数据 */
	while(1)
	{
		BufFreeLen = BUF_SIZE - strlen(Buf);
		BufSourceLen = strlen(pBuf);
		strncat(Buf, pBuf, (BufSourceLen > BufFreeLen ? BufFreeLen : BufSourceLen));
		if (BUF_SIZE == strlen(Buf)) SectorCount = BUF_SIZE / DEF_SECTOR_SIZE;
		else									/* 最后一次写数据, Buf缓冲区没有存满的情况 */
		{
			SectorCount = strlen(Buf) / DEF_SECTOR_SIZE + (strlen(Buf) % DEF_SECTOR_SIZE ? 1 : 0);
			Buf[strlen(Buf)] = 0; 
		}
		Status = CH376SectorWrite(Buf, SectorCount, NULL);
		if (SectorCount != BUF_SIZE / DEF_SECTOR_SIZE) break;
		memset(Buf, 0, BUF_SIZE);				/* 缓存区清空 */
		pBuf += BufFreeLen;
	}
	CH376WriteVar32(VAR_FILE_SIZE, FileSize);	/* 将正确的当前文件长度写入CH376内存 */
	Status = CH376SectorWrite(pBuf, 0, NULL);	/* 写0长度,实际是刷新文件长度 把缓冲区数据真正写入USB */
	return Status;
}

UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize)
{
	if (FileFlag == FILE_T5L51_BIN)
	SendString(pBuf, BufSize);
	return DWIN_OK;
}

/*****************************************************************************
 函 数 名  : FindDWINFile
 功能描述  : 搜索DWIN升级文件
 输入参数  : PUINT8 MatchString  将返回 匹配文件的绝对路径	 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 FindDWINFile(PUINT8 MatchString)
{
	FAT_NAME MatchLish[MATCH_LIST_SIZE];
	UINT8 Status = 0, i = 0;
	UINT16 NameLen = 0;
	Status = CH376MatchFile(MatchString, DWIN_DIR, MatchLish);	
	for (i = 0; i < 10; i++)
	{
		NameLen = strlen(MatchLish[i].NAME);
		if ((MatchLish[i].NAME[NameLen - 3]  == 'B') &&
			(MatchLish[i].NAME[NameLen - 2] == 'I') &&
			(MatchLish[i].NAME[NameLen - 1] == 'N') &&
			(MatchLish[i].Attr != ATTR_VOLUME_ID) &&
			(MatchLish[i].Attr != ATTR_DIRECTORY))
		{
			memset(MatchString, 0, strlen(MatchString));
			strcpy(MatchString, DWIN_DIR);
			strcat(MatchString, "/");
			strcat(MatchString, MatchLish[i].NAME);		/* 把找到的绝对路径写入MatchString */
			return DWIN_OK;
		}
	}
	return DWIN_ERROR;
}









