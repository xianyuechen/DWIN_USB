/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : file_sys.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月4日
功能描述   : CH376驱动程序接口
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "file_sys.h"

UINT8 xdata GlobalBuf[64];
/********************************内部函数声明*********************************/
UINT8	Wait376Interrupt(void);
UINT8	Query376Interrupt(void);
void	CH376SetFileName(PUINT8 name);
UINT8	CH376DeleteFile(PUINT8 pName);
UINT8	CH376SeparatePath(PUINT8 path);
UINT8	CH376FileOpenDir(PUINT8 PathName, UINT8 StopName);
void	CH376WriteVar8(UINT8 var, UINT8 dat);
UINT8	CH376ReadVar8(UINT8 var);
void	CH376WriteVar32(UINT8 var, UINT32 dat);
UINT32	CH376ReadVar32(UINT8 var);
UINT32	CH376Read32bitDat(void);
void	CH376Write32bitDat(UINT32 mData);
void	CH376EndDirInfo(void);
UINT8	CH376SendCmdWaitInt(UINT8 mCmd);
UINT8	CH376GetIntStatus(void);
UINT8	CH376DiskReqSense(void);
UINT8	CH376DiskWriteSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount);
UINT8	CH376DiskReadSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount);
void	CH376WriteHostBlock(PUINT8 buf, UINT8 len);
UINT8	CH376ReadBlock(PUINT8 buf);  
UINT8	CH376DiskQuery(PUINT32 DiskFree);
UINT8	CH376DiskCapacity(PUINT32 DiskCap);
UINT8	CH376WriteReqBlock(PUINT8 pbuf);
UINT8	CH376CheckNameSum(PUINT8 pDirName);
UINT8	CH376ByteLocate(UINT32 offset);
UINT8	CH376ByteRead(PUINT8 pbuf, UINT16 ReqCount, PUINT16 pRealCount);
UINT8	CH376LocateInUpDir(PUINT8 pPathName);

/********************************函数定义开始*********************************/

void AlphabetTransfrom(PUINT8 name)			/* 小写文件名统一转换为大写文件名 */
{
	PUINT8 c;
	for(c = name; *c != 0; c++)
	{
		if((*c >= 'a') && (*c <= 'z')) *c = *c -32;
	}
}

UINT8 Wait376Interrupt(void)
{
	UINT32 i = 0;
	while (Query376Interrupt() == FALSE)
	{
		if(i++ == 0xFFFFF) break;
	}	/* 一直等中断 */
	return (CH376GetIntStatus());			/* 检测到中断 */	
}

UINT8 Query376Interrupt(void)
{
	return (CH376_INT_WIRE ? FALSE : TRUE);  /* CH376的中断引脚查询 */
}

void CH376SetFileName(PUINT8 name)  /* 设置将要操作的文件的文件名 */
{
	UINT8	c;
	xWriteCH376Cmd(CMD10_SET_FILE_NAME);
	c = *name;
	xWriteCH376Data(c);
	while (c) 
	{
		name ++;
		c = *name;
		if (c == DEF_SEPAR_CHAR1 || c == DEF_SEPAR_CHAR2) c = 0;  /* 强行将文件名截止 */
		xWriteCH376Data (c);
	}	
}

UINT8 CH376Error(void)
{
	CH376CloseFile(0);
	return DWIN_ERROR;
}

UINT8 CH376DeleteFile(PUINT8 pName)		/* 在根目录或者当前目录下删除文件或者目录(文件夹) */
{
	CH376SetFileName(pName);
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}	

UINT8 CH376FileOpen(PUINT8 name)	/* 在根目录或者当前目录下打开文件或者目录(文件夹) */
{
	CH376SetFileName(name);
	return (CH376SendCmdWaitInt(CMD0H_FILE_OPEN));
}

UINT8 CH376SeparatePath(PUINT8 path)  /* 从路径中分离出最后一级文件名或者目录(文件夹)名,返回最后一级文件名或者目录名的字节偏移 */
{
	PUINT8	pName;
	for (pName = path; *pName != 0; ++ pName);
	while (*pName != DEF_SEPAR_CHAR1 && *pName != DEF_SEPAR_CHAR2 && pName != path) 
		pName --;
	if (pName != path) pName ++;
	return (pName - path);
}

UINT8 CH376FileOpenDir(PUINT8 PathName, UINT8 StopName)  /* 打开多级目录下的文件或者目录的上级目录,支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
/* StopName 指向最后一级文件名或者目录名 */
{
	UINT8 i, s;
	s = 0;
	i = 1;
	while (1) 
	{
		while (PathName[i] != DEF_SEPAR_CHAR1 && PathName[i] != DEF_SEPAR_CHAR2 && PathName[i] != 0) ++ i;
		if (PathName[i]) i ++;
		else i = 0;  /* 路径结束 */
		s = CH376FileOpen(&PathName[s]);
		if (i && i != StopName) 
		{  /* 路径尚未结束 */
			if (s != ERR_OPEN_DIR) 
			{
				if (s == USB_INT_SUCCESS) return ERR_FOUND_NAME;
				else if (s == ERR_MISS_FILE) return ERR_MISS_DIR;
				else return s;  /* 操作出错 */
			}
			s = i;  /* 从下一级目录开始继续 */
		}
		else return s;  /* 路径结束,USB_INT_SUCCESS为成功打开文件,ERR_OPEN_DIR为成功打开目录(文件夹),其它为操作出错 */
	}
}

