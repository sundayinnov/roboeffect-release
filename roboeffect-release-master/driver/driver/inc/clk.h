/**
 *******************************************************************************
 * @file    clk.h
 * @brief	Clock driver interface
 *
 * @author  Sam
 * @version V1.0.0
 *
 * $Created: 2015-11-05 10:46:11$
 *
 * @Copyright (C) 2015, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */
 
 
/**
 * @addtogroup CLOCK
 * @{
 * @defgroup clk clk.h
 * @{
 */
 
#ifndef __CLK_H__
#define __CLK_H__
 
#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

/**
 * CLK module switch macro define
 */
typedef enum __CLOCK_MODULE1_SWITCH
{
	PLL_CLK1_EN = (1 << 0),				/**<PLL1 module clk switch */
	PLL_CLK2_EN = (1 << 1),				/**<PLL2 module clk switch */
	PLL_CLK3_EN = (1 << 2),				/**<PLL3 module clk switch */
	PLL_CLK4_EN = (1 << 3),  			/**<PLL4 module clk switch */
	PLL_CLK5_EN = (1 << 4),            	/**<PLL5 module clk switch */
	MIC_Digital_En = (1 << 5),			/**<MIC Digital module clk switch */
	AUDIO_DAC0_CLK_EN = (1 << 6),       /**<Audio DAC port0 module clk switch */
	AUDIO_DAC1_CLK_EN = (1 << 7),		/**<Audio DAC port1 module clk switch */
	AUDIO_ADC0_CLK_EN = (1 << 8),       /**<Audio ADC port0 module clk switch */
	AUDIO_ADC1_CLK_EN = (1 << 9),		/**<Audio ADC port1 module clk switch */
	I2S0_CLK_EN = (1 << 10),			/**<I2S port0 module clk switch */
	I2S1_CLK_EN = (1 << 11),			/**<I2S port1 module clk switch */
	FLASH_CONTROL_CLK_EN = (1 << 12),	/**<Flash control module clk switch */
	USB_CLK_EN = (1 << 13),				/**<USB module clk switch */
	UART0_CLK_EN = (1 << 14),           /**<UART0 module clk switch */
	UART1_CLK_EN = (1 << 15),           /**<UART1 module clk switch */

	PSRAM_CONTROL_CLK_EN = (1 << 17),   /**<PSRAM control module clk switch */
	SPDIF_CLK_EN = (1 << 18),           /**<SPDIF module clk switch */
	FFT_CLK_EN = (1 << 19),            	/**<FFT module clk switch */
	EFUSE_CLK_EN = (1 << 20),          	/**<EFUSE module clk switch */
	I2C_CLK_EN = (1 << 21),          	/**<I2C module clk switch */
	ADC_CLK_EN = (1 << 22),          	/**<ADC module clk switch */
	SPIM_CLK_EN = (1 << 23),          	/**<SPI master module clk switch */
	SDIO0_CLK_EN = (1 << 24),          	/**<SDIO port0 module clk switch */
	SDIO1_CLK_EN = (1 << 25),          	/**<SDIO port1 module clk switch */
	USI_CLK_EN = (1 << 26),          	/**<USI(7816) module clk switch */

	ALL_MODULE1_CLK_SWITCH = (0xFFFFFFFF),/**<all module clk SWITCH*/
} CLOCK_MODULE1_SWITCH;

typedef enum __CLOCK_MODULE2_SWITCH
{
	TIMER1_CLK_EN = (1 << 0),			/**<TIMER1 module clk switch */
	TIMER2_CLK_EN = (1 << 1),			/**<TIMER2 module clk switch */
	TIMER3_CLK_EN = (1 << 2),			/**<TIMER3 module clk switch */
	TIMER4_CLK_EN = (1 << 3),			/**<TIMER4 module clk switch */
	TIMER5_CLK_EN = (1 << 4),			/**<TIMER5 module clk switch */
	TIMER6_CLK_EN = (1 << 5),			/**<TIMER6 module clk switch */
	ALL_MODULE2_CLK_SWITCH = (0x3F),		/**<all module clk SWITCH*/
} CLOCK_MODULE2_SWITCH;

typedef enum __CLK_MODE{
	RC_CLK_MODE,
	PLL_CLK_MODE,
	SYSTEM_CLK_MODE /*Fpll / 4 / SYS DIV NUM or RC / 2 / DIV NUM*/
}CLK_MODE;

typedef enum __PLL_CLK_INDEX{
	PLL_CLK_1 = 0,	/**PLL分频时钟器1,硬件自动生成为11.2896M，软件可以修改*/
	PLL_CLK_2		/**PLL分频时钟器2,硬件自动生成为12.288M，软件可以修改*/
}PLL_CLK_INDEX;

