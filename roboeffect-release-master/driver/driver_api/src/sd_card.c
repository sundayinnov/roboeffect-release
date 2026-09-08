/**
 *****************************************************************************
 * @file     sdio_card1.c
 * @author   owen
 * @version  V1.0.0
 * @date     2017-05-15
 * @brief    sd card module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */
#include <string.h>
#include "type.h"
#include "debug.h"
#include "delay.h"
#include "timeout.h"
#include "sdio.h"
#include "sd_card.h"
#include "dma.h"
#include "core_d1088.h"
#include "gpio.h"
#ifdef FUNC_OS_EN
#include "rtos_api.h" //add for mutex declare

osMutexId SDIOMutex = NULL;
#endif

/**<传输状态 */
#define SD_STATUS_ACCEPTED				0x02
#define SD_STATUS_CRC_ERROR				0x05
#define SD_STATUS_WRITE_ERROR			0x06

/**<卡类型*/
#define	MC_MCDMAS_MMC					1
#define	MC_MCDMAS_SD					2

#define	SD_BLOCK_SIZE					512             /**<块大小固定为512字节*/

/**<卡状态定义*/
#define	CURRENT_STATE_IDLE				0
#define	CURRENT_STATE_READY				1
#define	CURRENT_STATE_IDENT				2              /**<identification*/
#define	CURRENT_STATE_STBY				3
#define	CURRENT_STATE_TRAN				4
#define	CURRENT_STATE_DATA				5              /**<sending data*/
#define	CURRENT_STATE_RCV				6
#define	CURRENT_STATE_PRG				7
#define	CURRENT_STATE_DIS				8              /**<disconnect*/

#define	VOLTAGE_WINDOWS					0x40FF8000     /**<2.7v~3.6v*/
#define VOLTAGE_PATTERN                 0x000001AA     /**<0x01:2.7-3.6V,0xAA:check pattern,pattern can use any 8-bit*/

#define CARD_DBG	printf


SD_CARD_FUN *SDIOApi = NULL;

static uint8_t SdioGroup = 0;
void CardPortInit(uint8_t SdioPort)
{
	SdioGroup = SdioPort;
	switch(SdioPort)
	{
		case SDIO0_A2_A3_A4:
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA2);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA3);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA4);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA2);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA3);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA4);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA2);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA4);
			GPIO_PortAModeSet(GPIOA2, SDIO0_DAT_A2_MODE);
			GPIO_PortAModeSet(GPIOA3, SDIO0_CLK_A3_MODE);
			GPIO_PortAModeSet(GPIOA4, SDIO0_CMD_A4_MODE);

			break;
		case SDIO1_A5_A6_A7:
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA5);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA6);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA7);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA5);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA6);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA7);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA5);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA7);
			GPIO_PortAModeSet(GPIOA5, SDIO1_DAT_A5_MODE);
			GPIO_PortAModeSet(GPIOA6, SDIO1_CLK_A6_MODE);
			GPIO_PortAModeSet(GPIOA7, SDIO1_CMD_A7_MODE);
			break;
		case SDIO0_A10_A11_A12:
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA10);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA11);
			GPIO_RegOneBitClear(GPIO_A_PD, GPIOA12);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA10);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA11);
			GPIO_RegOneBitSet(GPIO_A_PU, GPIOA12);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA10);
			GPIO_RegOneBitSet(GPIO_A_DS, GPIOA12);
			GPIO_PortAModeSet(GPIOA10, SDIO0_DAT_A10_MODE);
			GPIO_PortAModeSet(GPIOA11, SDIO0_CLK_A11_MODE);
			GPIO_PortAModeSet(GPIOA12, SDIO0_CMD_A12_MODE);
			break;
		default:
			break;
	}
}

