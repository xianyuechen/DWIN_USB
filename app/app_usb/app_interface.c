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
#include "stdio.h"
/********************************对内函数声明*********************************/

UINT8 SenderBuf(PUINT8 pBuf, UINT8 FileFlag, UINT32 BufSize);
void Delay(void);
void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);

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
	return (CH376FileOrDirCreate(pPathName));
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
UINT8 CH376ReadFile(PUINT8 pPathName, PUINT8 pBuf, PUINT32 pFileSize, UINT32 SectorOffset)	/* 读取文件信息 */	
{	/* 字符存储缓冲区pBuf 4096字节 = 8个扇区 */
	UINT8 Status = 0;
	UINT32 SectorCount = 0, Count = 0;

	if (NULL == pPathName)	return DWIN_NULL_POINT;
	AlphabetTransfrom(pPathName);
	memset(pBuf, 0, BUF_SIZE);	
	/* (1) 检测文件存在与否,获取文件长度和可读取扇区数 */
	Status = CH376FileOpenPath(pPathName);	
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	CH376SecLocate(SectorOffset);
	*pFileSize = CH376GetFileSize() - (SectorOffset << 9);
	
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
	if (SectorCount > BUF_SIZE / DEF_SECTOR_SIZE) 
	{
		Count = BUF_SIZE / DEF_SECTOR_SIZE;
	}
	else
	{
		Count = SectorCount;
	}
	Status = CH376SectorRead(pBuf, (UINT8)Count, NULL);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
			
	if (*pFileSize > BUF_SIZE)
	{
		Status = SenderBuf(pBuf, FILE_T5L51_BIN, BUF_SIZE);
		*pFileSize -= BUF_SIZE;
	}
	else Status = SenderBuf(pBuf, FILE_T5L51_BIN, *pFileSize);
	
	CH376CloseFile(0);
	return DWIN_OK;
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
             PUINT8 pFileSuffix  将返回 匹配文件的后缀	 
 输出参数  : DWIN_OK 成功
 			 其他 出错
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 FindDWINFile(PUINT8 pMatchString, PUINT8 pFileSuffix)
{
	FAT_NAME xdata MatchLish[MATCH_LIST_SIZE];
	UINT8 Status = 0, i = 0;
	UINT16 NameLen = 0;
	if (strlen(pFileSuffix) != 3) return DWIN_ERROR;				/* 检测后缀名是否合法 */
	Status = CH376MatchFile(pMatchString, DWIN_DIR, MatchLish);	
	for (i = 0; i < 10; i++)
	{
		NameLen = strlen(MatchLish[i].NAME);
		if ((MatchLish[i].NAME[NameLen - 3] == *pFileSuffix++) &&	/* 后缀匹配且非文件目录 */
			(MatchLish[i].NAME[NameLen - 2] == *pFileSuffix++) &&
			(MatchLish[i].NAME[NameLen - 1] == *pFileSuffix)   &&
			(MatchLish[i].Attr != ATTR_VOLUME_ID) &&
			(MatchLish[i].Attr != ATTR_DIRECTORY))
		{
			memset(pMatchString, 0, strlen(pMatchString));
			strcpy(pMatchString, DWIN_DIR);
			strcat(pMatchString, "/");
			strcat(pMatchString, MatchLish[i].NAME);				/* 把找到的绝对路径写入MatchString */
			return DWIN_OK;
		}
	}
	return DWIN_ERROR;
}
/*****************************************************************************
 函 数 名  : SystemUpdate
 功能描述  : 系统升级
 输入参数  : UINT8 FileType    升级文件类型
             UINT16 FileNumber 升级文件的编号	 
 输出参数  : DWIN_OK 成功
             其他    失败
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 SystemUpdate(UINT8 FileType, UINT16 FileNumber)
{
	UINT8 xdata Buf[BUF_SIZE + CONTROL_SIZE];
	UINT8 xdata FileName[22];	/* /DWIN_SET/ + 长度 8 + '.' + 3 */
	UINT8 xdata Suffix[SUFFIX_SIZE];
	UINT8 Status = 0;
	UINT8 UpSpace = 0;
	PUINT8 pBufFile = Buf;
	UINT32 FileAddr = 0;
	UINT32 AddrBuff = 0;
	UINT32 FileSize = 0;
	UINT32 SectorOffset = 0;	
	memset(Buf, 0, sizeof(Buf));
	memset(FileName, 0, sizeof(FileName));
	memset(Suffix, 0, SUFFIX_SIZE);
	/* (1) 根据文件类型 设定相关参数 */
	switch (FileType)
	{
		case FILE_T5L51_BIN:
		{
			strcpy(FileName, "T5L51*");
			strcpy(Suffix, "BIN");
			FileAddr = ADDR_T5L51_BIN;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_DWINOS_BIN:
		{
			strcpy(FileName, "DWINOS*");
			strcpy(Suffix, "BIN");
			FileAddr = ADDR_DWIN_OS;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_XXX_LIB:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "LIB");
			FileAddr = LIB(FileNumber);
			UpSpace = SPACE_1;
			break;
		}
		case FILE_XXX_BIN:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "BIN");
			FileAddr = FONT(FileNumber);
			UpSpace = SPACE_2;
			break;
		}
		case FILE_XXX_ICL:
		{
			sprintf(FileName, "%d*", FileNumber);
			strcpy(Suffix, "ICL");
			FileAddr = ICL(FileNumber);
			UpSpace = SPACE_2;
			break;
		}
		default:
			return DWIN_ERROR;
	}
	/* (2) 查找文件 存在则读取文件信息到后4K缓冲区 */
	Status = FindDWINFile(FileName, Suffix);/* 查找目标文件名 */
	if (Status != DWIN_OK) return Status;
	pBufFile += CONTROL_SIZE;				/* 切换到数据保存区域 */
	Status = CH376ReadFile(FileName, pBufFile, &FileSize, 0);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	if (FileSize > BUF_SIZE) FileSize = BUF_SIZE;
	/* (3) 设置前512字节控制字信息 */
	ReadDGUS(ADDR_UP_CONFIG, Buf, 4);			
	AddrBuff = (Buf[3] << 8) & 0xFF00;		/* 获取升级DGUS地址 低8bit是0x00 */
	ReadDGUS(AddrBuff, Buf, 1);
	if (Buf[0] == FLAG_NO_EN) 
	{										/* 设置更新参数 */
		UpdateSet(Buf, FLAG_NO_EN, UpSpace, FileAddr, (UINT16)FileSize);
	}
	else return DWIN_ERROR;										
	WriteDGUS(AddrBuff, Buf, (UINT16)FileSize + CONTROL_SIZE);	/* 首次写入不启动升级 防止升级出错 */
	AddrBuff += CONTROL_SIZE / 2;			/* 移动地址指针到写入控制字的尾部 */
	while (FileSize == BUF_SIZE)				/* 文件还未读取完毕 */
	{
		AddrBuff += BUF_SIZE / 2;			/* 地址偏移:移动地址指针到写入数据的尾部 */
		SectorOffset += 8;					/* 扇区偏移:一次偏移8个扇区(512B) 4096B */
		Status = CH376ReadFile(FileName, pBufFile, &FileSize, SectorOffset);
		if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
		if (FileSize > BUF_SIZE) FileSize = BUF_SIZE;
		WriteDGUS(AddrBuff, pBufFile, (UINT16)FileSize);
	}	
	ReadDGUS(AddrBuff, Buf, 1);
	/* (4) 首位写入升级标志 开启升级 */
	if (Buf[0] == FLAG_NO_EN)
	{
		Buf[0] = FLAG_EN;					/* 启动升级 */
		WriteDGUS(AddrBuff, Buf, 4);
	}
	//SendString(Buf, BUF_SIZE + CONTROL_SIZE);
	return DWIN_OK;
}
/*****************************************************************************
 函 数 名  : UpdateSet
 功能描述  : 更新控制子设置
 输入参数  : PUINT8 pBuf     控制字BUF缓冲区
             UINT8 Flag_EN   升级使能标志位
			 UINT8 UpSpace   升级空间
			 UINT32 UpAddr   文件升级地址	
			 UINT16 FileSize 升级文件大小 
 输出参数  : DWIN_OK 成功
             其他    失败
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize)
{
	 *pBuf++ = Flag_EN;					/* 升级标志 */
	 *pBuf++ = UpSpace;					/* 升级空间选择 */
	 *pBuf++ = (UINT8)(UpAddr >> 24);	/* 远程升级目标地址 */
	 *pBuf++ = (UINT8)(UpAddr >> 16);
	 *pBuf++ = (UINT8)(UpAddr >> 8);
	 *pBuf++ = (UINT8)(UpAddr);
	 *pBuf++ = (UINT8)(FileSize >> 8);	/* 数据字节长度 0x0001 - 0x0FFF */
	 *pBuf++ = (UINT8)(FileSize); 		
	 *pBuf++ = 0x00;					/* 默认不进行CRC校验 */
	 *pBuf++ = 0x00;
}