typedef enum __PLL_CLK_DIV_MODE{
	PLL_CLK_GEN = 0,	/**PLL分频时钟，硬件自动分频器*/
	PLL_CLK_USER_DEF	/**PLL分频时钟，客户软件自定义*/
}PLL_CLK_DIV_MODE;

typedef enum __AUDIO_MODULE{
	AUDIO_DAC0,
	AUDIO_DAC1,
	AUDIO_ADC0,
	AUDIO_ADC1,
	AUDIO_I2S0,
	AUDIO_I2S1
}AUDIO_MODULE;


typedef enum __MCLK_CLK_SEL{
	PLL_CLOCK1 = 0,	/**PLL分频1时钟，硬件Gen生成时钟频率为11.2896M，或者软件自定义*/
	PLL_CLOCK2,		/**PLL分频2时钟，硬件Gen生成时钟频率为12.288M，或者软件自定义*/
	OSC_IN,			/**晶体时钟直接作为音频主时钟输入*/
	GPIO_IN0,		/**MCLK0输入，具体对照GPIO复用关系*/
	GPIO_IN1		/**MLCK1输入，具体对照GPIO复用关系*/
}MCLK_CLK_SEL;

/**
 * @brief	系统参考时钟源配置选择
 * @param	IsOsc TURE：晶体时钟；FALSE：XIN端口外灌方波时钟
 * @param   Freq  系统参考时钟的工作频率：32.768K,1M,2M,...40M,单位HZ
 * @return	无
 * @note	如果使用PLL时钟则必须要先调用该函数
 */
void Clock_Config(bool IsOsc, uint32_t Freq);

/**
 * @brief	获取系统工作的系统时钟频率
 * @param	无
 * @return	系统工作频率
 */
uint32_t Clock_SysClockFreqGet(void);

/**
 * @brief	获取内核工作的系统时钟频率（为系统总线时钟的2倍）
 * @param	无
 * @return	系统工作频率
 */
uint32_t Clock_CoreClockFreqGet(void);
/**
 * @brief  设置系统时钟分频系数
 * @param  DivVal [0-256]，0关闭分频功能, 1不分频
 * @return 无
 */
void Clock_SysClkDivSet(uint32_t DivVal);
    
/**
 * @brief  获取系统分频系数
 * @param  NONE
 * @return  系统分频系数[1-256]
 */
uint32_t Clock_SysClkDivGet(void);

/**
 * @brief  设置APB总线分频系数，基于系统总线分频
 * @param  DivVal [2-16], the divided frequency is main_frequency/(N+1)；default is 8
 * @Note: 1. N must is odd, apb clock frequency must not high 20Mhz；
 *        2. The value must not configured higer than 14 when sys_clk is 120M
 * @return 无
 */
void Clock_ApbClkDivSet(uint32_t DivVal);

/**
 * @brief  获取APB总线分频系数，基于系统总线分频
 * @param  无
 * @return  APB总线分频系数[2-12]
 */
uint32_t Clock_ApbClkDivGet(void);

/**
 * @brief	SarADC模块时钟分频比配置，即时钟频率选择
 * @param	DivNum 分频比选择[2-15]
 * @return  无
 */
void Clock_SarADCClkDivSet(uint32_t DivVal);

/**
 * @brief	获取SarADC模块时钟分频比
 * @param	无
 * @return  DivNum 分频比选择[2-15]
 */
uint32_t Clock_SarADCClkDivGet(void);

/**
 * @brief  设置PLL3分频系数，flash HPM使用
 * @param  DivVal 分频系数[2-14]
 * @return  无
 */
void Clock_Pll3ClkDivSet(uint32_t DivVal);

/**
 * @brief  获取PLL3分频系数，flash HPM使用
 * @param  无
 * @return  分频系数[2-14]
 */
uint32_t Clock_Pll3ClkDivGet(void);

/**
 * @brief  设置PLL5分频系数，USB/UART使用
 * @param  DivVal 分频系数[2-14]
 * @return  无
 */
void Clock_Pll5ClkDivSet(uint32_t DivVal);

/**
 * @brief  获取PLL5分频系数，USB/UART使用
 * @param  无
 * @return  分频系数[2-14]
 */
uint32_t Clock_Pll5ClkDivGet(void);

/**
 * @brief  设置SPI master工作时钟分频系数，从系统时钟分频
 * @param  DivVal 分频系数[2-14]
 * @return  无
 */
void Clock_SPIMClkDivSet(uint32_t DivVal);

/**
 * @brief  获取SPI master工作时钟分频系数，从系统时钟分频
 * @param  无
 * @return  分频系数[2-14]
 */
