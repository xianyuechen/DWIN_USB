/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : dgus.c
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月21日
功能描述   : 提供单片机访问DGUS变量接口 文件升级相关的接口定义
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#include "dgus.h"

void UpdateSet(PUINT8 pBuf, UINT8 Flag_EN, UINT8 UpSpace, UINT32 UpAddr, UINT16 FileSize);
/*****************************************************************************
 函 数 名  : ReadDGUS
 功能描述  : 读DGUS寄存器
 输入参数  : UINT32 Addr  DGUS寄存器地址
             PUINT8 pBuf  接收缓冲区
			 UINT16 Len   读取数据字节长度	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void ReadDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset = 0;
	UINT8 EndLen = 0;
	Offset = Addr & 0x01;
	Addr = Addr >> 1;
	ADR_H = (UINT8)(Addr >> 16);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_L = (UINT8)(Addr);
	ADR_INC = 1;
	RAMMODE = 0xAF;
	while (!APP_ACK);
	if (Offset == 1)			/* 地址偏移修正 */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA1;
		if (Len == 1) return;
        *pBuf++ = DATA0;
		Len -= 2;
	}
	EndLen = Len & 0x03;		/* 尾部长度 0-3 */
	Len = Len >> 2;
	while (Len--)
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		*pBuf++ = DATA2;
		*pBuf++ = DATA1;
		*pBuf++ = DATA0;
	}
	if (EndLen)					/* 尾部有数据 根据数据长度选择读取次数 */
	{
		APP_EN = 1;
		while (APP_EN);
		*pBuf++ = DATA3;
		if (EndLen & 2) *pBuf++ = DATA2;
		if (EndLen & 3) *pBuf++ = DATA1;
	}
	RAMMODE = 0x00;	 
}
/*****************************************************************************
 函 数 名  : WriteDGUS
 功能描述  : 写DGUS寄存器
 输入参数  : UINT32 Addr  DGUS寄存器地址
             PUINT8 pBuf  发送缓冲区
			 UINT16 Len   发送数据字节长度	 
 输出参数  : 无
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
void WriteDGUS(UINT32 Addr, PUINT8 pBuf, UINT16 Len)
{
	UINT8 Offset;
	UINT8 EndLen;
	Offset = Addr & 1;
	Addr >>= 1;
	ADR_L = (UINT8)(Addr);
	ADR_M = (UINT8)(Addr >> 8);
	ADR_H = (UINT8)(Addr >> 16);
	ADR_INC = 1;
	RAMMODE = 0x8F;
	while (!APP_ACK);
	if (Offset == 1)			/* 地址偏移修正 */
	{
		ADR_INC = 0;
        RAMMODE = 0xAF;
        APP_EN = 1;
        while (APP_EN);			/* 读写执行结束	*/

        ADR_INC = 0x01; 
        RAMMODE = 0x8F;     
        DATA1 = *pBuf++;
		if (Len == 1) return;
        DATA0 = *pBuf++;
        APP_EN = 1;
        while (APP_EN);			/* 读写执行结束	*/
        Len -= 2;
	}
	EndLen = Len & 0x03;		/* 尾部长度 0-3 */
	Len >>=2;
	while (Len--)
	{
		DATA3 = *pBuf++;
		DATA2 = *pBuf++;
		DATA1 = *pBuf++;
		DATA0 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	if (EndLen)					/* 尾部有数据 根据数据长度选择写入次数 */
	{
		DATA3 = *pBuf++;
		if (EndLen & 2) DATA2 = *pBuf++;
		if (EndLen & 3) DATA1 = *pBuf++;
		APP_EN = 1;
		while (!APP_EN);
	}
	RAMMODE = 0x00;
}
/*****************************************************************************
 函 数 名  : SystemUpdate
 功能描述  : 系统升级
 输入参数  : UINT8 FileType  升级文件类型	 
 输出参数  : DWIN_OK 成功
             其他    失败
 修改历史  :
 日    期  : 2019年6月21日
 作    者  : chenxianyue
 修改内容  : 创建
*****************************************************************************/
UINT8 SystemUpdate(UINT8 FileType)
{
	UINT8 xdata Buf[BUF_SIZE + CONTROL_SIZE];
	UINT8 xdata FileName[20];	/* 长度 8 + '.' + 3 */
	UINT8 Status = 0;
	UINT8 UpSpace = 0;
	PUINT8 pBufFile = Buf;
	UINT32 FileAddr = 0;
	UINT32 AddrBuff = 0;
	UINT32 FileSize = 0;	
	memset(Buf, 0, sizeof(Buf));
	memset(FileName, 0, sizeof(FileName));
	/* (1) 根据文件类型 设定相关参数 */
	switch (FileType)
	{
		case FILE_T5L51_BIN:
		{
			strcpy(FileName, "T5L51*");
			FileAddr = ADDR_T5L51_BIN;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_DWINOS_BIN:
		{
			strcpy(FileName, "DWINOS*");
			FileAddr = ADDR_DWIN_OS;
			UpSpace = SPACE_1;
			break;
		}

		case FILE_XXX_LIB:
		{
			UpSpace = SPACE_1;
			break;
		}
		case FILE_ALL:
		{
			//SystemUpdate(FILE_T5L51_BIN);
			//SystemUpdate(FILE_DWINOS_BIN);
			//SystemUpdate(FILE_XXX_LIB);
			break;
		}
		default:
			return UPDATE_FAILED;
	}
	/* (2) 查找文件 存在则读取文件信息到后4K缓冲区 */
	Status = FindDWINFile(FileName);	/* 查找目标文件名 */
	if (Status != DWIN_OK) return Status;
	pBufFile += CONTROL_SIZE;			/* 切换到数据保存区域 */
	Status = CH376ReadFile(FileName, pBufFile, &FileSize);
	if (Status != USB_INT_SUCCESS) return DWIN_ERROR;
	/* (3) 设置前512字节控制字信息 */
	ReadDGUS(ADDR_UP_SET, Buf, 4);			
	AddrBuff = (Buf[3] << 8) & 0xFF00;	/* 获取升级DGUS地址 */
	ReadDGUS(AddrBuff, Buf, 1);
	if (Buf[0] == FLAG_NO_EN) 
	{									/* 设置更新参数 */
		UpdateSet(Buf, FLAG_NO_EN, SPACE_1, FileAddr, (UINT16)FileSize);
	}
	else return DWIN_ERROR;
										/* 首次写入不启动升级 防止升级出错 */
	WriteDGUS(AddrBuff, Buf, (UINT16)FileSize + CONTROL_SIZE);	
	ReadDGUS(AddrBuff, Buf, 1);
	/* (4) 首位写入升级标志 开启升级 */
	if (Buf[0] == FLAG_NO_EN)
	{
		Buf[0] = FLAG_EN;				/* 启动升级 */
		WriteDGUS(AddrBuff, Buf, 4);
	}
	SendString(Buf, BUF_SIZE + CONTROL_SIZE);
	return DWIN_OK;
}

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