void CardIdentified(uint8_t SdioPort)
{
	switch(SdioPort)
	{
		case SDIO0_A2_A3_A4:
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA2);
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA4);
				break;

		case SDIO1_A5_A6_A7:
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA5);
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA7);

			break;
		case SDIO0_A10_A11_A12:
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA10);
			GPIO_RegOneBitClear(GPIO_A_DS, GPIOA12);
			break;

		default:
			break;
	}
}
uint32_t SDCard_Heap[152/4];
//初始化SDIO控制器
void SDCard_ControllerInit(void)
{
	SDIOApi = (SD_CARD_FUN*)SDCard_Heap;
	memset(SDIOApi,0,sizeof(SD_CARD_FUN));
	CARD_DBG("memsize:%lu\n",sizeof(SD_CARD_FUN));
	//函数SDIO接口注册
#if (SDIO_CARD == SDIO0)
	SDIOApi->SDIO_ClkSet				= SDIO0_ClkSet;
	SDIOApi->SDIO_ClkEnable           	= SDIO0_ClkEnable;
	SDIOApi->SDIO_ClkDisable          	= SDIO0_ClkDisable;
	SDIOApi->SDIO_MultiRead_Enable    	= SDIO0_MultiRead_Enable;
	SDIOApi->SDIO_MultiRead_Disable   	= SDIO0_MultiRead_Disable;
	SDIOApi->SDIO_AutoKillTXClkEnable   = SDIO0_AutoKillTXClkEnable;
	SDIOApi->SDIO_AutoKillTXClkDisable  = SDIO0_AutoKillTXClkDisable;
	SDIOApi->SDIO_AutoKillRXClkEnable   = SDIO0_AutoKillRXClkEnable;
	SDIOApi->SDIO_AutoKillRXClkDisable  = SDIO0_AutoKillRXClkDisable;
	SDIOApi->SDIO_DataStartStatusGet  	= SDIO0_DataStartStatusGet;
	SDIOApi->SDIO_Init                	= SDIO0_Init;
	SDIOApi->SDIO_CmdSend       		= SDIO0_CmdSend;
	SDIOApi->SDIO_CmdRespGet      		= SDIO0_CmdRespGet;
	SDIOApi->SDIO_RXStart         		= SDIO0_RXStart;
	SDIOApi->SDIO_RXIsDone            	= SDIO0_RXIsDone;
	SDIOApi->SDIO_RXDoneFlagClear     	= SDIO0_RXDoneFlagClear;
	SDIOApi->SDIO_RXEnd               	= SDIO0_RXEnd;
	SDIOApi->SDIO_TXStart         		= SDIO0_TXStart;
	SDIOApi->SDIO_TXIsDone            	= SDIO0_TXIsDone;
	SDIOApi->SDIO_TXDoneFlagClear     	= SDIO0_TXDoneFlagClear;
	SDIOApi->SDIO_TXEnd               	= SDIO0_TXEnd;
	SDIOApi->SDIO_IsDataLineBusy      	= SDIO0_IsDataLineBusy;
	SDIOApi->SDIO_InterruptEnable     	= SDIO0_InterruptEnable;
	SDIOApi->SDIO_InterruptDisable    	= SDIO0_InterruptDisable;
	SDIOApi->SDIO_InterruptFlagGet    	= SDIO0_InterruptFlagGet;
	SDIOApi->SDIO_InterruptFlagClear  	= SDIO0_InterruptFlagClear;
	SDIOApi->SDIO_ByteMode_Enable		= SDIO0_ByteMode_Enable;
	SDIOApi->SDIO_ByteMode_Disable		= SDIO0_ByteMode_Disable;
	SDIOApi->SDIO_Config				= SDIO0_Config;
	SDIOApi->SDIO_Start					= SDIO0_Start;
#elif(SDIO_CARD == SDIO1)
	SDIOApi->SDIO_ClkSet				= SDIO1_ClkSet;
	SDIOApi->SDIO_ClkEnable           	= SDIO1_ClkEnable;
	SDIOApi->SDIO_ClkDisable          	= SDIO1_ClkDisable;
	SDIOApi->SDIO_MultiRead_Enable    	= SDIO1_MultiRead_Enable;
	SDIOApi->SDIO_MultiRead_Disable   	= SDIO1_MultiRead_Disable;
	SDIOApi->SDIO_AutoKillTXClkEnable  	= SDIO1_AutoKillTXClkEnable;
	SDIOApi->SDIO_AutoKillTXClkDisable	= SDIO1_AutoKillTXClkDisable;
	SDIOApi->SDIO_AutoKillRXClkEnable  	= SDIO1_AutoKillRXClkEnable;
	SDIOApi->SDIO_AutoKillRXClkDisable 	= SDIO1_AutoKillRXClkDisable;
	SDIOApi->SDIO_DataStartStatusGet  	= SDIO1_DataStartStatusGet;
	SDIOApi->SDIO_Init                	= SDIO1_Init;
	SDIOApi->SDIO_CmdSend       		= SDIO1_CmdSend;
	SDIOApi->SDIO_CmdRespGet      		= SDIO1_CmdRespGet;
	SDIOApi->SDIO_RXStart         		= SDIO1_RXStart;
	SDIOApi->SDIO_RXIsDone            	= SDIO1_RXIsDone;
	SDIOApi->SDIO_RXDoneFlagClear     	= SDIO1_RXDoneFlagClear;
	SDIOApi->SDIO_RXEnd               	= SDIO1_RXEnd;
	SDIOApi->SDIO_TXStart         		= SDIO1_TXStart;
	SDIOApi->SDIO_TXIsDone            	= SDIO1_TXIsDone;
	SDIOApi->SDIO_TXDoneFlagClear     	= SDIO1_TXDoneFlagClear;
	SDIOApi->SDIO_TXEnd               	= SDIO1_TXEnd;
	SDIOApi->SDIO_IsDataLineBusy      	= SDIO1_IsDataLineBusy;
	SDIOApi->SDIO_InterruptEnable     	= SDIO1_InterruptEnable;
	SDIOApi->SDIO_InterruptDisable    	= SDIO1_InterruptDisable;
	SDIOApi->SDIO_InterruptFlagGet    	= SDIO1_InterruptFlagGet;
	SDIOApi->SDIO_InterruptFlagClear  	= SDIO1_InterruptFlagClear;
	SDIOApi->SDIO_ByteMode_Enable		= SDIO1_ByteMode_Enable;
	SDIOApi->SDIO_ByteMode_Disable		= SDIO1_ByteMode_Disable;
	SDIOApi->SDIO_Config				= SDIO1_Config;
	SDIOApi->SDIO_Start					= SDIO1_Start;
#else
..........................
#endif


#ifdef FUNC_OS_EN
	if(SDIOMutex == NULL)
		SDIOMutex = osMutexCreate();
#endif
	SDIOApi->SDIO_ByteMode_Enable();//字节模式，和传输速率相关，务必注意，如果使用word模式，读扇区mem地址一定要4字节对齐。
	SDIOApi->SDIO_Config();
}

