/******************************************************************************
																	
                  版权所有 (C), 2019, 北京迪文科技有限公司	
																			  
*******************************************************************************
文 件 名   : file_sys.h
版 本 号   : V1.0
作    者   : chenxianyue
生成日期   : 2019年6月4日
功能描述   : CH376驱动程序接口
修改历史   :
日    期   : 
作    者   : 
修改内容   : 	
******************************************************************************/
#ifndef _FILE_SYS_H_
#define _FILE_SYS_H_

#include "driver/system/sys.h"
#include "driver/usb/ch376.h"
/********************************宏定义**************************************/
#define	ERR_USB_UNKNOWN					(0xFA)	/* 未知错误,不应该发生的情况,需检查硬件或者程序错误 */

#define DIR_FILE_MAX					(40)	/* 目录下枚举最多文件数 */
#define PATH_NUMBER						(64)	/* 绝对路径数量 */

/********************************结构体声明***********************************/
typedef struct _FAT_NAME
{
	UINT8 NAME[20];							/* 文件名/路径名 相对路径 */
	UINT8 Attr;								/* 文件属性 */
}FAT_NAME, *P_FAT_NAME;

/********************************对外函数声明*********************************/
void AlphabetTransfrom(PUINT8 name);		/* 小写文件名统一转换为大写文件名 */

/* 文件操作 */
UINT8 CH376Error(void);
UINT8 CH376FileOpen(PUINT8 name);			/* 在根目录或者当前目录下打开文件或者目录(文件夹) */
UINT8 CH376DirCreate(PUINT8 PathName);		/* 在根目录或者当前目录创建目录 */
UINT8 CH376FileCreate(PUINT8 PathName);		/* 在根目录或者当前目录创建文件 */
UINT8 CH376DeleteFile(PUINT8 pName);		/* 在根目录或者当前目录删除文件或者目录 */
UINT8 CH376FileDeletePath(PUINT8 PathName);	/* 删除文件,如果已经打开则直接删除,否则对于文件会先打开再删除,支持多级目录路径 */
UINT8 CH376FileOpenPath(PUINT8 PathName);	/* 打开多级目录下的文件或者目录(文件夹),支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
UINT8 CH376FileCreatePath(PUINT8 PathName);	/* 新建多级目录下的文件,支持多级目录路径,支持路径分隔符,路径长度不超过255个字符 */
UINT8 CH376CloseFile(UINT8 param);			/* 文件关闭 */
UINT8 CH376MatchFile(PUINT8 String, PUINT8 PathName, P_FAT_NAME MatchLish);	/* 匹配文件 */
UINT8 GetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir);				/* 获取文件属性 */
UINT8 SetFileMessage(PUINT8 pFilePath, P_FAT_DIR_INFO pDir);				/* 修改文件属性 */
/* 磁盘信息 */
UINT8 CH376DiskConnect(void);				/* 检查U盘是否连接,不支持SD卡 */
UINT8 CH376DiskMount(void);					/* 初始化磁盘并测试磁盘是否就绪 */
UINT8 CH376GetDiskStatus(void);				/* 获取磁盘和文件系统的工作状态 */
/* 扇区操作	*/
void CH376WriteVar32(UINT8 var, UINT32 dat);
UINT32 CH376GetFileSize(void);				/* 读取当前文件长度 */											
UINT8 CH376SectorWrite(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount);	/* 以扇区为单位在当前位置写入数据块,不支持SD卡 */
UINT8 CH376SectorRead(PUINT8 buf, UINT8 ReqCount, PUINT8 RealCount);	/* 以扇区为单位从当前位置读取数据块,不支持SD卡 */
UINT8 CH376SecLocate(UINT32 offset);		/* 以扇区为单位移动当前文件指针	*/

UINT8	CH376CreateLongName( PUINT8 PathName, PUINT8 LongName );
#endif