UINT8 CH376DirCreate(PUINT8 PathName)		/* 在根目录或者当前目录创建目录 */
{
	CH376SetFileName(PathName); 
	CH376WriteVar32(VAR_CURRENT_CLUST, 0);
	return (CH376SendCmdWaitInt(CMD0H_DIR_CREATE));		
}

UINT8 CH376FileDeletePath(PUINT8 PathName)	/* 删除文件,如果已经打开则直接删除,否则对于文件会先打开再删除,支持多级目录路径 */
{
	UINT8 s;
	if (PathName ) 
	{  /* 文件尚未打开 */
		for (s = 1; PathName[s] != DEF_SEPAR_CHAR1 && PathName[s] != DEF_SEPAR_CHAR2 && PathName[s] != 0; ++ s);  /* 搜索下一个路径分隔符或者路径结束符 */
		if (PathName[s]) 
		{
			s = CH376FileOpenPath(PathName);
			if (s != USB_INT_SUCCESS && s != ERR_OPEN_DIR) return s;  /* 操作出错 */
		}
		else CH376SetFileName(PathName);  	/* 没有路径分隔符,是根目录或者当前目录下的文件或者目录,设置将要操作的文件的文件名 */
	}
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}

UINT8 CH376FileOpenPath(PUINT8 PathName)  	/* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
{
	return (CH376FileOpenDir(PathName, 0xFF));
}

UINT8 CH376FileCreate(PUINT8 PathName)		/* 在根目录或者当前目录创建文件 */
{
	CH376SetFileName(PathName);
	CH376WriteVar32(VAR_CURRENT_CLUST, 0);
	return (CH376SendCmdWaitInt(CMD0H_FILE_CREATE));
}

UINT8 CH376FileCreatePath(PUINT8 PathName)  /* 新建多级目录下的文件,支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
{
	UINT8 s;
	UINT8 Name;
	Name = CH376SeparatePath(PathName);
	if (Name) 
	{
		s = CH376FileOpenDir(PathName, Name);
		if (s != ERR_OPEN_DIR) 
		{
			if (s == USB_INT_SUCCESS) return (ERR_FOUND_NAME);
			else if (s == ERR_MISS_FILE) return (ERR_MISS_DIR);
			else return s;  /* 操作出错 */
		}
	}
	return (CH376FileCreate(&PathName[Name]));  /* 在根目录或者当前目录下新建文件 */
}

UINT8 CH376CloseFile(UINT8 param)		/* 文件关闭 */
{
	xWriteCH376Cmd(CMD1H_FILE_CLOSE);
	xWriteCH376Data(param);
	return (Wait376Interrupt());
}

void CH376WriteVar8(UINT8 var, UINT8 dat)  /* 写CH376芯片内部的8位变量 */
{
	xWriteCH376Cmd(CMD20_WRITE_VAR8);
	xWriteCH376Data(var);
	xWriteCH376Data(dat);
}

UINT8 CH376ReadVar8(UINT8 var)  /* 读CH376芯片内部的8位变量 */
{
	UINT8	c0;
	xWriteCH376Cmd(CMD11_READ_VAR8);
	xWriteCH376Data(var);
	c0 = xReadCH376Data();
	return c0;
}

void CH376WriteVar32(UINT8 var, UINT32 dat)  /* 写CH376芯片内部的32位变量 */
{
	xWriteCH376Cmd(CMD50_WRITE_VAR32);
	xWriteCH376Data(var);
	xWriteCH376Data((UINT8)dat);
	xWriteCH376Data((UINT8)((UINT16)dat >> 8));
	xWriteCH376Data((UINT8)( dat >> 16 ));
	xWriteCH376Data((UINT8)( dat >> 24 ));
}

UINT32 CH376ReadVar32(UINT8 var)  /* 读CH376芯片内部的32位变量 */
{
	xWriteCH376Cmd(CMD14_READ_VAR32);
	xWriteCH376Data(var);
	return (CH376Read32bitDat());  /* 从CH376芯片读取32位的数据并结束命令 */
}

UINT32 CH376Read32bitDat(void)  /* 从CH376芯片读取32位的数据并结束命令 */
{
	UINT8	c0, c1, c2, c3;
	c0 = xReadCH376Data();
	c1 = xReadCH376Data();
	c2 = xReadCH376Data();
	c3 = xReadCH376Data();
	return (c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24);
}

void CH376Write32bitDat(UINT32 mData)  /* 向CH376芯片发送32位的数据并结束命令 */
{
	xWriteCH376Data((UINT8)mData);
	xWriteCH376Data((UINT8)(mData >> 8));
	xWriteCH376Data((UINT8)(mData >> 16));
	xWriteCH376Data((UINT8)(mData >> 24));
}

void CH376EndDirInfo(void)  /* 在调用CH376DirInfoRead获取FAT_DIR_INFO结构之后应该通知CH376结束 */
{
	CH376WriteVar8(0x0D, 0x00);
}

UINT8 CH376SendCmdWaitInt(UINT8 mCmd)  /* 发出命令码后,等待中断 */
{
	xWriteCH376Cmd(mCmd);
	return (Wait376Interrupt());
}