static SD_CARD_ERR_CODE SDCard_SendAppCommand(uint8_t Cmd, uint32_t Param)
{
	if(SDIOApi->SDIO_CmdSend(CMD55_APP_CMD, SDIOApi->SDCard.RCA, 20) == NO_ERR)
	{
		DelayUs(5);// > 1us between two cmd  by pi 2017.12.21
		if(SDIOApi->SDIO_CmdSend(Cmd, Param, 20) == NO_ERR)
		{
			return NONE_ERR;
		}
		return CMD_SEND_TIME_OUT_ERR;
	}
	return CMD55_SEND_ERR;
}

SD_CARD_ERR_CODE SDCard_Detect(void)
{
	if(SDIOApi->SDCard.IsCardInit)//卡已初始化
	{
		if(SDIOApi->SDIO_CmdSend(CMD13_SEND_STATUS, SDIOApi->SDCard.RCA, 20)  == NO_ERR)              	//read card status
		{
			return NONE_ERR;
		}
		else
		{
			SDCard_ControllerInit();
			return CMD13_SEND_ERR;
		}
	}
	else//卡未初始化
	{
		SDCard_ControllerInit();
		SDIOApi->SDIO_CmdSend(CMD0_GO_IDLE_STATE, 0, 20);
		SDIOApi->SDIO_CmdSend(CMD8_SEND_IF_COND, VOLTAGE_PATTERN, 20);
		if(SDCard_SendAppCommand(ACMD41_SD_SEND_OP_COND, VOLTAGE_WINDOWS) == NONE_ERR)
		{
			CARD_DBG("SD link!\n");
			SDIOApi->SDCard.CardType = MC_MCDMAS_SD;
			return NONE_ERR;
		}

		//check mmc card
		SDIOApi->SDIO_CmdSend(CMD0_GO_IDLE_STATE, 0, 20);
		SDIOApi->SDIO_CmdSend(CMD8_SEND_IF_COND, VOLTAGE_PATTERN, 20);
		if(SDIOApi->SDIO_CmdSend(CMD1_SEND_OP_COND, VOLTAGE_WINDOWS, 20) ==NO_ERR)
		{
			CARD_DBG("MMC link!\n");
			SDIOApi->SDCard.CardType = MC_MCDMAS_MMC;
			return NONE_ERR;
		}
		return NOCARD_LINK_ERR;
	}
}

