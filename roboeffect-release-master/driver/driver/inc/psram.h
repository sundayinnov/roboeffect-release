/**
 *******************************************************************************
 * @file    psram.h
 * @brief	The driver of psram module
 *
 * @author  shengqi_zhao
 * @version V1.0.0
 *
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup PSRAM
 * @{
 * @defgroup psram psram.h
 * @{
 */
#ifndef   __PSRAM_H__
#define   __PSRAM_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

#include "type.h"

/**
 * @brief  The base address of psram
 */
#define PSRAM_BASE  0x60000000


/**
 * @brief The acess mode of Psram
 * @SPI_1BIT_MODE   cmd 1bit addr 1bit data 1bit
 * @SPI_QUAD_MODE   cmd 1bit addr 4bit data 4bit
 * @SPI_QPI_MODE    cmd 4bit addr 4bit data 4bit
*/
typedef enum{
	SPI_1BIT_MODE = 0x01,
	SPI_QUAD_MODE = 0x02,
	SPI_QPI_MODE  = 0x03,
}PSRAM_TYPE;


/**
 * @brief intial the access mode and param of psram, then the psram can be accessed as normal ram
 *
 * @param[in]   mode              speicifiy the cmd/data access length of psram,see  PSRAM_TYPE definiton
 * @param[in]   burstlen,         the psram can be accessed by burst mode,this param set to promote access efficiency, 0 means burst off
 * @param[in]   read_wait_cycle   set according to the datasheet wrote you use
 * @param[in]   write_wait_cycle  set according to the datasheet wrote you use
 * @param[in]   hold_cycle        set according to the datasheet wrote you use
 * @return		none
 */
void Psram_Init(PSRAM_TYPE mode, uint16_t burstlen, uint8_t read_wait_cycle, uint8_t    write_wait_cycle, uint8_t hold_cycle);


/**
 * @brief  exit from quad mode ,then can initial the psram again
 *
 * @param[in]    none
 * @return   	 none
 */
void Psram_QuadModeExit(void);

/**
 * @brief  delay line level select,0~7 level
 *
 * @param[in]    delay_cfg 0~7 supported
 * @return		 none
 */
void Psram_DelayLine_Config(uint8_t delay_cfg);


/**
 * @brief  clock rate select, system_clock/2^(crat+1), 0~11 is supported
 *
 * @param[in]    crat 0~11 is supported
 * @return	     none
 */
void Psram_ClkRate_Config(uint8_t crat);


/**
 * @breif read len byte data from psram, use the memcpy funtion direcctly
 *
 * @param[in]   buffer   the buffer start address used to store the data read from psram
 * @param[in]   pos      the offset relative to the psram base address ,see the micro of the psram base definition
 * @param[in]   len      the len acculated by byte should to be read from psram
 * @return      none
 *
 */
#define Psram_Read(buffer,  len, pos)   memcpy (buffer, ( void* )( PSRAM_BASE + pos ), len )



/**
 * @breif write len bytes from buffer to  psram, use the memcpy funtion direcctly
 *
 * @param[in]   buffer   the buffer start address used to store the data write to psram
 * @param[in]   pos      the offset relative to the psram base address ,see the micro of the psram base definition
 * @param[in]   len      the len acculated by byte should to be written from psram
 * @return      none
 *
 */
#define Psram_Write(buffer, len, pos)  memcpy((void*)(PSRAM_BASE + pos), buffer, len)

/**
 * @brief  将psram初始化成SPIM使用
 *
 * @param[in]    none
 * @return   	 none
 */
void Psram_Spi_Init(void);

/**
 * @breif 以SPIM格式发送数据
 *
 * @param[in]   buf   发送数据buf，必须是RAM
 * @param[in]   len   发送数据长度
 * @return      none
 */
void Psram_Spi_Write_Data(uint8_t *buf, uint32_t len);

/**
 * @breif 以SPIM格式接收数据
 *
 * @param[in]   buf   接收数据buf，必须是RAM
 * @param[in]   len   接收数据长度
 * @return      none
 */
void Psram_Spi_Read_Data(uint8_t *buf, uint32_t len);

/**
 * @breif 使用DMA以SPIM格式发送数据
 *
 * @param[in]   buf   发送数据buf，必须是RAM
 * @param[in]   len   发送数据长度
 * @return      none
 */
void Psram_Spi_Write_Data_Dma(uint8_t *buf, uint32_t len);

/**
 * @breif 使用DMA以SPIM格式接收数据
 *
 * @param[in]   buf   接收数据buf，必须是RAM
 * @param[in]   len   接收数据长度
 * @return      none
 */
void Psram_Spi_Read_Data_Dma(uint8_t *buf, uint32_t len);

/**
 * @breif IsPsramSpiWriteDataDmaDone
 * @param[in]   none
 * @return      1-DMA Done
 */
uint32_t IsPsramSpiWriteDataDmaDone(void);

/**
 * @breif CleanPsramSpiWriteDataDmaFlag
 * @param[in]   none
 * @return      none
 */
void CleanPsramSpiWriteDataDmaFlag(void);

/**
 * @breif IsPsramSpiReadDataDmaDone
 * @param[in]   none
 * @return      1-DMA Done
 */
uint32_t IsPsramSpiReadDataDmaDone(void);

/**
 * @breif CleanPsramSpiReadDataDmaFlag
 * @param[in]   none
 * @return      none
 */
void CleanPsramSpiReadDataDmaFlag(void);

/**
 * @breif Psram_Spi_CPHA_CPOL_Init
 * @param[in]   CPHA
 * @param[in]   CPOL
 * @return      none
 */
void Psram_Spi_CPHA_CPOL_Init(uint8_t CPHA, uint8_t CPOL);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__PSRAM_H__

/**
 * @}
 * @}
 */

