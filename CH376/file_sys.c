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
#include <string.h>

/********************************内部函数声明*********************************/
UINT8	Wait376Interrupt(void);
UINT8	Query376Interrupt(void);
void	CH376SetFileName(PUINT8 name);
UINT8	CH376SeparatePath(PUINT8 path);
UINT8	CH376FileOpenDir(PUINT8 PathName, UINT8 StopName);
UINT8	CH376FileCreate(PUINT8 PathName);
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

/*********************************函数实现*************************************/
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

UINT8 CH376FileOpen(PUINT8 name)  /* 在根目录或者当前目录下打开文件或者目录(文件夹) */
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
		else CH376SetFileName(PathName);  /* 没有路径分隔符,是根目录或者当前目录下的文件或者目录,设置将要操作的文件的文件名 */
	}
	return (CH376SendCmdWaitInt(CMD0H_FILE_ERASE));
}

UINT8 CH376FileOpenPath(PUINT8 PathName)  /* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
{
	return (CH376FileOpenDir(PathName, 0xFF));
}

UINT8 CH376FileCreate(PUINT8 PathName)		/* 在根目录或者当前目录创建文件 */
{
	CH376SetFileName(PathName);
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

UINT8 CH376DiskQuery(PUINT32 DiskFree) 		//查询磁盘剩余空间信息，扇区数
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
			MatchLish -> Attr = pDir -> DIR_Attr;
			MatchLish++;
		}
		else if (ERR_MISS_FILE == s) break;		/* 没有找到更多的匹配文件 */ 		
	}
	if (DIR_FILE_MAX == FileCount)  CH376EndDirInfo();
	return s;
}

UINT8 GetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
{
	UINT8 xdata Buf[64];
	UINT8 Status = 0;
	P_FAT_DIR_INFO pFile;
	memset(Buf, 0, sizeof(Buf));
	
	Status = CH376FileOpenPath(pFilePath);
	UART5_Sendbyte(Status);	
	if (Status != USB_INT_SUCCESS) 
	{
		CH376CloseFile(0);
		return DWIN_ERROR;
	}
	xWriteCH376Cmd(CMD1H_DIR_INFO_READ);	
	Status = Wait376Interrupt();
	UART5_Sendbyte(Status);	
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
	pDir -> DIR_FileSize 	= 	(pFile -> DIR_FileSize >> 24) | ((pFile -> DIR_FileSize >> 8) & 0xFF00) |
								((pFile -> DIR_FileSize << 8) & 0xFF0000) | ((pFile -> DIR_FileSize << 24) & 0xFF000000);
	CH376CloseFile(0);
	return DWIN_OK;
}

UINT8 SetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir)
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