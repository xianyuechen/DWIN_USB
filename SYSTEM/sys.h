#include "t5los8051.h"

#ifndef _SYS_H_
#define _SYS_H_

#ifndef UINT8
typedef unsigned char	UINT8;
#endif
#ifndef UINT16
typedef unsigned int	UINT16;
#endif
#ifndef UINT32
typedef unsigned long	UINT32;
#endif
#ifndef INT8
typedef char            INT8;
#endif
#ifndef INT16
typedef int           	INT16;
#endif
#ifndef INT32
typedef long			INT32;
#endif
#ifndef PUINT8
typedef unsigned char	*PUINT8;
#endif
#ifndef PUINT16
typedef unsigned int	*PUINT16;
#endif
#ifndef PUINT32
typedef unsigned long	*PUINT32;
#endif
#ifndef UINT8V
typedef unsigned char volatile	UINT8V;
#endif
#ifndef PUINT8V
typedef unsigned char volatile	*PUINT8V;
#endif


#define DWIN_OK							(0x00)
#define DWIN_ERROR						(0xFF)
#define DWIN_NULL_POINT					(0x02)
#define DWIN_PARAM_ERROR				(0x03)
#define NULL							((void *)0)
#define FOSC							(206438400UL)
#define T1MS    						(65536-FOSC/12/1000)
void INIT_CPU(void);

#endif