UINT8 CH376GetIntStatus(void)  /* 获取中断状态并取消中断请求 */
{
	UINT8 i, Data;
	xWriteCH376Cmd(CMD01_GET_STATUS);
	for (i=100; i!=0; i--)
	{
		Data = 	xReadCH376Data();
		if(Data != 0xFF) break;
	} 
	return Data;
}

UINT8 CH376DiskConnect(void)  /* 检查U盘是否连接,不支持SD卡 */
{
	return (CH376SendCmdWaitInt(CMD0H_DISK_CONNECT));
}

UINT8 CH376DiskReqSense(void)  /* 检查USB存储器错误 */
{
	UINT8	s;;
	s = CH376SendCmdWaitInt(CMD0H_DISK_R_SENSE);
	return s;
}

UINT8 CH376DiskMount(void)  /* 初始化磁盘并测试磁盘是否就绪 */
{
	return (CH376SendCmdWaitInt(CMD0H_DISK_MOUNT));
}

UINT8 CH376GetDiskStatus(void)  /* 获取磁盘和文件系统的工作状态 */
{
	return (CH376ReadVar8(VAR_DISK_STATUS));
}

UINT32 CH376GetFileSize(void)  /* 读取当前文件长度 */
{
	return(CH376ReadVar32(VAR_FILE_SIZE));
}

UINT8 CH376SectorWrite(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount)	
/* 以扇区为单位在当前位置写入数据块,不支持SD卡 */
{
	UINT8 	s, cnt;
	UINT32	StaSec;

	if (RealCount) *RealCount = 0;
	do
	{
		xWriteCH376Cmd(CMD1H_SEC_WRITE);
		xWriteCH376Data(ReqCount);
		s = Wait376Interrupt();
		if(USB_INT_SUCCESS != s) return s;
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();
		cnt = xReadCH376Data();				/* 可写扇区数 */
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		StaSec = CH376Read32bitDat();
		if (cnt == 0) break;
		s = CH376DiskWriteSec(buf, StaSec, cnt);
		if (s != USB_INT_SUCCESS) return s;
		buf += cnt * DEF_SECTOR_SIZE;
		if (RealCount) *RealCount += cnt;
		ReqCount -= cnt; 		
	}
	while (ReqCount);
	
	return s;
}

UINT8 CH376SectorRead(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount)		/* 以扇区为单位从当前位置读取数据块,不支持SD卡 */
{
	UINT8	s, cnt;
	UINT32	StaSec;
	if (RealCount) *RealCount = 0;
	do
	{
		xWriteCH376Cmd(CMD1H_SEC_READ);
		xWriteCH376Data(ReqCount);
		s = Wait376Interrupt();
		if (s != USB_INT_SUCCESS) return s;
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();						/* 长度总是sizeof(CH376_CMD_DATA.SectorRead) */
		cnt = xReadCH376Data();					/* CH376_CMD_DATA.SectorRead.mSectorCount */
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		StaSec = CH376Read32bitDat();			/* CH376_CMD_DATA.SectorRead.mStartSector,从CH376芯片读取32位的数据并结束命令 */
		if (cnt == 0) break;
		s = CH376DiskReadSec(buf, StaSec, cnt);	/* 从U盘读取多个扇区的数据块到缓冲区 */
		if (s != USB_INT_SUCCESS) return s;
		buf += cnt * DEF_SECTOR_SIZE;
		if (RealCount) *RealCount += cnt;
		ReqCount -= cnt;
	}
	while (ReqCount);
	return s;
}

UINT8 CH376SecLocate(UINT32 offset)		//以扇区为单位移动当前文件指针
										//扇区偏移量0表示将文件指针移动到文件开头
										//扇区偏移量FFFFFFFFH表示将文件指针移动到文件末尾 最大只能到0X00FF FFFF
{
	xWriteCH376Cmd(CMD4H_SEC_LOCATE);
	xWriteCH376Data((UINT8)offset);
	xWriteCH376Data((UINT8)((UINT16)offset>>8));
	xWriteCH376Data((UINT8)(offset>>16));
	xWriteCH376Data(0);  /* 超出最大文件尺寸 */
	return (Wait376Interrupt());
}

UINT8 CH376DiskWriteSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount)  
/* 将缓冲区中的多个扇区的数据块写入U盘,不支持SD卡 */
/* iLbaStart 是写入的线起始性扇区号, iSectorCount 是写入的扇区数 */
{
	UINT8	s, err;
	UINT16	mBlockCount;
	for (err = 0; err != 3; ++ err) 
	{  /* 出错重试 */  	
		xWriteCH376Cmd(CMD5H_DISK_WRITE);
		xWriteCH376Data((UINT8)iLbaStart);
		xWriteCH376Data((UINT8)((UINT16)iLbaStart >> 8));
		xWriteCH376Data((UINT8)(iLbaStart >> 16));
		xWriteCH376Data((UINT8)(iLbaStart >> 24));
		xWriteCH376Data(iSectorCount);
		for (mBlockCount = iSectorCount * DEF_SECTOR_SIZE / CH376_DAT_BLOCK_LEN; mBlockCount != 0; --mBlockCount)
		{
			s = Wait376Interrupt(); 
			if ( s == USB_INT_DISK_WRITE) 
			{
				CH376WriteHostBlock(buf, CH376_DAT_BLOCK_LEN);  /* 向USB主机端点的发送缓冲区写入数据块 */
				xWriteCH376Cmd(CMD0H_DISK_WR_GO);
				buf += CH376_DAT_BLOCK_LEN;
			}
			else break;  /* 返回错误状态 */
		}
		if (mBlockCount == 0) 
		{
			s = Wait376Interrupt();  /* 等待中断并获取状态 */
			if (s == USB_INT_SUCCESS) return USB_INT_SUCCESS;  /* 操作成功 */
		}
		if (s == USB_INT_DISCONNECT) return s;  /* U盘被移除 */
		CH376DiskReqSense();  /* 检查USB存储器错误 */
	}
	return s;  /* 操作失败 */
}

