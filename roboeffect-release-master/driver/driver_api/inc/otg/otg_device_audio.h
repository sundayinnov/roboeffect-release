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

#ifndef __OTG_DEVICE_AUDIO_H__
#define	__OTG_DEVICE_AUDIO_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
	
#include "type.h"


typedef uint32_t (*UsbAudioGetFunc)();
typedef uint32_t (*UsbAudioSetFunc)(uint32_t Samples);

typedef struct _UsbAudioSpeaker_
{
	uint32_t			LastTick;
	uint32_t			SampleRate;
	uint16_t			LeftVol;
	uint16_t			RightVol;

	bool				MuteFlag;
	bool				AltSet;
	bool				InitOk;

}UsbAudioSpeaker_;

extern UsbAudioSpeaker_ UsbAudioSpeaker;
extern UsbAudioSpeaker_ UsbAudioMic;

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd);

void OtgMicFifoClear(void);
void hid_recive_data();
void hid_send_data(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__OTG_DEVICE_AUDIO_H__
