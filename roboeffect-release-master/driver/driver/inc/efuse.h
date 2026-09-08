/*
 * efuse.h
 *
 *  Created on: Otc 26, 2017
 *      Author: jerry_rao
 */
/**
 * @addtogroup EFUSE
 * @{
 * @defgroup efuse efuse.h
 * @{
 */

#ifndef __EFUSE_H__
#define __EFUSE_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * @addtogroup EFUSE
 * @{
 * @defgroup efuse efuse.h
 * @{
 */
 
#include "type.h"

/**
 * @brief  读取Efuse中指定地址的数据
 * @param  Addr为Efuse中指定的地址，十六进制表示
 *         可读的地址为：0x00~0xFA，其中
 *             (1)客户自定义的地址：0xC1~0xC8，共8个字节地址空间，默认为全0，可读写
 *             (2)芯片唯一的ID编号：0x0B~0x12，共8个字节地址空间，出厂已有值
 * @return 读取到指定地址的数据
 */
uint8_t Efuse_ReadData(uint8_t Addr);


/**
 *
 * @brief  禁止读取Efuse地址数据
 *
 */
void Efuse_ReadDataDisable();


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __EFUSE_H__ 
/**
 * @}
 * @}
 */

