/**
 **************************************************************************************
 * @file    chip_info.h
 * @brief	define all chip info
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2018-1-15 19:46:00$
 *
 * @Copyright (C) 2016, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup chip_info chip_info.h
 * @{
 */
 
#ifndef __CHIP_INFO_H__
#define __CHIP_INFO_H__

#ifdef  __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

#define		CHIP_VERSION_ECO1		0xFFFF
#define		CHIP_VERSION_ECO2		0xFFFD

uint16_t Chip_Version(void);

//LDOVol: 0: 低压(3.6V)； 1：高压(大于3.6V)
void Chip_Init(uint32_t LDOVol);

#define		HSOC_GPIO_CONFIG()		if(Chip_Version() == CHIP_VERSION_ECO1){\
									GPIO_RegOneBitClear(GPIO_B_PU, GPIO_INDEX4);\
									GPIO_RegOneBitSet(GPIO_B_PD, GPIO_INDEX4);\
									GPIO_RegOneBitClear(GPIO_B_PU, GPIO_INDEX5);\
									GPIO_RegOneBitSet(GPIO_B_PD, GPIO_INDEX5);\
									}

const unsigned char *GetLibVersionDriver(void);

const unsigned char *GetLibVersionLrc(void);

const unsigned char *GetLibVersionRTC(void);

const unsigned char *GetLibVersionFatfsACC(void);


/**
 * @brief	获取芯片的唯一ID号
 * @param	64bit ID号
 * @return	无
 */
void Chip_IDGet(uint64_t* IDNum);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif /*__CHIP_INFO_H__*/

/**
 * @}
 * @}
 */

