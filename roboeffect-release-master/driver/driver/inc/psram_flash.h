/**
 *****************************************************************************
 * @file     psram_flash.h
 * @author   Peter
 * @version  V0.0.1
 * @date     09-13-2018
 * @brief
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2018 MVSilicon </center></h2>
 */

 /**
 * @addtogroup PSRAM_FLASH
 * @{
 * @defgroup psram_flash psram_flash.h
 * @{
 */

#ifndef SRC_PSRAM_FLASH_H_
#define SRC_PSRAM_FLASH_H_

#include "type.h"

/**
 * @brief 初始化psram接口
 * @param	none
 * @return	none
 * @note
 */
void Psram_Fshc_Init(void);

/**
 * @brief  获取flash信息
 * @param  none
 * @return 返回flash信息
 * @note this function should be called after SpiFlashInit
 */
SPI_FLASH_INFO* Psram_FlashInfo_Get(void);

/**
 * @brief  获取flash状态
 * @param  none
 * @return 0-idle 1-busy
 * @note this function should be called after SpiFlashInit
 */
uint32_t Psram_Flash_IsBusy(void);

/**
 * @brief flash整片擦除
 * @param	none
 * @return	none
 * @note
 */
void Psram_Flash_Chip_Erase(void);

/**
 * @brief	擦除flash
 * @param	Offset	 擦除地址，必须4KB对齐
 * @param	Size	擦除size，必须4KB对齐
 * @return	ERASE_FLASH_ERR-擦除失败	FLASH_NONE_ERR-擦出成功
 * @note
 */
int32_t Psram_Flash_Erase(uint32_t Offset,uint32_t Size);

/**
 * @brief	sector擦除
 * @param	Offset	 擦除地址，必须4KB对齐
 * @return	ERASE_FLASH_ERR-擦除失败	FLASH_NONE_ERR-擦出成功
 * @note	函数返回之后，需要使用Psram_Flash_IsBusy判断是否擦除完成
 */
int32_t Psram_Flash_Sector_Erase_NoWait(uint32_t Offset);

/**
 * @brief	block(32K)擦除
 * @param	Offset	 擦除地址，必须32KB对齐
 * @return	ERASE_FLASH_ERR-擦除失败	FLASH_NONE_ERR-擦出成功
 * @note	函数返回之后，需要使用Psram_Flash_IsBusy判断是否擦除完成
 */
int32_t Psram_Flash_Block32k_Erase_NoWait(uint32_t Offset);

/**
 * @brief	block(64K)擦除
 * @param	Offset	 擦除地址，必须64KB对齐
 * @return	ERASE_FLASH_ERR-擦除失败	FLASH_NONE_ERR-擦出成功
 * @note	函数返回之后，需要使用Psram_Flash_IsBusy判断是否擦除完成
 */
int32_t Psram_Flash_Block64k_Erase_NoWait(uint32_t Offset);

/**
 * @brief	读flash
 * @param	addr 地址，必须4Byte对齐
 * @param	buf 数据buf
 * @param	len 长度
 * @return	FLASH_NONE_ERR-成功
 * @note
 */
int32_t Psram_Flash_Read(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief	读flash
 * @param	addr 地址，必须4Byte对齐
 * @param	buf 数据buf，必须4Byte对齐
 * @param	len 长度，必须4Byte对齐
 * @return	FLASH_NONE_ERR-成功
 * @note	函数返回后，需要判断DMA是否完成
 */
int32_t Psram_Flash_Read_DMA(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief	写flash
 * @param	addr 地址
 * @param	buf 数据buf
 * @param	len 长度
 * @return	FLASH_NONE_ERR-成功
 * @note
 */
int32_t Psram_Flash_Write(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief	写flash
 * @param	addr 地址,必须4Byte对齐
 * @param	buf 数据buf,必须4Byte对齐
 * @param	len 长度,必须4Byte对齐
 * @return	FLASH_NONE_ERR-成功
 * @note	addr+len不允许跨page，函数返回后，需要判断DMA和写是否完成
 */
int32_t Psram_Flash_Page_Write_DMA(uint32_t addr, uint8_t *buf, uint32_t len);

/**
 * @brief 解锁-flash全部解锁
 * @param	none
 * @return	none
 * @note
 */
void Psram_Flash_unLock(void);

/**
 * @brief 加锁
 * @param	none
 * @return	none
 * @note
 */
void Psram_Flash_Lock(void);

#endif /* SRC_PSRAM_FLASH_H_ */
