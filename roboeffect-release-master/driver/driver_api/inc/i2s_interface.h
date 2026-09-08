/**
 **************************************************************************************
 * @file    adc_interface.h
 * @brief   audio adc interface
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2017-11-23 14:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef		__AudioI2S_INTERFACE_H__
#define		__AudioI2S_INTERFACE_H__

void AudioI2S_Bclk_Reset(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, uint8_t lrclk_rate);
void AudioI2S_RXInit(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize);
void AudioI2S_TXInit(I2S_MODULE I2SModuleIdx, uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize);
uint16_t AudioI2S_GetSpaceLen(I2S_MODULE I2SModuleIdx);
void  AudioI2S_DataSet(I2S_MODULE I2SModuleIdx, void* Buf, uint16_t Len);
uint16_t AudioI2S_GetDataLen(I2S_MODULE I2SModuleIdx);
uint16_t AudioI2S_GetData(I2S_MODULE I2SModuleIdx, void* Buf, uint16_t MaxLen);
#endif