SD_CARD_ERR_CODE SDCard_Identify(void)
{
	TIMER Timer;
	uint8_t  resp[16];
	uint8_t  i;

	//设置电压窗口
	TimeOutSet(&Timer, 2000);
	while(1)
	{
		if(SDIOApi->SDCard.CardType == MC_MCDMAS_SD)
		{
			if(SDCard_SendAppCommand(ACMD41_SD_SEND_OP_COND, VOLTAGE_WINDOWS))
			{
				return ACMD41_SEND_ERR;
			}
		}
		if(SDIOApi->SDCard.CardType == MC_MCDMAS_MMC)
		{
			if(SDIOApi->SDIO_CmdSend(CMD1_SEND_OP_COND, VOLTAGE_WINDOWS, 20))
			{
				return CMD1_SEND_ERR;
			}
		}
		SDIOApi->SDIO_CmdRespGet(resp, SDIO_RESP_TYPE_R3);

		if(*(uint32_t *)resp & 0x80000000)//ocr register bit definition, bit31-Card power up status bit (busy) bit30-Card Capacity Status (CCS)
		{
			if(*(uint32_t *)resp & 0x40000000)//for SDHC
			{
				SDIOApi->SDCard.IsSDHC = TRUE;
			}
			break;
		}

		if(IsTimeOut(&Timer))
		{
			CARD_DBG("CMD_SEND_TIME_OUT_ERR\n");
			return CMD_SEND_TIME_OUT_ERR;
		}
	}

	//获取CID(unique card identification)
	if(SDIOApi->SDIO_CmdSend(CMD2_ALL_SEND_CID, 0, 20))      //success card will from ready switch to identification
	{
		CARD_DBG("SdCardInit() CMD2 ERROR !\n");
		return CMD2_SEND_ERR;
	}
	SDIOApi->SDIO_CmdRespGet(resp, SDIO_RESP_TYPE_R2);
	for(i = 0; i < 15; i++)
	{
		*((uint8_t *)(&(SDIOApi->SDCardId)) + i + 1) = resp[14 - i];
	}

	//获取相对地址，最多重试10次
	for(i = 0; i < 10; i++)
	{
		if(SDIOApi->SDIO_CmdSend(CMD3_SEND_RELATIVE_ADDR, 0, 20) == NO_ERR)
		{
			SDIOApi->SDIO_CmdRespGet(resp, SDIO_RESP_TYPE_R6);
			if(((*(uint32_t *)(resp)) & 0x00001E00) >> 9 == CURRENT_STATE_STBY)
			{
				SDIOApi->SDCard.RCA = (*(uint32_t *)resp) & 0xFFFF0000;
				break;
			}
		}
	}

	CARD_DBG("RCA: %08lx\n", SDIOApi->SDCard.RCA);

	//获取CSD寄存器（Card Specific Data）里面的描述值，譬如，块长度，卡容量信息等。
	if(SDIOApi->SDIO_CmdSend(CMD9_SEND_CSD, SDIOApi->SDCard.RCA, 20))
	{
		return CMD9_SEND_ERR;
	}
	SDIOApi->SDIO_CmdRespGet(resp, SDIO_RESP_TYPE_R2);
	//根据CSD计算BlockNum
	if(resp[14])	//CSD V2
	{
		SDIOApi->SDCard.BlockNum = (((resp[7] & 0x3F) << 16) + (resp[6] << 8) + resp[5] + 1) << 10;	// memory capacity = (C_SIZE+1) * 512KByte
	}
	else			//CSD V1
	{
		uint8_t exp;
		SDIOApi->SDCard.BlockNum = ((resp[8] & 0x03) << 10) + (resp[7] << 2) + ((resp[6] & 0xC0) >> 6) + 1;

		exp = ((resp[5] & 0x03) << 1) + ((resp[4] & 0x80) >> 7) + 2 + (resp[9] & 0x0F) - 9;
		SDIOApi->SDCard.BlockNum <<= exp;	// (C_SIZE + 1) * 2 ^ (C_SIZE_MULT + 2)
	}
	CARD_DBG("BlockNum:%lu\n", SDIOApi->SDCard.BlockNum);
	//get card max transfer data speed
	i = (resp[11] >> 3) & 0x0F;

	CARD_DBG("resp:0X%X\n", resp[11]);
	if(i == 0 )
	{
		DBG("Csd error!\n");
	}
	if((resp[11] & 0x07) == 1)	//base 1M
	{
		if(i == 0x0F)//8M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x02;	//频率设为7.5MHz 30/4
		}
		else if(i > 0x08)//4.0M,4.5M,5.0M,5.5M,6.0M,7.0M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x03;	//频率设为3.75MHz 30/8
		}
		else if(i > 0x04)//2.0M,2.5M,3.0M,3.5M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x04;	//频率设为1.875MHz 30/16
		}
		else //1.0M,1.2M,1.3M,1.5M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x05;	//频率设为0.9375MHz 30/32
		}
	}
	else if((resp[11] & 0x07) == 2)	//base 10M
	{
		if(i < 0x04)//10M, 12M, 13M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x02;	//10M,频率设为7.5MHz  30/4
		}
		else if(i < 0x07)//15M,20M,25M
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x01;	//频率设为15MHz  30/2
		}
		else
		{
			SDIOApi->SDCard.MaxTransSpeed = 0x00;	//频率设为30MHz
		}
	}
	else
	{
		DBG("Csd max speed error!\n");
	}

	CARD_DBG("SDCard.MaxTransSpeed:%d\n", SDIOApi->SDCard.MaxTransSpeed);
	//进入transfer state
	if(SDIOApi->SDIO_CmdSend(CMD7_SELECT_DESELECT_CARD, SDIOApi->SDCard.RCA, 20))
	{
		return CMD7_SEND_ERR;
	}
	DelayUs(5);
	//获取状态
	if(SDIOApi->SDIO_CmdSend(CMD13_SEND_STATUS, SDIOApi->SDCard.RCA, 20))
	{
		return CMD13_SEND_ERR;
	}
	SDIOApi->SDIO_CmdRespGet(resp, SDIO_RESP_TYPE_R1);
	if(((*(uint32_t *)(resp)) & 0x00001E00) >> 9  != CURRENT_STATE_TRAN)
	{
		return GET_RESPONSE_STATUS_ERR;
	}

	//设置块长度
	if(SDIOApi->SDIO_CmdSend(CMD16_SET_BLOCKLEN, SD_BLOCK_SIZE, 20))
	{
		return CMD16_SEND_ERR;
	}

	return NONE_ERR;
}

