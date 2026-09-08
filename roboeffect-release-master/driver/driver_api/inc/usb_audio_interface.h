/**
 *****************************************************************************
 * @file     otg_device_audio.h
 * @author   Owen
 * @version  V1.0.0
 * @date     24-June-2015
 * @brief    audio device interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#ifndef __USB_AUDIO_INTERFACE_H__
#define	__USB_AUDIO_INTERFACE_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"
#include "mcu_circular_buf.h"

typedef struct __USB_AUDIO_UNIT
{
	MCU_CIRCULAR_CONTEXT  ct;
	uint8_t				 *fifo_addr;
	uint16_t 			  fifo_size;

} usb_audio_unit;
void AudioUSB_GetInit(usb_audio_unit *usb_audio_get);
void AudioUSB_PutInit(usb_audio_unit * usb_audio_put);
uint16_t AudioUSB_GetDataLen(void);
uint16_t AudioUSB_GetData(void* Buf, uint16_t MaxLen);
uint16_t AudioUSB_DataSet(void* Buf, uint16_t MaxLen);

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd);

void AudioUSB_ModeSet(uint8_t USB_AUDIO_MDOE);

/////////////////////////
void PCAudioStop(void);

void PCAudioPP(void);

void PCAudioNext(void);

void PCAudioPrev(void);

void PCAudioVolUp(void);

void PCAudioVolDn(void);

//-------------------------------------------//
#ifdef  __cplusplus
}
#endif//__cplusplus

#endif