uint32_t Clock_SPIMClkDivGet(void);

/**
 * @brief  设置SDIO0工作时钟分频系数，从系统时钟分频
 * @param  DivVal 分频系数[2-14]
 * @return  无
 */
void Clock_SDIO0ClkDivSet(uint32_t DivVal);

/**
 * @brief  获取SDIO0工作时钟分频系数，从系统时钟分频
 * @param  无
 * @return  分频系数[2-14]
 */
uint32_t Clock_SDIO0ClkDivGet(void);

/**
 * @brief  设置SDIO1工作时钟分频系数，从系统时钟分频
 * @param  DivVal 分频系数[2-14]
 * @return  无
 */
void Clock_SDIO1ClkDivSet(uint32_t DivVal);

/**
 * @brief  获取SDIO1工作时钟分频系数，从系统时钟分频
 * @param  无
 * @return  分频系数[2-14]
 */
uint32_t Clock_SDIO1ClkDivGet(void);

/**
 * @brief  设置RC32K分频系数，基于RC12M分频，该时钟主要给WDG使用
 * @param  DivVal 分频系数[2-512]
 * @return  无
 */
void Clock_RC32KClkDivSet(uint32_t DivVal);

/**
 * @brief  获取RC32K分频系数，基于RC12M分频，该时钟主要给WDG使用
 * @param  无
 * @return  分频系数[2-512]
 */
uint32_t Clock_RC32KClkDivGet(void);

/**
 * @brief	获取rc频率
 * @param	IsReCount 是否再次获取硬件计数器值。TRUE：再次启动硬件计数器。FALSE：获取上次记录值。
 * @return  rc频率，单位Hz
 */	
uint32_t Clock_RcFreqGet(bool IsReCount);

/**
 * @brief	获取pll频率
 * @param	无
 * @return  pll频率
 * @note    该函数调用需要应用确保pll已经lock，否则读数错误
 */
uint32_t Clock_PllFreqGet(void);

/**
 * @brief	设置OSC分频系数，分频之后的时钟提供给RTC使用
 * @param	DivVal 分频系数[1-2048]。
 * @return  无
 */
void Clock_OSCClkDivSet(uint32_t DivVal);

/**
 * @brief	获取OSC分频系数，分频之后的时钟提供给RTC使用
 * @param	无
 * @return  OSC分频系数[1-2048]
 */
uint32_t Clock_OSCClkDivGet(void);

/**
 * @brief	选OSC输出是否经过分频系数
 * @param	IsDivSel，TRUE：经过分频系数；FALSE：直接输出
 * @return  无
 * @note	该晶体时钟输出，会给到RTC使用，并且可以输出的GPIO上
 *			HSOC时会一般会分频到32.768Hz，LSOC是直接输出
 */
void Clock_OSCClkDivSelect(bool IsDivSel);

/**
 * @brief	设置pll工作频率,等待pll lock
 * @param	PllFreq pll频率,单位KHz[240000K-480000K]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
bool Clock_PllLock(uint32_t PllFreq);

/**
 * @brief	设置pll工作频率,快速锁定模式，等待pll lock
 * @param	PllFreq pll频率,单位KHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @param	Slope [0-16777216]
 * @return  PLL锁定情况  TRUE:按设定目标频率锁定
 */
bool Clock_PllQuicklock(uint32_t PllFreq, uint8_t K1, uint8_t OS, uint32_t NDAC, uint32_t FC, uint32_t Slope);

/**
 * @brief	设置pll工作频率,开环锁定模式(不需要晶体，无自校准)
 * @param	PllFreq pll频率,单位KHz[240000K-480000K]
 * @param	K1 [0-15]
 * @param	OS [0-31]
 * @param	NDAC [0-4095]
 * @param	FC [0-2]
 * @return  无
 */
void Clock_PllFreeRun(uint32_t PllFreq, uint32_t K1, uint32_t OS, uint32_t NDAC, uint32_t FC);

/**
 * @brief	pll free run 一般只需要调此函数即可
 * @param	无
 * @return  无
 */
bool Clock_PllFreeRunEfuse(void);

/**
 * @brief	pll模块关闭
 * @param	无
 * @return  无
 */
void Clock_PllClose(void);

/**
 * @brief	选择系统工作时钟
 * @param	ClkMode, RC_CLK_MODE: RC12M时钟; PLL_CLK_MODE:pll时钟;
 * @return  是否成功切换系统时钟，TRUE：工程切换；FALSE：切换时钟失败。
 * @note    系统时钟工作于pll时钟时，为pll时钟的4分频
 */
