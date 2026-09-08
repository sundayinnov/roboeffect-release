/**
 *****************************************************************************
 * @file     sd_card.h
 * @author   owen
 * @version  V1.0.0
 * @date     2017-05-15
 * @brief    sd card module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __CARD_H__
#define __CARD_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "sdio.h"

#define	SD_BLOCK_SIZE					512 

#define SDIO0	0
#define SDIO1	1
#define SDIO_CARD	SDIO1

//Use SDIO0 must set DMA Channel
//PERIPHERAL_ID_SDIO0_RX,
//PERIPHERAL_ID_SDIO0_TX,
//Use SDIO1 must set DMA Channel
//PERIPHERAL_ID_SDIO1_RX,
//PERIPHERAL_ID_SDIO1_TX,

#define SDIO0_DAT_A2_MODE				1
#define SDIO0_CLK_A3_MODE				2
#define SDIO0_CMD_A4_MODE				2
#define SDIO1_DAT_A5_MODE				2
#define SDIO1_CLK_A6_MODE				2
#define SDIO1_CMD_A7_MODE				1
#define SDIO0_DAT_A10_MODE				1
#define SDIO0_CLK_A11_MODE				2
#define SDIO0_CMD_A12_MODE				1

#define SDIO0_A2_A3_A4					0
#define SDIO1_A5_A6_A7					1
#define SDIO0_A10_A11_A12				2


typedef void (*sdio_ClkSet) (uint8_t);
typedef void (*sdio_ClkEnable)(void);
typedef void (*sdio_ClkDisable)(void);
typedef void (*sdio_MultiRead_Enable)(void);
typedef void (*sdio_MultiRead_Disable)(void);
typedef void (*sdio_AutoKillTXClkEnable)(void);
typedef void (*sdio_AutoKillTXClkDisable)(void);
typedef void (*sdio_AutoKillRXClkEnable)(void);
typedef void (*sdio_AutoKillRXClkDisable)(void);
typedef bool (*sdio_DataStartStatusGet)(void);
typedef bool (*sdio_Init)(void);
typedef bool (*sdio_CmdSend)(uint8_t, uint32_t, uint16_t);
typedef bool (*sdio_CmdRespGet)(uint8_t* RespBuf, uint8_t RespType);
typedef bool (*sdio_RXStart)(uint8_t *Buf,uint32_t Size);
typedef bool (*sdio_RXIsDone)(void);
typedef void (*sdio_RXDoneFlagClear)(void);
typedef void (*sdio_RXEnd)(void);
typedef bool (*sdio_TXStart)(uint8_t *Buf,uint32_t Size);
typedef bool (*sdio_TXIsDone)(void);
typedef void (*sdio_TXDoneFlagClear)(void);
typedef void (*sdio_TXEnd)(void);
typedef bool (*sdio_IsDataLineBusy)();
typedef void (*sdio_InterruptEnable)(SDIO_INT_TYPE IntType);
typedef void (*sdio_InterruptDisable)(SDIO_INT_TYPE IntType);
typedef bool (*sdio_InterruptFlagGet)(SDIO_INT_TYPE IntType);
typedef void (*sdio_InterruptFlagClear)(SDIO_INT_TYPE IntType);
typedef void (*sdio_Config)(void);
typedef void (*sdio_Start)(SDIO_DIR Direction,uint32_t Size);
typedef void (*sdio_ByteMode_Enable)(void);
typedef void (*sdio_ByteMode_Disable)(void);


/**
 * SD CARD attribute definition
 */
typedef struct
{
	uint8_t		CardType;
	uint8_t   	MaxTransSpeed;
	bool		IsCardInit;
	bool		IsSDHC;
	uint32_t	BlockNum;
	uint32_t	RCA;//高16位为RCA，在使用RCA时低16位可为任意字符，此处将resp中R6的低16位一起保存了（同O18B一致，但并无必要）
} SD_CARD;

/**
 * SD CARD CID definition
 */
typedef struct _SD_CARD_ID
{
	uint8_t		MID;			/**< Manufacturer ID CID[127:120]*/
	uint8_t		OID[2];			/**< OEM/Application ID CID[119:104]*/
	uint8_t		PNM[5];			/**< Product name CID[103:64]*/
	uint8_t		PRV;			/**< Product revision CID[63:56]*/
	uint8_t		PSN[4];			/**< Product serial number CID[55:24]*/
	uint16_t	Rsv : 4;		/**< reserved CID[23:20]*/
	uint16_t	MDT : 12;		/**< Manufacturing date CID[19:8]*/
	uint8_t		CRC : 7;		/**< CRC7 checksum CID[7:1]*/
	uint8_t	NoUse : 1;		/**< not used, always 1  CID[0:0]*/
} SD_CARD_ID;