UINT8 CH376DiskReadSec(PUINT8 buf, UINT32 iLbaStart, UINT8 iSectorCount)  /* 从U盘读取多个扇区的数据块到缓冲区,不支持SD卡 */
/* iLbaStart 是准备读取的线性起始扇区号, iSectorCount 是准备读取的扇区数 */
{
	UINT8	s, err;
	UINT16	mBlockCount;
	for (err = 0; err != 3; ++err)
	{  /* 出错重试 */
		xWriteCH376Cmd(CMD5H_DISK_READ);  /* 从USB存储器读扇区 */
		xWriteCH376Data((UINT8)iLbaStart);  /* LBA的最低8位 */
		xWriteCH376Data((UINT8)((UINT16)iLbaStart >> 8));
		xWriteCH376Data((UINT8)(iLbaStart >> 16));
		xWriteCH376Data((UINT8)(iLbaStart >> 24));  /* LBA的最高8位 */
		xWriteCH376Data(iSectorCount);  /* 扇区数 */
		for (mBlockCount = iSectorCount * DEF_SECTOR_SIZE / CH376_DAT_BLOCK_LEN; mBlockCount != 0; --mBlockCount)
		{  /* 数据块计数 */
			s = Wait376Interrupt();  /* 等待中断并获取状态 */
			if (s == USB_INT_DISK_READ)
			{  /* USB存储器读数据块,请求数据读出 */
				s = CH376ReadBlock(buf);  /* 从当前主机端点的接收缓冲区读取数据块,返回长度 */
				xWriteCH376Cmd(CMD0H_DISK_RD_GO);  /* 继续执行USB存储器的读操作 */
				buf += s;
			}
			else break;  /* 返回错误状态 */
		}
		if (mBlockCount == 0)
		{
			s = Wait376Interrupt();  /* 等待中断并获取状态 */
			if (s == USB_INT_SUCCESS) return USB_INT_SUCCESS;  /* 操作成功 */
		}
		if (s == USB_INT_DISCONNECT) return s;  /* U盘被移除 */
		CH376DiskReqSense();  /* 检查USB存储器错误 */
	}
	return s;  /* 操作失败 */
}

void CH376WriteHostBlock(PUINT8 buf, UINT8 len)  /* 向USB主机端点的发送缓冲区写入数据块 */
{
	xWriteCH376Cmd(CMD10_WR_HOST_DATA);
	xWriteCH376Data(len);  /* 长度 */
	if (len) 
	{
		do 
		{
			xWriteCH376Data(*buf);
			buf ++;
		} 
		while (--len);
	}
}

UINT8 CH376ReadBlock(PUINT8 buf)  /* 从当前主机端点的接收缓冲区读取数据块,返回长度 */
{
	UINT8 s, l;
	xWriteCH376Cmd(CMD01_RD_USB_DATA0);
	s = l = xReadCH376Data();
	if (l)
	{
		do
		{
			*buf = xReadCH376Data();
			buf ++;
		}
		while (--l);
	}
	return s;
}

UINT8 CH376DiskQuery(PUINT32 DiskFree) 		/* 查询磁盘剩余空间信息，扇区数	*/
{
	UINT8	s, c0, c1, c2, c3;
	s = CH376SendCmdWaitInt(CMD0H_DISK_QUERY);
	if (s == USB_INT_SUCCESS) 
	{
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();  // 长度总是sizeof
		xReadCH376Data();  // DiskQuery.mTotalSector
		xReadCH376Data();
		xReadCH376Data();
		xReadCH376Data();
		c0 = xReadCH376Data();  // DiskQuery.mFreeSector
		c1 = xReadCH376Data();
		c2 = xReadCH376Data();
		c3 = xReadCH376Data();
		*DiskFree = c0 | (UINT16)c1 << 8 | (UINT32)c2 << 16 | (UINT32)c3 << 24;
		xReadCH376Data();  // DiskQuery.mDiskFat
	}
	else *DiskFree = 0;
	return s;
}

UINT8 CH376DiskCapacity(PUINT32 DiskCap)  /* 查询磁盘物理容量,扇区数 */
{
	UINT8	s;
	s = CH376SendCmdWaitInt(CMD0H_DISK_CAPACITY);
	if (s == USB_INT_SUCCESS) 
	{
		xWriteCH376Cmd(CMD01_RD_USB_DATA0);
		xReadCH376Data();  /* 长度总是sizeof(CH376_CMD_DATA.DiskCapacity) */
		*DiskCap = CH376Read32bitDat();  /* CH376_CMD_DATA.DiskCapacity.mDiskSizeSec,从CH376芯片读取32位的数据并结束命令 */
	}
	else *DiskCap = 0;
	return s;
}