bool Clock_SysClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Uart模块时钟选择,pll时钟还是RC时钟
 * @param	ClkMode RC_CLK_MODE: RC时钟; PLL_CLK_MODE:pll时钟（PLL分频之后）
 * @return  无
 */
void Clock_UARTClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Timer3模块时钟选择,系统时钟时钟还是RC时钟
 * @param	ClkMode SYSTEM_CLK_MODE: 系统时钟; RC_CLK_MODE:RC 12M时钟
 * @return  无
 */
void Clock_Timer3ClkSelect(CLK_MODE ClkMode);

/**
 * @brief	Timer4模块时钟选择,系统时钟时钟还是RC时钟
 * @param	ClkMode SYSTEM_CLK_MODE: 系统时钟; RC_CLK_MODE:RC 12M时钟
 * @return  无
 */
void Clock_Timer4ClkSelect(CLK_MODE ClkMode);

/**
 * @brief	模块时钟使能
 * @param	ClkSel 模块源，根据CLOCK_MODULE1_SWITCH选择
 * @return  无
 */
void Clock_Module1Enable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	模块时钟禁能
 * @param	ClkSel 模块源，根据CLOCK_MODULE1_SWITCH选择
 * @return  无
 */
void Clock_Module1Disable(CLOCK_MODULE1_SWITCH ClkSel);

/**
 * @brief	模块时钟使能
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_Module2Enable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	模块时钟禁能
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_Module2Disable(CLOCK_MODULE2_SWITCH ClkSel);

/**
 * @brief	高频晶体时钟模块进入关闭，进入低功耗
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_HOSCDisable(void);

/**
 * @brief	高频晶体时钟模块使能
 * @param	无
 * @return  无
 */
void Clock_HOSCEnable(void);

/**
 * @brief	高频晶体时钟模块关闭，进入低功耗
 * @param	无
 * @return  无
 */
void Clock_HOSCDisable(void);

/**
 * @brief	低频晶体时钟模块打开
 * @param	无
 * @return  无
 */
void Clock_LOSCEnable(void);

/**
 * @brief	低频晶体时钟模块关闭
 * @param	无
 * @return  无
 */
void Clock_LOSCDisable(void);

/**
 * @brief	高频晶体时钟模块进入关闭，进入低功耗
 * @param	ClkSel 模块源，根据CLOCK_MODULE2_SWITCH选择
 * @return  无
 */
void Clock_HOSCDisable(void);

/**
 * @brief	音频PLL时钟生成
 * @param	Index PLL源，PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	Mode PLL生成目标频率的方式，PLL_CLK_GEN:硬件生成;PLL_CLK_USER_DEF:软件配置分频比（DivVal）;
 * @param	DivVal PLL分频比，当Mode=PLL_CLK_USER_DEF时有效。8bit整数，8bit小数。
 * @return  无
 */
void Clock_AudioPllClockSet(PLL_CLK_INDEX Index, PLL_CLK_DIV_MODE Mode, uint16_t DivVal);


//音频时钟设置，PLL1 = 11.2986M；PLL2 = 12.288M
//该API会根据PLL锁定频率自动计算
void Clock_AudioPllClockAutoSet(PLL_CLK_INDEX Index);

/**
 * @brief	音频模块主时钟源选择
 * @param	Module 音频模块
 * @param	ClkSel 时钟来源类型选择
 * @return  无
 */
void Clock_AudioMclkSel(AUDIO_MODULE Module, MCLK_CLK_SEL ClkSel);

/**
 * @brief	音频时钟源采样率微调，PLL1和PLL2
 * @param	Index PLL源，PLL_CLK_1:11.2896M;PLL_CLK_2:12.288M;
 * @param	Sign  0：调慢；1：调快
 * @param	Ppm 时钟微调参数，为0时硬件时钟无微调功能。
 * @return  无
 */
void Clock_AudioPllClockAdjust(PLL_CLK_INDEX Index,uint8_t Sign, uint8_t Ppm);

/**
 * @brief	DeepSleep时钟选择
 * @param	ClkMode, RC_CLK_MODE: RC12M时钟; PLL_CLK_MODE:pll时钟;
 * @return  无
 */
void Clock_DeepSleepSysClkSelect(CLK_MODE ClockSelect);


/**
 * @brief	12M晶体时钟匹配电容设置
 * @param	XICap, 12M晶体Xin电容设置，有效位4bit，0：6pF; 15：21pF；step：1pF。
 * @param	XOCap, 12M晶体Xout电容设置，有效位4bit，0：6pF; 15：21pF；step：1pF。
 * @return  无
 */
void Clock_HOSCCapSet(uint32_t XICap, uint32_t XOCap);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__CLK_H__

/**
 * @}
 * @}
 */
 
