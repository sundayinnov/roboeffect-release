/**
  ******************************************************************************
  * @file    reset.h
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
 * @defgroup reset reset.h
 * @{
 */

#ifndef __RESET_H__
#define __RESET_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

/**
 * reset module(Register) macro define
 */
typedef enum __REGISTER_RESET_MODULE
{
	PWR_REG_SEPA = (1 << 0),				/**<Power register separate reset*/
	EFUSE_REG_SEPA = (1 << 1),				/**<Efuse register separate reset*/
	FLASH_CONTROL_REG_SEPA = (1 << 2),		/**<Flash control register separate reset*/

	TIMER1_TIMER2_REG_SEPA = (1 << 4),      /**<Timer1 and Timer2 register separate reset */
	TIMER5_TIMER6_REG_SEPA = (1 << 5),		/**<Timer5 and Timer6 register separate reset */
	TIMER3_TIMER4_REG_SEPA = (1 << 6),      /**<Timer3 and Timer4 register separate reset */
	SDIO0_REG_SEPA = (1 << 7),				/**<SDIO0 register separate reset */
	SDIO1_REG_SEPA = (1 << 8),       		/**<SDIO1 register separate reset */
	SPIM_REG_SEPA = (1 << 9),				/**<SPIM register separate reset */
	UART0_REG_SEPA = (1 << 10),				/**<UART0 register separate reset */
	UART1_REG_SEPA = (1 << 11),				/**<UART1 register separate reset */
	DMIC_REG_SEPA = (1 << 12),				/**<DMIC register separate reset */
	USB_REG_SEPA = (1 << 13),				/**<USB register separate reset */
	PSRAM_REG_SEPA = (1 << 14),          	/**<PSRAM register separate reset */
	SPIS_REG_SEPA = (1 << 15),           	/**<SPIS register separate reset */

	I2S0_REG_SEPA = (1 << 17),   			/**<I2S0 register separate reset */
	I2S1_REG_SEPA = (1 << 18),           	/**<I2S1 register separate reset */
	AUDIO_DAC_REG_SEPA = (1 << 19),         /**<Audio DAC0 and DAC1 register separate reset */
	AUDIO_ADC_REG_SEPA = (1 << 20),         /**<Audio ADC0 and ADC1 register separate reset */
	ADC_REG_SEPA = (1 << 21),          		/**<SarADC register separate reset */
	FFT_REG_SEPA = (1 << 22),          		/**<FFT register separate reset */

} REGISTER_RESET_MODULE;

/**
 * reset module(Function) macro define
 */
typedef enum __FUNCTION_RESET_MODULE
{
	I2S0_FUNC_SEPA = (1 << 0),				/**<I2S0 function separate reset*/
	I2S1_FUNC_SEPA = (1 << 1),				/**<I2S1 function separate reset*/
	AUDIO_ADC0_FUNC_SEPA = (1 << 2),		/**<Audio ADC0 function separate reset*/
	AUDIO_ADC1_FUNC_SEPA = (1 << 3),		/**<Audio ADC1 function separate reset*/
	AUDIO_DAC0_FUNC_SEPA = (1 << 4),        /**<Audio DAC0 function separate reset */
	AUDIO_DAC1_FUNC_SEPA = (1 << 5),		/**<Audio ADC1 function separate reset */
	ADC_FUNC_SEPA = (1 << 6),       		/**<SarADC function separate reset */
	USB_FUNC_SEPA = (1 << 7),				/**<USB function separate reset */
	UART0_FUNC_SEPA = (1 << 8),      	 	/**<UART0 function separate reset */
	UART1_FUNC_SEPA = (1 << 9),				/**<UART1 function separate reset */
	FLASH_CONTROL_FUNC_SEPA = (1 << 10),	/**<Flash control function separate reset */
	TIMER1_FUNC_SEPA = (1 << 11),			/**<Timer1 function separate reset */
	TIMER2_FUNC_SEPA = (1 << 12),			/**<Timer2 function separate reset */
	TIMER3_FUNC_SEPA = (1 << 13),			/**<Timer3 function separate reset */
	TIMER4_FUNC_SEPA = (1 << 14),           /**<Timer4 function separate reset */
	TIMER5_FUNC_SEPA = (1 << 15),           /**<Timer5 function separate reset */
	TIMER6_FUNC_SEPA = (1 << 16),           /**<Timer6 function separate reset */
	SPIM_FUNC_SEPA = (1 << 17),   			/**<SPIM function separate reset */
	SDIO0_FUNC_SEPA = (1 << 18),           	/**<SDIO0 function separate reset */
	SDIO1_FUNC_SEPA = (1 << 19),            /**<SDIO1 function separate reset */
	DMA_FUNC_SEPA = (1 << 20),          	/**<DMA function separate reset */
	I2C_FUNC_SEPA = (1 << 21),          	/**<I2C function separate reset */
	USI_FUNC_SEPA = (1 << 22),          	/**<USI function separate reset */
	DMIC_FUNC_SEPA = (1 << 23),          	/**<DMIC function separate reset */
	PSRAM_FUNC_SEPA = (1 << 24),          	/**<PSRAM function separate reset */
	SPDIF_FUNC_SEPA = (1 << 25),          	/**<Spdif function separate reset */
	SPIS_FUNC_SEPA = (1 << 26),          	/**<SPIS function separate reset */
	FFT_FUNC_SEPA = (1 << 27),          	/**<FFT function separate reset */

} FUNCTION_RESET_MODULE;

/**
 * @brief	获取复位源
 * @param	None
 * @return	返回复位源
 * 			bit0=1	Power On Reset
 * 			bit1=1	Pin Reset
 * 			bit2=1	WatchDog Reset
 * 			bit3=1	LVD Reset
 * 			bit4=1	CPU Debug Reset
 * 			bit5=1	System Reset
 * 			bit6=1	CPU Reset
 * @note
 */
uint16_t Reset_FlagGet(void);

/**
 * @brief	Clear复位标志
 * @param	None
 * @return	None
 * @note
 */
void Reset_FlagClear(void);

/**
 * @brief	系统复位
 * @param	None
 * @return	None
 * @note
 */
void Reset_McuSystem(void);

/**
 * @brief	模块寄存器复位
 * @param	module：模块
 * @return	None
 * @note
 */
void Reset_RegisterReset(REGISTER_RESET_MODULE module);

/**
 * @brief	模块功能复位
 * @param	module：模块
 * @return	None
 * @note
 */
void Reset_FunctionReset(FUNCTION_RESET_MODULE module);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__RESET_H__

/**
 * @}
 * @}
 */