UINT8 CH376MatchFile(PUINT8 String, PUINT8 PathName, P_FAT_NAME MatchLish)	/* 匹配文件 */
{
	UINT8 s, FileCount, i;
	UINT8 xdata pBuf[64];
	PUINT8 pNameBuf;
	P_FAT_DIR_INFO pDir;
	if (NULL == String) return (DWIN_NULL_POINT);
	s = CH376FileOpenPath(PathName);
	if (ERR_OPEN_DIR != s) return s;		/* 打开的不是目录或者目录不存在 */	
	CH376SetFileName(String);
	xWriteCH376Cmd(CMD0H_FILE_OPEN);
	for (FileCount = 0; FileCount < DIR_FILE_MAX; FileCount++)
	{
		s = Wait376Interrupt();
		if (USB_INT_DISK_READ == s)
		{
			CH376ReadBlock(pBuf);
			xWriteCH376Cmd(CMD0H_FILE_ENUM_GO);
			pDir = (P_FAT_DIR_INFO)pBuf;
			if (pDir -> DIR_Name[0] == '.') continue;	/* . .. 直接跳过 */
			if (pDir -> DIR_Name[0] == 0x05) pDir -> DIR_Name[0] = 0xE5;
			pNameBuf = MatchLish -> NAME;
			for (i = 0; i < 11; i++)					/* 转换成标准文件名 */
			{
				if (pDir -> DIR_Name[i] != 0x20) 
				{  /* 有效字符 */
					if (i == 8) 
					{
						*pNameBuf++ = '.';
					}
					*pNameBuf = pDir -> DIR_Name[i];  /* 复制文件名的一个字符 */
					pNameBuf++;
				}
			}
			*pNameBuf = 0;
			MatchLish -> FILE_SIZE = pDir -> DIR_FileSize;
			MatchLish++;
		}
		else if (ERR_MISS_FILE == s) break;		/* 没有找到更多的匹配文件 */ 		
	}
	if (DIR_FILE_MAX == FileCount)  CH376EndDirInfo();
	return s;
}

UINT8 CH376GetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
{
	UINT8 xdata Buf[64];
	UINT8 Status = 0;
	P_FAT_DIR_INFO pFile;
	memset(Buf, 0, sizeof(Buf));
	Status = CH376FileOpenPath(pFilePath);			
	if (Status != USB_INT_SUCCESS && Status != ERR_OPEN_DIR) 
	{
		CH376CloseFile(0);
		return DWIN_ERROR;
	}
	xWriteCH376Cmd(CMD1H_DIR_INFO_READ);
	xWriteCH376Data(0xFF);		
	Status = Wait376Interrupt();	
	if (Status != USB_INT_SUCCESS)
	{
		CH376CloseFile(0);
		return DWIN_ERROR;
	}
	CH376ReadBlock(Buf);
	pFile = (P_FAT_DIR_INFO)Buf;
	/* 数据存放小端对齐 改为大端对齐和DGUS对应 */
	pDir -> DIR_Attr 		=	(pFile -> DIR_Attr) ;
	pDir -> DIR_CrtTime 	=	(pFile -> DIR_CrtTime << 8) | (pFile -> DIR_CrtTime >> 8);
	pDir -> DIR_CrtDate 	=	(pFile -> DIR_CrtDate << 8) | (pFile -> DIR_CrtDate >> 8);
	pDir -> DIR_LstAccDate	=	(pFile -> DIR_LstAccDate << 8) | (pFile -> DIR_LstAccDate >> 8);
	pDir -> DIR_WrtTime 	=	(pFile -> DIR_WrtTime << 8) | (pFile -> DIR_WrtTime >> 8);
	pDir -> DIR_WrtDate 	=	(pFile -> DIR_WrtDate << 8) | (pFile -> DIR_WrtDate >> 8);
	pDir -> DIR_FileSize 	= 	((pFile -> DIR_FileSize >> 24) & 0x000000FF) | ((pFile -> DIR_FileSize >>  8) & 0x0000FF00) |
								((pFile -> DIR_FileSize <<  8) & 0x00FF0000) | ((pFile -> DIR_FileSize << 24) & 0xFF000000);
	CH376CloseFile(0);
	return DWIN_OK;
}

