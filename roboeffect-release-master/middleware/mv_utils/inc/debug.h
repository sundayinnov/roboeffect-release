////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2011, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:debug.h
//
//		Description	:
//					Define debug ordinary print & debug routine
//
//		Changelog	:
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include <stdio.h>
#include "type.h"
#include "user_config.h"
#if CFG_DEBUG_EN
#define	DBG(format, ...)			printf(format, ##__VA_ARGS__)

#define	APP_DBG(format, ...)		printf(format, ##__VA_ARGS__)

#define	OTG_DBG(format, ...)		//printf(format, ##__VA_ARGS__)

#else
#define	DBG(format, ...)

#define	APP_DBG(format, ...)

#define	OTG_DBG(format, ...)
#endif

int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__DBG_H__ 

