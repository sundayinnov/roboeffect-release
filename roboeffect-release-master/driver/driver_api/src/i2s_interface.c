/**
 **************************************************************************************
 * @file    audioI2S_interface.c
 * @brief   audio I2S interface
 *
 * @author
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#include <stdint.h>
#include <string.h>
#include "type.h"
#include "i2s.h"
#include "dma.h"
#include "clk.h"
#include "gpio.h"
#include "user_config.h"

static const uint32_t BclkTable[30] = { 2, 3,  4, 6, 8, 10, 11, 12, 16, 
							   20, 22, 24, 32,  40, 44, 48, 50,
							   60, 64, 88, 96, 100}; 



void AudioI2S_Bclk_Reset(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, uint8_t lrclk_rate)
{
	uint32_t I2sBclk_Rate;
	uint32_t I2sLrclk_Rate;
	uint32_t I2sMclk;
	uint32_t i;

	if(lrclk_rate != 64 && lrclk_rate != 32 && lrclk_rate != 48 && lrclk_rate != 40)
	{
		I2sLrclk_Rate = 64;
	}
	else
	{
		I2sLrclk_Rate = lrclk_rate;
	}

	if((SampleRate == 11025) || (SampleRate == 22050) || (SampleRate == 44100) || (SampleRate == 88200) || (SampleRate == 176400))
	{
		I2sMclk = 11289600;
	}
	else
	{
		I2sMclk = 12288000;
	}
	
	I2sBclk_Rate = I2sMclk / (I2sLrclk_Rate * SampleRate);
	I2sBclk_Rate = (I2sBclk_Rate << 1);
	for(i=0; i<21; i++)
	{
		if((I2sBclk_Rate >= BclkTable[i]) && (I2sBclk_Rate < BclkTable[i+1]))
		{
			break;
		}
	}
	
	I2S_LrclkFreqSet(I2SModuleIdx, I2sLrclk_Rate);
	I2S_BclkFreqSet(I2SModuleIdx, i);
}

void AudioI2S_TXInit(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize)
{
	uint8_t 		PeripheralIDTmp;

	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_TX : PERIPHERAL_ID_I2S0_TX);

	DMA_ChannelDisable(PeripheralIDTmp);
	DMA_CircularConfig(PeripheralIDTmp, PcmFifoSize/2, PcmFifo, PcmFifoSize);
	//DMA_ChannelEnable(PeripheralIDTmp);

	if(I2SModuleIdx == I2S0_MODULE)
	{
		I2S_ModuleDisable(I2S0_MODULE);
		//Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK2);

		// GPIO_PortAModeSet(CFG_I2S0_LRCLK_PIN, CFG_I2S0_LRCLK_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S0_BCLK_PIN, CFG_I2S0_BCLK_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S0_DO_PIN, CFG_I2S0_DO_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S0_DIN_PIN, CFG_I2S0_DIN_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S0_MCLK_PIN, CFG_I2S0_MCLK_OUT_PIN_MUX_SEL);

		I2S_SampleRateSet(I2S0_MODULE, SampleRate);
		I2S_FadeTimeSet(I2S0_MODULE, (2^12)/(44100*100));
		I2S_SetMasterMode(I2S0_MODULE, I2S_FORMAT_I2S, I2S_LENGTH_16BITS);//I2S_LENGTH_16BITS I2S_LENGTH_24BITS
		I2S_AlignModeSet(I2S0_MODULE, I2S_LOW_BITS_ACTIVE);
		I2S_ModuleEnable(I2S0_MODULE);
	}
	else
	{
		I2S_ModuleDisable(I2S1_MODULE);

		//Clock_AudioMclkSel(AUDIO_I2S1, PLL_CLOCK2);
		// GPIO_PortAModeSet(CFG_I2S1_LRCLK_PIN, CFG_I2S1_LRCLK_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S1_BCLK_PIN, CFG_I2S1_BCLK_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S1_DO_PIN, CFG_I2S1_DO_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S1_DIN_PIN, CFG_I2S1_DIN_PIN_MUX_SEL);
		// GPIO_PortAModeSet(CFG_I2S1_MCLK_PIN, CFG_I2S1_MCLK_OUT_PIN_MUX_SEL);

		I2S_SampleRateSet(I2S1_MODULE, SampleRate);
		I2S_FadeTimeSet(I2S0_MODULE, (2^12)/(44100*100));
		I2S_SetMasterMode(I2S1_MODULE, I2S_FORMAT_I2S, I2S_LENGTH_16BITS);//I2S_LENGTH_16BITS I2S_LENGTH_24BITS
		I2S_AlignModeSet(I2S1_MODULE, I2S_LOW_BITS_ACTIVE);
		I2S_ModuleEnable(I2S1_MODULE);
	}
	DMA_ChannelEnable(PeripheralIDTmp);
}

void AudioI2S_RXInit(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize)
{
	uint8_t 		PeripheralIDTmp;

	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_RX : PERIPHERAL_ID_I2S0_RX);

	DMA_ChannelDisable(PeripheralIDTmp);
	DMA_CircularConfig(PeripheralIDTmp, PcmFifoSize/2, PcmFifo, PcmFifoSize);
	//DMA_ChannelEnable(PeripheralIDTmp);

	if(I2SModuleIdx == I2S0_MODULE)
	{
		I2S_ModuleDisable(I2S0_MODULE);
		//Clock_AudioMclkSel(AUDIO_I2S0, PLL_CLOCK2);

		// AudioI2S_GPIO_ModeSet(I2S0_MODULE);
		I2S_SampleRateSet(I2S0_MODULE, SampleRate);
		I2S_SetMasterMode(I2S0_MODULE, I2S_FORMAT_I2S, I2S_LENGTH_16BITS);//I2S_LENGTH_16BITS I2S_LENGTH_24BITS
		I2S_AlignModeSet(I2S0_MODULE, I2S_LOW_BITS_ACTIVE);
		I2S_ModuleEnable(I2S0_MODULE);
	}
	else
	{
		I2S_ModuleDisable(I2S1_MODULE);

		//Clock_AudioMclkSel(AUDIO_I2S1, PLL_CLOCK2);
		// AudioI2S_GPIO_ModeSet(I2S1_MODULE);
		I2S_SampleRateSet(I2S1_MODULE, SampleRate);
		I2S_SetMasterMode(I2S1_MODULE, I2S_FORMAT_I2S, I2S_LENGTH_16BITS);//I2S_LENGTH_16BITS I2S_LENGTH_24BITS
		I2S_AlignModeSet(I2S1_MODULE, I2S_LOW_BITS_ACTIVE);
		I2S_ModuleEnable(I2S1_MODULE);
	}
	DMA_ChannelEnable(PeripheralIDTmp);
}


uint16_t AudioI2S_GetSpaceLen(I2S_MODULE I2SModuleIdx)
{
	uint8_t 		PeripheralIDTmp;

	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_TX : PERIPHERAL_ID_I2S0_TX);
	return DMA_CircularSpaceLenGet(PeripheralIDTmp)/4;
}

void  AudioI2S_DataSet(I2S_MODULE I2SModuleIdx, void* Buf, uint16_t Len)
{
	uint8_t 		PeripheralIDTmp;

	Len = Len * 4;
	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_TX : PERIPHERAL_ID_I2S0_TX);
	DMA_CircularDataPut(PeripheralIDTmp, Buf, Len & 0xFFFFFFFC);
	//DMA_CircularDataPut(PeripheralIDTmp, Buf, Len);
}

uint16_t AudioI2S_GetDataLen(I2S_MODULE I2SModuleIdx)
{
	uint8_t 		PeripheralIDTmp;

	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_RX : PERIPHERAL_ID_I2S0_RX);
	return DMA_CircularDataLenGet(PeripheralIDTmp) / 4;
}

uint16_t AudioI2S_GetData(I2S_MODULE I2SModuleIdx, void* Buf, uint16_t MaxLen)
{
	uint8_t 		PeripheralIDTmp;

	PeripheralIDTmp = (I2SModuleIdx == I2S1_MODULE? PERIPHERAL_ID_I2S1_RX : PERIPHERAL_ID_I2S0_RX);
	uint16_t Len = DMA_CircularDataLenGet(PeripheralIDTmp);

	if(Len > MaxLen * 4)
	{
		Len = MaxLen * 4;
	}

	DMA_CircularDataGet(PeripheralIDTmp, Buf, Len & 0xFFFFFFFC);

	return Len/4;
}
