/**
 *******************************************************************************
 * @file    backup.h
 * @brief	Backup module driver interface
 
 * @author  Sean
 * @version V1.0.0
 
 * $Created: 2016-12-26 13:25:07$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */
 
 /**
 * @addtogroup BACKUP
 * @{
 * @defgroup backup backup.h
 * @{
 */

#ifndef __BACKUP_H__
#define __BACKUP_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

typedef enum _CLK_32K_SEL
{
	RC32K_SOURCE = 0,
	OSC32K_SOURCE = 1
}CLK_32K_SEL;

typedef enum _CLK_32K_MODE
{
	CLK_LOSC_32K_MODE = 0,
	CLK_HOSC_xMHZ_DIV_MODE = 1
}CLK_32K_MODE;

typedef enum _OSC_DIRECT_INPUT_SEL
{
	 OSC_USE = 0,
	 EXTERNAL_DIRECT_INPUT_USE = 1
}OSC_DIRECT_INPUT_SEL;

typedef enum _POWERKEY_SWITCH_MODE
{
	HARD_MODE = 0,
	SOFT_MODE = 1
}POWERKEY_SWITCH_MODE;

typedef enum _HARD_TRIGGER_MODE
{
	LEVEL_TRIGGER = 0,
	EDGE_TRIGGER = 1
}HARD_TRIGGER_MODE;

typedef enum _POWERKEY_ACTIVE_LEVEL
{
	LOW_INDICATE_POWERON = 0,
	HIGH_INDICATE_POWERON = 1
}POWERKEY_ACTIVE_LEVEL;


/**
 * @brief	backup register write enable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_WriteEnable(void);

/**
 * @brief	backup register write disable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_WriteDisable(void);

/**
 * @brief	trigger system entering power down
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_SystemPowerDown(void);

/**
 * @brief	enable fast synchronous isolation when power fail
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_FastPowerDownEnable(void);

/**
 * @brief	disable fast synchronous isolation when power fail
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_FastPowerDownDisable(void);

/**
 * @brief	enable automatic power on from passive power off state when power ready
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_AutoPowerOnEnable(void);

/**
 * @brief	disable automatic power on from passive power off state when power ready
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_AutoPowerOnDisable(void);

/**
 * @brief	enable ldo12 off when powerdown
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_LDO12OffEnable(void);

/**
 * @brief	disable ldo12 off when powerdown
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_LDO12OffDisable(void);

/**
 * @brief	select RC32K or OSC32K
 * @param Clk32kSel :32K clk source
 *   @arg 0 :  RC32K
 *   @arg 1 :  OSC32K
 * @return	NONE
 * @note
 */
void BACKUP_32KSourceSelect(CLK_32K_SEL Clk32kSel);

/**
 * @brief	select between low speed osc clock and high speed osc clock
 * @param Clk32kMode :clock source
 *	 @arg 0 : select 32K clock(RC32K/OSC32K)
 *	 @arg 1 : select xMHz high frequency osc clock
 * @return	NONE
 * @note
 */
void BACKUP_32KModeSelect(CLK_32K_MODE Clk32kMode);

/**
 * @brief	Indicate osc clock is togglling, when this bit is toggling
 * @param	NONE
 * @return	this bit is toggling, TRUE: toggling.
 * @note
 */
bool BACKUP_IsOscClkToggle(void);

/**
 * @brief	enable RC32K or OSC32K
 * @param Clk32kSel :32K clk source
 *   @arg 0 :  RC32K
 *   @arg 1 :  OSC32K
 * @return	NONE
 * @note
 */
void BACKUP_32KEnable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	disable RC32K or OSC32K
 * @param Clk32kSel :32K clk source
 *   @arg 0 :  RC32K
 *   @arg 1 :  OSC32K
 * @return	NONE
 * @note
 */
void BACKUP_32KDisable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	get RC32K Enable state or OSC32K Enable state
 * @param Clk32kSel :32K clk source
 *   @arg 0 :  RC32K
 *   @arg 1 :  OSC32K
 * @return	32K clk state. TRUE:Enable;FALSE: Disable.
 * @note
 */
bool BACKUP_Is32KEnable(CLK_32K_SEL Clk32kSel);

/**
 * @brief	 choose if 32K clock is from oscillator or external direct input
 * @param	OscDircet
 *   @arg 0 : use oscillator
 *   @arg 1 : external direct input
 * @return	NONE
 * @note
 */
void BACKUP_OscDirectInputSelect(OSC_DIRECT_INPUT_SEL OscDircet);

/**
 * @brief	analog PVD enable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_PVDEnable(void);

/**
 * @brief	analog PVD disable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_PVDDisable(void);

/**
 * @brief	 config analog PVD threshold
 * @param	val: PVD val
 *    @arg  0:0.9v
 *    @arg 	1:0.83V
 *    @arg 	2:0.76V
 *    @arg 	3:0.69V
 *    @arg 	4:0.62V
 *    @arg 	5:0.55V
 *    @arg 	6:0.48V
 *    @arg 	7:0.41V
 * @return	NONE
 * @note
 */
void BACKUP_PVDVSelSet(uint8_t val);

/**
 * @brief	analog PVD threshold get
 * @param	NONE
 * @return
 *   @arg  	0:0.9v
 *   @arg  	1:0.83V
 *   @arg  	2:0.76V
 *   @arg  	3:0.69V
 *   @arg  	4:0.62V
 *   @arg  	5:0.55V
 *   @arg  	6:0.48V
 *   @arg  	7:0.41V
 * @note
 */
uint8_t BACKUP_PVDVSelGet(void);