SD_CARD_ERR_CODE SDCard_Init(void)
{
	uint8_t Retry = 4;
#ifdef FUNC_OS_EN
	osMutexLock(SDIOMutex);
#endif
	SDCard_ControllerInit();

	//根据以往SD卡兼容性经验，插入卡需延时一段时间
	WaitMs(150);
	while(1)
	{
		SDCard_Detect();
		if(SDCard_Identify() == NONE_ERR)
		{
			CARD_DBG("CardGetInfo OK\n");
			break;
		}
		if(!(--Retry))
		{
			CARD_DBG("retry 4 times was still failed\n");
#ifdef FUNC_OS_EN
			osMutexUnlock(SDIOMutex);
#endif
			return GET_SD_CARD_INFO_ERR;
		}
	}

	SDIOApi->SDCard.IsCardInit = TRUE;
	SDIOApi->SDIO_ClkSet(SDIOApi->SDCard.MaxTransSpeed);
	SDIOApi->SDIO_ClkEnable();
#ifdef FUNC_OS_EN
	osMutexUnlock(SDIOMutex);
#endif
	CardIdentified(SdioGroup);
	return NONE_ERR;
}


SD_CARD_ERR_CODE SDCard_ReadBlock(uint32_t Block, uint8_t* Buffer, uint8_t Size)
{
	TIMER Timer;

	if(Block > SDIOApi->SDCard.BlockNum)
	{
		return BLOCK_NUM_EXCEED_BOUNDARY;
	}

	if(!SDIOApi->SDCard.IsSDHC)
	{
		Block *= SD_BLOCK_SIZE;
	}
#ifdef FUNC_OS_EN
	osMutexLock(SDIOMutex);
#endif

	TimeOutSet(&Timer, 20 * Size);  // 20*Size
	if(Size == 1) //提高兼容性/稳定性 使用CMD17
	{
		DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_RX);
		DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
		DMA_BlockConfig(PERIPHERAL_ID_SDIO1_RX);
		DMA_BlockBufSet(PERIPHERAL_ID_SDIO1_RX, Buffer, SD_BLOCK_SIZE);
		DMA_ChannelEnable(PERIPHERAL_ID_SDIO1_RX);
		SDIOApi->SDIO_Start(SDIO_DIR_RX,SD_BLOCK_SIZE);
		SDIOApi->SDIO_CmdSend(CMD17_READ_SINGLE_BLOCK, Block, 20);
		while(1)
		{
			if(DMA_InterruptFlagGet(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT))
			{
				break;
			}
			if(IsTimeOut(&Timer))
			{
				DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_RX);
				DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
				SDIOApi->SDIO_RXEnd();
		#ifdef FUNC_OS_EN
				osMutexUnlock(SDIOMutex);
		#endif
				CARD_DBG("Card read one sector Timeout\n");
				return READ_SD_CARD_TIME_OUT_ERR;
			}
	#ifdef FUNC_OS_EN
			vTaskDelay(1);
	#endif
		}
		DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
		TimeOutSet(&Timer, 20);//快速插拔补丁，20MS 数值可缩减待测试验证
		while(!SDIOApi->SDIO_RXIsDone()&& !IsTimeOut(&Timer));
		SDIOApi->SDIO_RXEnd();
	}
	else
	{
		SDIOApi->SDIO_MultiRead_Enable();
		DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_RX);
		DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
		DMA_BlockConfig(PERIPHERAL_ID_SDIO1_RX);
		DMA_BlockBufSet(PERIPHERAL_ID_SDIO1_RX, Buffer, SD_BLOCK_SIZE*Size);
		DMA_ChannelEnable(PERIPHERAL_ID_SDIO1_RX);
		SDIOApi->SDIO_Start(SDIO_DIR_RX,SD_BLOCK_SIZE);
		SDIOApi->SDIO_CmdSend(CMD18_READ_MULTIPLE_BLOCK, Block, 20);
		while(1)
		{
			if(DMA_InterruptFlagGet(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT))
			{
				break;
			}

			if(IsTimeOut(&Timer))
			{
				DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_RX);
				SDIOApi->SDIO_MultiRead_Disable();
				DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
				SDIOApi->SDIO_RXEnd();
		#ifdef FUNC_OS_EN
				osMutexUnlock(SDIOMutex);
				SDIOApi->SDIO_CmdSend(CMD12_STOP_TRANSMISSION, 0, 20);
		#endif
				CARD_DBG("Card read timeout\n");
				return READ_SD_CARD_TIME_OUT_ERR;
			}
	#ifdef FUNC_OS_EN
			vTaskDelay(1);
	#endif
		}
		SDIOApi->SDIO_MultiRead_Disable();
		DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
		TimeOutSet(&Timer, 20);//快速插拔补丁，20MS 数值可缩减待测试验证
		while(!SDIOApi->SDIO_RXIsDone()&& !IsTimeOut(&Timer));
		SDIOApi->SDIO_RXEnd();
		SDIOApi->SDIO_CmdSend(CMD12_STOP_TRANSMISSION, 0, 20);
	}
	SDIOApi->SDIO_ClkDisable();
