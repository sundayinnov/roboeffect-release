/**
  ******************************************************************************
  * @file    core_d1088.h
  * @author  Peter Zhai
  * @version V1.0
  * @date    2017-10-27
  * @brief
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/**
 * @addtogroup SYSTEM
 * @{
 * @defgroup core_d1088 core_d1088.h
 * @{
 */

#ifndef __CORE_D1088_H__
#define __CORE_D1088_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include <nds32_intrinsic.h>

#include "debug.h"
#include "type.h"
#define CACHE_DBG(format, ...)		//DBG(format, ##__VA_ARGS__)

#define CACHEABILITY_DEVICE   	   0  //device space
#define CACHEABILITY_NO_CACHE 	   2  //non-cacheable memory
#define CACHEABILITY_WRITE_BACK    4  //cacheable, write-back, write-allocate memory
#define CACHEABILITY_WRITE_THROUGH 5  //cacheable, write-through, no-write-allocate memory

#define EXECUTABLE_DISABLE 		0 //None executable page for all modes
#define EXECUTABLE_PRIVILEGED   2 //Privileged code
#define EXECUTABLE_USER         3 //User code

#define ACCESS_READ 1 //Read only
#define ACCESS_RW   3 //Read/Write

#define MPU_ENTRY_DISABLE   0
#define MPU_ENTRY_ENABLE    1

extern const uint32_t MPUConfigTable[8][7];

/**
 * @brief	Ê¹ÄÜIDCache
 * @param	None
 * @return	None
 * @note
 */
void EnableIDCache(void);

/**
 * @brief	½ûÄÜIDCache
 * @param	None
 * @return	None
 * @note
 */
void DisableIDCache(void);

#define		McuCycleCountStart()	__nds32__mtsr(0, NDS32_SR_PFMC0);\
									__nds32__mtsr(1, NDS32_SR_PFM_CTL);

#define		McuCycleCountend()		({\
									uint32_t cycle;\
									__nds32__mtsr(0, NDS32_SR_PFM_CTL);\
									cycle =  __nds32__mfsr(NDS32_SR_PFMC0);\
									})


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __CORE_D1088_H__ 

/**
 * @}
 * @}
 */

