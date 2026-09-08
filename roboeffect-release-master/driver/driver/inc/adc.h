/**
 *******************************************************************************
 * @file    adc.h
 * @brief	SarAdc module driver interface
 
 * @author  Sam
 * @version V1.0.0
 
 * $Created: 2014-12-26 14:01:05$
 * @Copyright (C) 2014, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 ******************************************************************************* 
 */

/**
 * @addtogroup ADC
 * @{
 * @defgroup adc adc.h
 * @{
 */
 
#ifndef __ADC_H__
#define __ADC_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 

typedef enum __ADC_DC_CLK_DIV
{
	CLK_DIV_NONE,
	CLK_DIV_2,
	CLK_DIV_4,
	CLK_DIV_8,
	CLK_DIV_16,
	CLK_DIV_32,
	CLK_DIV_64,
	CLK_DIV_128,
	CLK_DIV_256,
	CLK_DIV_512,
	CLK_DIV_1024,
	CLK_DIV_2048,
	CLK_DIV_4096
	
}ADC_DC_CLK_DIV;


typedef enum __ADC_DC_CHANNEL_SEL
{
	ADC_CHANNEL_VIN,		/**channel 0*/
	ADC_CHANNEL_VDDA,		/**channel 1*/
	ADC_CHANNEL_VDD1V2,		/**channel 2*/
	ADC_CHANNEL_GPIOA17,	/**channel 3*/
	ADC_CHANNEL_GPIOA18,	/**channel 4*/
	ADC_CHANNEL_GPIOA23,	/**channel 5*/
	ADC_CHANNEL_GPIOA24,	/**channel 6*/
	ADC_CHANNEL_GPIOA25,	/**channel 7*/
	ADC_CHANNEL_GPIOA26,	/**channel 8*/
	ADC_CHANNEL_GPIOA27,	/**channel 9*/
	ADC_CHANNEL_GPIOA28,	/**channel 10*/
	ADC_CHANNEL_GPIOA29,	/**channel 11*/
	ADC_CHANNEL_GPIOA30,	/**channel 12*/
	ADC_CHANNEL_GPIOA31,	/**channel 13*/
	ADC_CHANNEL_GPIOB6,		/**channel 14*/
	ADC_CHANNEL_GPIOB7		/**channel 15*/
	
}ADC_DC_CHANNEL_SEL;


typedef enum __ADC_VREF
{
	ADC_VREF_VDD,
	ADC_VREF_VDDA,
	ADC_VREF_Extern //外部输入
	
}ADC_VREF;

typedef enum __ADC_CONV
{
	ADC_CON_SINGLE,
	ADC_CON_CONTINUA
}ADC_CONV;

/**
 * @brief  ADC使能
 * @param  无
 * @return 无
 */
void ADC_Enable(void);

/**
 * @brief  ADC禁能
 * @param  无
 * @return 无
 */
void ADC_Disable(void);

/**
 * @brief  ADC电容校准，校准之后会提高ADC采样精度
 * @param  无
 * @return 无
 */
void ADC_Calibration(void);

/**
 * @brief  设置ADC工作时钟分频
 * @param  Div 分频系数
 * @return 无
 */
void ADC_ClockDivSet(ADC_DC_CLK_DIV Div);

/**
 * @brief  获取ADC工作时钟分频
 * @param  无
 * @return 分频系数
 */
ADC_DC_CLK_DIV ADC_ClockDivGet(void);

/**
 * @brief  ADC参考电压选择
 * @param  Mode 参考电压源选择。2:Extern Vref; 1:VDDA; 0: 2*VMID
 * @return 无
 */
void ADC_VrefSet(ADC_VREF Mode);

/**
 * @brief  设置ADC转换工作模式
 * @param  Mode 工作模式。0:单次模式; 1:连续模式（配合DMA使用，只能采样一个通道）
 * @return 无
 */
void ADC_ModeSet(ADC_CONV Mode);

/**
 * @brief  获取ADC采样数据
 * @param  ChannalNum ADC采样的通道。
 * @return ADC采样量化后数据[0-4095]
 */
int16_t ADC_SingleModeDataGet(uint32_t ChannalNum);

//连续模式下可以使用DMA
/**
 * @brief  使能ADC转换采用DMA方式
 * @param  无
 * @return 无
 * @note  使用前先配置DMA所用的通道，并且DMA方式下ADC只能单通道采样
 */
void ADC_DMAEnable(void);

/**
 * @brief  ADC转换DMA模式禁能
 * @param  无
 * @return 无
 * @note  使用前先配置DMA所用的通道，并且DMA方式下ADC只能单通道采样
 */
void ADC_DMADisable(void);

/**
 * @brief  ADC连续采样方式启动
 * @param  ChannalNum ADC采样的通道。
 * @return 无
 */
void ADC_ContinuModeStart(uint32_t ChannalNum);

/**
 * @brief  MCU获取ADC连续采样方式采样结果
 * @param  无
 * @return ADC采样量化后数据[0-4095]
 */
uint16_t ADC_ContinuModeDataGet(void);

/**
 * @brief  ADC连续采样方式结束
 * @param  无
 * @return 无
 */
void ADC_ContinuModeStop(void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // __ADC_H__
/**
 * @}
 * @}
 */
 