/**
 * @brief	clear powerkey trigger state
 * @param   NONE
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyStateClear(void);

/**
 * @brief	set powerkey mode,hard mode or soft mode
 * @param	PowerKeyMode
 *	 @arg   0:powerkey hard mode
 *	 @arg   1:powerkey soft mode
 * @return  NONE
 * @note
 */
void BACKUP_PowerKeyModeSet(POWERKEY_SWITCH_MODE PowerKeyMode);

/**
 * @brief	get powerkey mode,hard mode or soft mode
 * @param	NONE
 * @return
 *	 @arg  0:powerkey hard mode
 *	 @arg  1:powerkey soft mode
 * @note
 */
POWERKEY_SWITCH_MODE BACKUP_PowerKeyModeGet(void);

/**
 * @brief	config level trigger or edge mode when in hard mode
 * @param	HardTriggerMode
 * 	    @arg   0:config level mode
 *	    @arg   1:config edge mode
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyHardModeSet(HARD_TRIGGER_MODE HardTriggerMode);

/**
 * @brief	get powerkey mode when in hard mode,level trigger or edge mode
 * @param	NONE
 * @return
 *  	@arg   0:config level mode
 *	    @arg   1:config edge mode*
 * @note
 */
HARD_TRIGGER_MODE BACKUP_PowerKeyHardModeGet(void);

/**
 * @brief	config active mode when in hard level mode
 * @param	PowerkeyActiveLevel
 *		@arg  0: powerkey high indicate power off powerkey low indicate power on
 *		@arg  1: powerkey high indicate power on powerkey low indicate power off
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyActiveLevelSet(POWERKEY_ACTIVE_LEVEL PowerkeyActiveLevel);

/**
 * @brief	get active mode when in hard level mode
 * @param	NONE
 * @return
 *		@arg  0:powerkey high indicate power off powerkey low indicate power on
 *		@arg  1:powerkey high indicate power onpowerkey low indicate power off
 * @note
 */
POWERKEY_ACTIVE_LEVEL BACKUP_PowerKeyActiveLevelGet(void);

/**
 * @brief	set the powerkey trigger time
 * @param	cnt define how long PAD_POWERKEY=1 lasts to power up the core. the time is equal to (256*s_trg_max_cnt+1)*(32k clock period)us.
 *              when okney_en open,s_trg_max_cnt should not lower than 1;
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyTrigMaxCntSet(uint8_t cnt);

/**
 * @brief	get the powerkey trigger time
 * @param	NONE
 * @return	cnt:define how long PAD_POWERKEY=1 lasts to power up the core. the time is equal to (256*s_trg_max_cnt+1)*(32k clock period)us.
 *              when okney_en open,s_trg_max_cnt should not lower than 1;
 * @note
 */
uint8_t BACKUP_PowerKeyTrigMaxCntGet(void);

/**
 * @brief	get powerkey trigger state
 * @param	NONE
 * @return	true:powerkey trig state is triggered
 *   		false:powerkey trig state is not triggered
 * @note
 */
bool BACKUP_PowerKeyTrigStateGet(void);

/**
 * @brief	get powerkey pin state
 * @param	NONE
 * @return	true:powerkey pin is high
 *   		false:powerkey state is low
 * @note
 */
bool BACKUP_PowerKeyPinStateGet(void);

/**
 * @brief	set powerkey enable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyEnable(void);

/**
 * @brief	set powerkey disable
 * @param	NONE
 * @return	NONE
 * @note
 */
void BACKUP_PowerKeyDisable(void);

/**
 * @brief	get powerkey status,enable disable
 * @param	NONE
 * @return	true:enabl;	false:disable
 * @note
 */
bool  BACKUP_IsPowerKeyEnable(void);

/**
 * @brief	get bool to determine if the backup pad was first powered on.
 * @param	NONE
 * @return	true:first time power on.
 *   		false:not the first time.
 * @note
 */
bool  BACKUP_FristPowerOnFlagGet(void);

/**
 * @brief	clean NVM mem when first power on
 * @param	none
 * @return  none
 * @note
 */
void BACKUP_NVMInit(void);

/**
 * @brief	read NVM data
 * @param  	NvmAddr 	the start address is 0,the end address is 49
 * @param 	Buf  		the buffer you read from the NVM
 * @param 	Length		it is form 1~50,NvmAddr + Length <= 50
 * @return
 *   		true:read success
 *   		false:read fail because the parameter has error
 * @note
 */
bool BACKUP_NvmRead(uint8_t NvmAddr, uint8_t* Buf, uint8_t Length);

/**
 * @brief	write NVM data
 * @param 	NvmAddr		the start address is 0,the end address is 49
 * @param 	Buf			the buffer you read from the NVM
 * @param 	Length		it is form 1~50,NvmAddr + Length <= 50
 * @return
 *   		true:write success
 *   		false:write fail because the parameter has error
 * @note
 */
bool BACKUP_NvmWrite(uint8_t NvmAddr, uint8_t* Buf, uint8_t Length);

/**
 * @brief	set reserve register one bit to 1
 * @param	BitMask register bit mask[bit0--bit7]
 * @return	none
 * @note bit0,bit1 is used by system
 */
void BACKUP_ResRegOneBitSet(uint8_t BitMask);

/**
 * @brief	clrea reserve register one bit to 0
 * @param	BitMask register bit mask[bit0--bit7]
 * @return	none
 * @note bit0,bit1 is used by system
 */
void BACKUP_ResRegOneBitClear(uint8_t BitMask);

/**
 * @brief	get reserve register value
 * @param	none
 * @return	register value
 * @note
 */
uint8_t BACKUP_ResRegGet(void);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__BACKUP_H__

/**
 * @}
 * @}
 */