UINT8 CH376SetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
{
	UINT8 xdata Buf[64];
	UINT8 Status = 0;
	P_FAT_DIR_INFO pFile;
	memset(Buf, 0, sizeof(Buf));
	pFile = (P_FAT_DIR_INFO)Buf;
	Status = CH376FileOpenPath(pFilePath);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	xWriteCH376Cmd(CMD1H_DIR_INFO_READ);
	xWriteCH376Data(0xFF);
	Status = Wait376Interrupt();
	xWriteCH376Cmd(CMD20_WR_OFS_DATA);	/* 发送向CH376内部缓冲区写入数据命令 */
	/* 1、将修改信息发送到BUF缓冲区 */
	/* 2、发送偏移地址 */
	/* 3、写入后续数据长度 */
	/* 4、先写入低8位数据 再写入高8位数据 */
	if (pDir -> DIR_Attr != 0)
	{
		pFile -> DIR_Attr =  pDir -> DIR_Attr;
		xWriteCH376Data(0x0B);
		xWriteCH376Data(1);
		xWriteCH376Data(pFile -> DIR_Attr);
	}
	if (pDir -> DIR_CrtTime != 0)
	{
		pFile -> DIR_CrtTime =  pDir -> DIR_CrtTime;
		xWriteCH376Data(0x0E);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_CrtDate);
		xWriteCH376Data(pFile -> DIR_CrtDate >> 8);
	}
	if (pDir -> DIR_CrtDate != 0)
	{
		pFile -> DIR_CrtDate =  pDir -> DIR_CrtDate;
		xWriteCH376Data(0x10);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_CrtDate);
		xWriteCH376Data(pFile -> DIR_CrtDate >> 8);
	}
	if (pDir -> DIR_WrtTime != 0)
	{
		pFile -> DIR_WrtTime =  pDir -> DIR_WrtTime;
		xWriteCH376Data(0x16);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_WrtTime);
		xWriteCH376Data(pFile -> DIR_WrtTime >> 8);
	}
	if (pDir -> DIR_WrtDate != 0)
	{
		pFile -> DIR_WrtDate =  pDir -> DIR_WrtDate;
		xWriteCH376Data(0x18);
		xWriteCH376Data(2);
		xWriteCH376Data(pFile -> DIR_WrtDate);
		xWriteCH376Data(pFile -> DIR_WrtDate >> 8);
	}
	xWriteCH376Cmd(CMD0H_DIR_INFO_SAVE);	/* 发送保存文件目录信息命令 */
	Status = Wait376Interrupt();
	CH376CloseFile(0);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	return DWIN_OK;	
}
UINT8 CH376WriteReqBlock(PUINT8 pbuf)  /* 向内部指定缓冲区写入请求的数据块,返回长度 */
{
	UINT8	s, l;
	xWriteCH376Cmd(CMD01_WR_REQ_DATA);
	s = l = xReadCH376Data();  /* 长度 */
	if (l) 
	{
		do 
		{
			xWriteCH376Data(*pbuf);
			pbuf ++;
		}
		while (-- l);
	}
	return s;
}
UINT8 CH376CheckNameSum(PUINT8 pDirName)  /* 计算长文件名的短文件名检验和,输入为无小数点分隔符的固定11字节格式 */
{
	UINT8 NameLen;
	UINT8 CheckSum;
	CheckSum = 0;
	for (NameLen = 0; NameLen != 11; NameLen++) 
		CheckSum = (CheckSum & 1 ? 0x80 : 0x00) + (CheckSum >> 1) + *pDirName++;
	return CheckSum;
}
UINT8 CH376ByteLocate(UINT32 offset)  /* 以字节为单位移动当前文件指针 */
{
	xWriteCH376Cmd(CMD4H_BYTE_LOCATE);
	xWriteCH376Data((UINT8)offset);
	xWriteCH376Data((UINT8)((UINT16)offset>>8));
	xWriteCH376Data((UINT8)(offset>>16));
	xWriteCH376Data((UINT8)(offset>>24));
	return Wait376Interrupt();
}
UINT8 CH376ByteRead(PUINT8 pbuf, UINT16 ReqCount, PUINT16 pRealCount)  /* 以字节为单位从当前位置读取数据块 */
{
	UINT8 s;
	xWriteCH376Cmd(CMD2H_BYTE_READ);
	xWriteCH376Data((UINT8)ReqCount);
	xWriteCH376Data((UINT8)(ReqCount>>8));
	if (pRealCount) *pRealCount = 0;
	while (1) 
	{
		s = Wait376Interrupt();
		if (s == USB_INT_DISK_READ) 
		{
			s = CH376ReadBlock(pbuf);  /* 从当前主机端点的接收缓冲区读取数据块,返回长度 */
			xWriteCH376Cmd(CMD0H_BYTE_RD_GO);
			pbuf += s;
			if (pRealCount) *pRealCount += s;
		}
		else return s;  /* 错误 */
	}
}
UINT8 CH376LocateInUpDir(PUINT8 pPathName)  /* 在上级目录(文件夹)中移动文件指针到当前文件目录信息所在的扇区 */
{
	UINT8	s;
	xWriteCH376Cmd(CMD14_READ_VAR32);
	xWriteCH376Data(VAR_FAT_DIR_LBA);  /* 当前文件目录信息所在的扇区LBA地址 */
	for (s = 4; s != 8; s ++) GlobalBuf[s] = xReadCH376Data();  /* 临时保存于全局缓冲区中,节约RAM */
	s = CH376SeparatePath(pPathName);  /* 从路径中分离出最后一级文件名或者目录名,返回最后一级文件名或者目录名的偏移 */
	if (s) s = CH376FileOpenDir(pPathName, s);  /* 是多级目录,打开多级目录下的最后一级目录,即打开文件的上级目录 */
	else s = CH376FileOpen("/");  /* 根目录下的文件,则打开根目录 */
	if (s != ERR_OPEN_DIR) return s;
	*(PUINT32)(&GlobalBuf[0]) = 0;  /* 目录扇区偏移扇区数,保存在全局缓冲区中,节约RAM */
	while (1) 
	{  /* 不断移动文件指针,直到与当前文件目录信息所在的扇区LBA地址匹配 */
		s = CH376SecLocate(*(PUINT32)(&GlobalBuf[0]));  /* 以扇区为单位在上级目录中移动文件指针 */
		if (s != USB_INT_SUCCESS) return s;
		CH376ReadBlock(&GlobalBuf[8]);  /* 从内存缓冲区读取CH376_CMD_DATA.SectorLocate.mSectorLba数据块,返回长度总是sizeof(CH376_CMD_DATA.SectorLocate) */
		if (*(PUINT32)(&GlobalBuf[8]) == *(PUINT32)(&GlobalBuf[4])) return USB_INT_SUCCESS;  /* 已到当前文件目录信息扇区 */
		xWriteCH376Cmd(CMD50_WRITE_VAR32);
		xWriteCH376Data(VAR_FAT_DIR_LBA);  /* 得到前一个扇区,设置为新的文件目录信息扇区LBA地址 */
		for (s = 8; s != 12; s ++) xWriteCH376Data(GlobalBuf[s]);
		++*(PUINT32)(&GlobalBuf[0]);
	}
}
UINT8 CH376LongNameWrite(PUINT8 pbuf, UINT16 ReqCount)  /* 长文件名专用的字节写子程序 */
{
	UINT8 s;
	xWriteCH376Cmd(CMD2H_BYTE_WRITE);
	xWriteCH376Data((UINT8)ReqCount);
	xWriteCH376Data((UINT8)(ReqCount>>8));
	while (1) 
	{
		s = Wait376Interrupt();
		if (s == USB_INT_DISK_WRITE) 
		{
			if (pbuf) pbuf += CH376WriteReqBlock(pbuf);  /* 向内部指定缓冲区写入请求的数据块,返回长度 */
			else 
			{
				xWriteCH376Cmd(CMD01_WR_REQ_DATA);  /* 向内部指定缓冲区写入请求的数据块 */
				s = xReadCH376Data();  /* 长度 */
				while (s--) xWriteCH376Data(0);  /* 填充0 */
			}
			xWriteCH376Cmd(CMD0H_BYTE_WR_GO);
		}
		else return s;  /* 错误 */
	}
}
UINT8 CH376CreateLongName(PUINT8 pPathName, PUINT8 pLongName)  /* 新建具有长文件名的文件,关闭文件后返回,LongName输入路径必须在RAM中 */
{
	UINT8 s, i;
	UINT8 DirBlockCnt;		/* 长文件名占用文件目录结构的个数 */
	UINT16 count;			/* 临时变量,用于计数,用于字节读文件方式下实际读取的字节数 */
	UINT16 NameCount;		/* 长文件名字节计数 */
	UINT32 NewFileLoc;		/* 当前文件目录信息在上级目录中的起始位置,偏移地址 */
	for (count = 0; count < LONG_NAME_BUF_LEN; count += 2) if (*(PUINT16)(&pLongName[count]) == 0) break;  /* 到结束位置 */
	if (count == 0 || count >= LONG_NAME_BUF_LEN || count > LONE_NAME_MAX_CHAR) return ERR_LONG_NAME_ERR;  /* 长文件名无效 */
	DirBlockCnt = count / LONG_NAME_PER_DIR;  /* 长文件名占用文件目录结构的个数 */
	i = count - DirBlockCnt * LONG_NAME_PER_DIR;
	if (i) 
	{  /* 有零头 */
		if (++DirBlockCnt * LONG_NAME_PER_DIR > LONG_NAME_BUF_LEN) return ERR_LONG_BUF_OVER;  /* 缓冲区溢出 */
		count += 2;  /* 加上0结束符后的长度 */
		i += 2;
		if (i < LONG_NAME_PER_DIR) 
		{  /* 最末的文件目录结构不满 */
			while (i++ < LONG_NAME_PER_DIR) pLongName[count++] = 0xFF;  /* 把剩余数据填为0xFF */
		}
	}
	s = CH376FileOpenPath(pPathName);  /* 打开多级目录下的文件 */
	if (s == USB_INT_SUCCESS) 
	{   /* 短文件名存在则返回错误 */
		s = ERR_NAME_EXIST;
		goto CH376CreateLongNameE;
	}
	if (s != ERR_MISS_FILE) return s;
	s = CH376FileCreatePath(pPathName);  /* 新建多级目录下的文件 */
	if (s != USB_INT_SUCCESS) return s;
	i = CH376ReadVar8(VAR_FILE_DIR_INDEX);  /* 临时用于保存当前文件目录信息在扇区内的索引号 */
	s = CH376LocateInUpDir(pPathName);  /* 在上级目录中移动文件指针到当前文件目录信息所在的扇区 */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;  /* 没有直接返回是因为如果是打开了根目录那么必须要关闭后才能返回 */
	NewFileLoc = CH376ReadVar32(VAR_CURRENT_OFFSET) + i * sizeof(FAT_DIR_INFO);  /* 计算当前文件目录信息在上级目录中的起始位置,偏移地址 */
	s = CH376ByteLocate(NewFileLoc);  /* 在上级目录中移动文件指针到当前文件目录信息的位置 */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	s = CH376ByteRead(&GlobalBuf[ sizeof(FAT_DIR_INFO)], sizeof(FAT_DIR_INFO), NULL);  /* 以字节为单位读取数据,获得当前文件的目录信息FAT_DIR_INFO */
	if ( s != USB_INT_SUCCESS ) goto CH376CreateLongNameE;
	for (i = DirBlockCnt; i != 0; --i) 
	{  /* 搜索空闲的文件目录结构用于存放长文件名 */
		s = CH376ByteRead(GlobalBuf, sizeof(FAT_DIR_INFO), &count);  /* 以字节为单位读取数据,获得下一个文件目录信息FAT_DIR_INFO */
		if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
		if (count == 0) break;  /* 无法读出数据,上级目录结束了 */
		if (GlobalBuf[0] && GlobalBuf[0] != 0xE5) 
		{  /* 后面有正在使用的文件目录结构,由于长文件名必须连接存放,所以空间不够,必须放弃当前位置并向后转移 */
			s = CH376ByteLocate(NewFileLoc);  /* 在上级目录中移动文件指针到当前文件目录信息的位置 */
			if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			GlobalBuf[0] = 0xE5;  /* 文件删除标志 */
			for (s = 1; s != sizeof(FAT_DIR_INFO); s ++) GlobalBuf[s] = GlobalBuf[sizeof(FAT_DIR_INFO) + s];
			s = CH376LongNameWrite(GlobalBuf, sizeof(FAT_DIR_INFO));  /* 写入一个文件目录结构,用于删除之前新建的文件,实际上稍后会将之转移到目录的最末位置 */
			if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			do 
			{  /* 向后搜索空闲的文件目录结构 */
				s = CH376ByteRead(GlobalBuf, sizeof(FAT_DIR_INFO), &count);  /* 以字节为单位读取数据,获得下一个文件目录信息FAT_DIR_INFO */
				if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
			} 
			while (count && GlobalBuf[0]);  /* 如果仍然是正在使用的文件目录结构则继续向后搜索,直到上级目录结束或者有尚未使用过的文件目录结构 */
			NewFileLoc = CH376ReadVar32(VAR_CURRENT_OFFSET);  /* 用上级目录的当前文件指针作为当前文件目录信息在上级目录中的起始位置 */
			i = DirBlockCnt + 1;  /* 需要的空闲的文件目录结构的个数,包括短文件名本身一个和长文件名 */
			if (count == 0) break;  /* 无法读出数据,上级目录结束了 */
			NewFileLoc -= sizeof(FAT_DIR_INFO);  /* 倒回到刚才搜索到的空闲的文件目录结构的起始位置 */
		}
	}
	if (i) 
	{  /* 空闲的文件目录结构不足以存放长文件名,原因是上级目录结束了,下面增加上级目录的长度 */
		s = CH376ReadVar8(VAR_SEC_PER_CLUS);  /* 每簇扇区数 */
		if (s == 128) 
		{  /* FAT12/FAT16的根目录,容量是固定的,无法增加文件目录结构 */
			s = ERR_FDT_OVER;  /* FAT12/FAT16根目录下的文件数应该少于512个,需要磁盘整理 */
			goto CH376CreateLongNameE;
		}
		count = s * DEF_SECTOR_SIZE;  /* 每簇字节数 */
		if (count < i * sizeof(FAT_DIR_INFO)) count <<= 1;  /* 一簇不够则增加一簇,这种情况只会发生于每簇为512字节的情况下 */
		s = CH376LongNameWrite( NULL, count );  /* 以字节为单位向当前位置写入全0数据块,清空新增加的文件目录簇 */
		if ( s != USB_INT_SUCCESS ) goto CH376CreateLongNameE;
	}
	s = CH376ByteLocate(NewFileLoc);  /* 在上级目录中移动文件指针到当前文件目录信息的位置 */
	if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	GlobalBuf[11] = ATTR_LONG_NAME;
	GlobalBuf[12] = 0x00;
	GlobalBuf[13] = CH376CheckNameSum(&GlobalBuf[sizeof(FAT_DIR_INFO)]);  /* 计算长文件名的短文件名检验和 */
	GlobalBuf[26] = 0x00;
	GlobalBuf[27] = 0x00;
	for (s = 0; DirBlockCnt != 0;) 
	{  /* 长文件名占用的文件目录结构计数 */
		GlobalBuf[0] = s ? DirBlockCnt : DirBlockCnt | 0x40;  /* 首次要置长文件名入口标志 */
		DirBlockCnt--;
		NameCount = DirBlockCnt * LONG_NAME_PER_DIR;
		for (s = 1; s < sizeof( FAT_DIR_INFO ); s += 2) 
		{  /* 输出长文件名,长文件名的字符在磁盘上UNICODE用小端方式存放 */
			if (s == 1 + 5 * 2) s = 14;  /* 从长文件名的第一组1-5个字符跳到第二组6-11个字符 */
			else if (s == 14 + 6 * 2) s = 28;  /* 从长文件名的第二组6-11个字符跳到第三组12-13个字符 */
			GlobalBuf[s] = pLongName[NameCount++];
			GlobalBuf[s + 1] = pLongName[NameCount++];
		}
		s = CH376LongNameWrite(GlobalBuf, sizeof(FAT_DIR_INFO));  /* 以字节为单位写入一个文件目录结构,长文件名 */
		if (s != USB_INT_SUCCESS) goto CH376CreateLongNameE;
	}
	s = CH376LongNameWrite(&GlobalBuf[ sizeof(FAT_DIR_INFO)], sizeof(FAT_DIR_INFO));  /* 以字节为单位写入一个文件目录结构,这是转移来的之前新建的文件的目录信息 */
CH376CreateLongNameE:
	CH376CloseFile(FALSE);  /* 对于根目录则必须要关闭 */
	return s;
}