typedef struct
{
	sdio_ClkSet SDIO_ClkSet;
	sdio_ClkEnable SDIO_ClkEnable;
	sdio_ClkDisable SDIO_ClkDisable;
	sdio_MultiRead_Enable SDIO_MultiRead_Enable;
	sdio_MultiRead_Disable SDIO_MultiRead_Disable;
	sdio_DataStartStatusGet SDIO_DataStartStatusGet;
	sdio_AutoKillTXClkEnable SDIO_AutoKillTXClkEnable;
	sdio_AutoKillTXClkDisable SDIO_AutoKillTXClkDisable;
	sdio_AutoKillRXClkEnable SDIO_AutoKillRXClkEnable;
	sdio_AutoKillRXClkDisable SDIO_AutoKillRXClkDisable;
	sdio_Init SDIO_Init;
	sdio_CmdSend SDIO_CmdSend;
	sdio_CmdRespGet SDIO_CmdRespGet;
	sdio_RXStart SDIO_RXStart;
	sdio_RXIsDone SDIO_RXIsDone;
	sdio_RXDoneFlagClear SDIO_RXDoneFlagClear;
	sdio_RXEnd SDIO_RXEnd;
	sdio_TXStart SDIO_TXStart;
	sdio_TXIsDone SDIO_TXIsDone;
	sdio_TXDoneFlagClear SDIO_TXDoneFlagClear;
	sdio_TXEnd SDIO_TXEnd;
	sdio_IsDataLineBusy SDIO_IsDataLineBusy;
	sdio_InterruptEnable SDIO_InterruptEnable;
	sdio_InterruptDisable SDIO_InterruptDisable;
	sdio_InterruptFlagGet SDIO_InterruptFlagGet;
	sdio_InterruptFlagClear SDIO_InterruptFlagClear;
	sdio_Config SDIO_Config;
	sdio_Start SDIO_Start;
	sdio_ByteMode_Enable SDIO_ByteMode_Enable;
	sdio_ByteMode_Disable SDIO_ByteMode_Disable;
	SD_CARD		    SDCard;
	SD_CARD_ID	    SDCardId;
} SD_CARD_FUN;
/**
 * err code definition
 */
typedef enum _SD_CARD_ERR_CODE
{
    CMD_SEND_TIME_OUT_ERR = -255,   /**<cmd send time out*/
    GET_RESPONSE_STATUS_ERR,        /**<get surrent transfer status err*/
    READ_SD_CARD_TIME_OUT_ERR,      /**<sd card read time out*/
    WRITE_SD_CARD_TIME_OUT_ERR,     /**<sd card write time out*/
    SD_CARD_IS_BUSY_TIME_OUT_ERR,   /**<sd card is busy time out*/
    NOCARD_LINK_ERR,                /**<sd card link err*/
    ACMD41_SEND_ERR,                /**<send ACMD41 err*/
    CMD1_SEND_ERR,                  /**<send CMD1 err*/
    CMD2_SEND_ERR,                  /**<get CID err*/
    ACMD6_SEND_ERR,                 /**<set bus witdh err*/
    CMD7_SEND_ERR,                  /**<select and deselect card err*/
    CMD9_SEND_ERR,                  /**<get CSD err*/
    CMD12_SEND_ERR,                 /**<stop cmd send err*/
    CMD13_SEND_ERR,                 /**<CMD13 read card status err*/
    CMD16_SEND_ERR,                 /**<set block length err*/
    CMD18_SEND_ERR,                 /**<CMD18 send err*/
    CMD25_SEND_ERR,                 /**<multi block write cmd send err*/
    CMD55_SEND_ERR,                 /**<send cmd55 err*/
    GET_SD_CARD_INFO_ERR,           /**<get sd card info err*/
    BLOCK_NUM_EXCEED_BOUNDARY,      /**<read block exceed boundary err*/
    NONE_ERR = 0,
} SD_CARD_ERR_CODE;


/**
 * @brief  sdio的IO初始化
 * @param  SDIO0_A2_A3_A4 , SDIO1_A5_A6_A7 , SDIO0_A10_A11_A12
 * @return NONE
 * @note
 */
void CardPortInit(uint8_t SdioPort);

/**
 * @brief  SD卡检测
 * @param  NONE
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_Detect(void);

/**
 * @brief  SD卡信息获取
 * @param  NONE
 * @return SD卡信息
 * @note
 */
SD_CARD* SDCard_GetCardInfo(void);

/**
 * @brief  SD卡初始化
 * @param  NONE
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_Init(void);

/**
 * @brief  SD卡读扇区
 * @param  Block 扇区号
 * @param  Buffer 读缓存区指针
 * @param  Size 扇区个数
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_ReadBlock(uint32_t Block, uint8_t* Buffer, uint8_t Size);

/**
 * @brief  SD卡写扇区
 * @param  Block 扇区号
 * @param  Buffer 写缓存区指针
 * @param  Size 扇区个数
 * @return SD_CARD_ERR_CODE
 * @note
 */
SD_CARD_ERR_CODE SDCard_WriteBlock(uint32_t Block, const uint8_t* Buffer, uint8_t Size);

/**
 * @brief  SD卡容量获取
 * @param  NONE
 * @return SD卡容量
 * @note
 */
uint32_t SDCard_CapacityGet(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__CARD_H__
