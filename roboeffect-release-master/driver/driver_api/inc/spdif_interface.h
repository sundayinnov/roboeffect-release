/**
 **************************************************************************************
 * @file    spdif_interface.h
 * @brief   audio spdif interface
 *
 * @author  Cecilia Wang
 * @version V1.0.0
 *
 * $Created: 2015-10-21 10:01:05$
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#ifndef __SPDIF_INTERFACE_H__
#define __SPDIF_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
//--------------------------------//
void AudioSPDIF_TXInit(uint32_t SpdifPin,uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize);
void AudioSPDIF_RXInit(uint32_t SpdifPin,uint32_t SampleRate, void *PcmFifo, uint16_t PcmFifoSize);
uint16_t AudioSPDIF_GetData(uint32_t samplerate, void* Buf, uint16_t MaxLen);
void AudioSPDIF_DataSet(uint32_t samplerate, void* Buf, uint16_t MaxLen);
uint16_t AudioSPDIF_GetDataLen(void);    
uint16_t AudioSPDIF_GetSpaceLen(void); 
//--------------------------------//
#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__SPDIF_INTERFACE_H__