#ifdef FUNC_OS_EN
	osMutexUnlock(SDIOMutex);
#endif
	return NONE_ERR;
}


SD_CARD_ERR_CODE SDCard_WriteBlock(uint32_t Block, const uint8_t* Buffer, uint8_t Size)
{
	uint32_t i;
	TIMER Timer;
	SD_CARD_ERR_CODE ret = NONE_ERR;

	if(Block > SDIOApi->SDCard.BlockNum)
	{
		return BLOCK_NUM_EXCEED_BOUNDARY;
	}

	if(!SDIOApi->SDCard.IsSDHC)
	{
		Block *= SD_BLOCK_SIZE;
	}
#ifdef FUNC_OS_EN
	osMutexLock(SDIOMutex);
#endif
	SDIOApi->SDIO_ClkEnable();
	TimeOutSet(&Timer,20);
	while(SDIOApi->SDIO_IsDataLineBusy() && !IsTimeOut(&Timer));
	if(SDIOApi->SDIO_IsDataLineBusy())
	{
		ret = WRITE_SD_CARD_TIME_OUT_ERR;
	}
	else
	{
		if(Size == 1)//某张2G卡发CMD12后有概率超时。用CMD24提高兼容性
		{
			SDIOApi->SDIO_CmdSend(CMD24_WRITE_BLOCK, Block, 20);
			DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_TX);
			DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
			DMA_BlockConfig(PERIPHERAL_ID_SDIO1_TX);
			DMA_BlockBufSet(PERIPHERAL_ID_SDIO1_TX, (void*)Buffer, SD_BLOCK_SIZE);
			DMA_ChannelEnable(PERIPHERAL_ID_SDIO1_TX);
			SDIOApi->SDIO_Start(SDIO_DIR_TX,SD_BLOCK_SIZE);
			TimeOutSet(&Timer, 20);
			while(!SDIOApi->SDIO_TXIsDone()&& !IsTimeOut(&Timer));
			if(!SDIOApi->SDIO_TXIsDone())
			{
				ret = WRITE_SD_CARD_TIME_OUT_ERR;
				CARD_DBG("SD Write,tx error\n");
			}
			else
			{
				SDIOApi->SDIO_TXDoneFlagClear();
				TimeOutSet(&Timer,250);
				while(SDIOApi->SDIO_IsDataLineBusy() && !IsTimeOut(&Timer));
				if(SDIOApi->SDIO_IsDataLineBusy())
				{
					CARD_DBG("SD write addr:0x%x error\n", (unsigned int)Block);
					ret = WRITE_SD_CARD_TIME_OUT_ERR;
				}
			}
		}
		else
		{
			SDIOApi->SDIO_CmdSend(CMD25_WRITE_MULTIPLE_BLOCK, Block, 20);
			for(i = 0; i < Size; i++)
			{
				DMA_ChannelDisable(PERIPHERAL_ID_SDIO1_TX);
				DMA_InterruptFlagClear(PERIPHERAL_ID_SDIO1_RX, DMA_DONE_INT);
				DMA_BlockConfig(PERIPHERAL_ID_SDIO1_TX);
				DMA_BlockBufSet(PERIPHERAL_ID_SDIO1_TX, (void*)(Buffer+512*i), SD_BLOCK_SIZE);
				DMA_ChannelEnable(PERIPHERAL_ID_SDIO1_TX);
				SDIOApi->SDIO_Start(SDIO_DIR_TX,SD_BLOCK_SIZE);
				TimeOutSet(&Timer, 20);//保护性，硬件发送和clk相关
				while(!SDIOApi->SDIO_TXIsDone()&& !IsTimeOut(&Timer));
				if(!SDIOApi->SDIO_TXIsDone())
				{
					ret = WRITE_SD_CARD_TIME_OUT_ERR;
					break;
				}
				SDIOApi->SDIO_TXDoneFlagClear();
				if(i == (Size - 1))//最后扇区
				{
					SDIOApi->SDIO_CmdSend(CMD12_STOP_TRANSMISSION, 0, 20);
					SDIOApi->SDIO_ClkEnable();//某2G卡不加此句容易写超时
					TimeOutSet(&Timer,500);//250, but for SDXC 500
					while(SDIOApi->SDIO_IsDataLineBusy() && !IsTimeOut(&Timer));
				}
				else
				{
					TimeOutSet(&Timer,250);
					while(SDIOApi->SDIO_IsDataLineBusy() && !IsTimeOut(&Timer));
				}

				if(SDIOApi->SDIO_IsDataLineBusy())
				{
					CARD_DBG("SD %d/%d Write Addr:0x%X error\n", (int)i + 1, (int)Size, (unsigned int)Block);
					ret = WRITE_SD_CARD_TIME_OUT_ERR;
					break;
				}
			}
		}
		SDIOApi->SDIO_TXEnd();
		SDIOApi->SDIO_ClkDisable();
	}
#ifdef FUNC_OS_EN
	osMutexUnlock(SDIOMutex);
#endif
	return ret;
}

SD_CARD* SDCard_GetCardInfo()
{
	return &SDIOApi->SDCard;
}


uint32_t SDCard_CapacityGet(void)
{
	return SDIOApi->SDCard.BlockNum;
}
