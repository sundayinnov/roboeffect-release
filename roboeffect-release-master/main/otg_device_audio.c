/**
 *****************************************************************************
 * @file     otg_device_audio.c
 * @author   Owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    device audio module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <string.h>
#include "type.h"
#include "otg_device_hcd.h"
#include "debug.h"
#include "otg_device_standard_request.h"
#include "audio_adc.h"
#include "dac.h"
#include "clk.h"
#include "adc_interface.h"
#include "dac_interface.h"
#include "mcu_circular_buf.h"
#include "otg_device_audio.h"
#include "user_config.h"
#include "otg_detect.h"
#ifdef CFG_APP_CONFIG
#include "resampler.h"
#include "app_config.h"
#include "audio_core_api.h"
#endif

#ifdef CFG_APP_USB_AUDIO_MODE_EN
#include"timer.h"
#include "irqn.h"
MCU_CIRCULAR_CONTEXT OtgMicFifo;

void OTG_DeviceAudioInit();

void OtgMicFifoConfig(uint8_t *Buf, uint16_t Len)
{
	MCUCircular_Config(&OtgMicFifo, Buf, Len);
}

void OtgMicFifoClear(void)
{
	OtgMicFifo.R = OtgMicFifo.W = 0;
}

void OtgMicFifoDataSet(uint8_t *Buf, uint16_t Samples)
{
	if(MCUCircular_GetSpaceLen(&OtgMicFifo) > Samples * 4)
		MCUCircular_PutData(&OtgMicFifo, Buf, Samples * 4);
}

uint16_t OtgMicFifoSpaceLenGet(void)
{
	return MCUCircular_GetSpaceLen(&OtgMicFifo) / 4;
}

uint16_t OtgMicFifoDataGet(uint8_t *Buf, uint16_t Samples)
{
	return MCUCircular_GetData(&OtgMicFifo, Buf, Samples * 4) / 4;
}

bool OtgMicOpen(void)
{
	return UsbAudioMic.InitOk && UsbAudioMic.AltSet;
}

extern void UsbAudioTimer1msProcess(void);
//__attribute__((section(".driver.isr")))
void Timer2Interrupt(void)
{
	Timer_InterruptFlagClear(TIMER2, UPDATE_INTERRUPT_SRC);
	OTG_PortLinkCheck();
#if defined(CFG_APP_USB_AUDIO_MODE_EN) && (OTG_DEVICE_MODE != AUDIO_ONLY_MIC)
	UsbAudioTimer1msProcess(); //1ms中断监控
#endif
}

#if defined(CFG_APP_USB_AUDIO_MODE_EN) && (OTG_DEVICE_MODE != AUDIO_ONLY_MIC)
bool IsUsbAudioMode =  TRUE; //USB声卡/mic数据时钟启用传输（有没有被暂停）

const uint8_t SilentData[192] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};

void UsbAudioTimer1msProcess(void)
{
	uint32_t FramLen = 192;
	static uint32_t FramCount = 0;
	static  bool IsUsbAduioInitFlag = FALSE;

	if(IsUsbAudioMode == FALSE)
	{
		return;
	}
	if(UsbAudioMic.InitOk == 0)
	{
		IsUsbAduioInitFlag = FALSE;
		return;
	}
	if(UsbAudioSpeaker.InitOk == 0)
	{
		IsUsbAduioInitFlag = FALSE;
		return;
	}

	FramCount++;
	if(AudioDAC_SampleRateGet(DAC0) == 44100)
	{
		FramLen = 176;
		if((FramCount%10) == 0)
		{
			FramCount = 0;
			FramLen = 180;
		}
	}
	////PC没有传送任何数据
	if(UsbAudioSpeaker.AltSet == 0)
	{
		//需要根据采样率填写数据
		//MCUCircular_PutData(&SpeakerCircularBuf,(void*)SilentData,FramLen);
		//uint16_t AudioDAC_DataSet(DAC_MODULE DACModule, void* Buf, uint16_t Len)
		// AudioDAC_DataSet(DAC0, (void*)SilentData, FramLen/4);
	}

	//adc
	////PC没有传送任何数据
	if(UsbAudioMic.AltSet == 0)
	{
		//需要更具采样率丢弃数据
	}
	if(!IsUsbAduioInitFlag)
	{
		IsUsbAduioInitFlag = TRUE;
	}
}
#endif

void usb_device_audio_init(uint8_t mode, uint16_t magic_num)
{
	if((mode == AUDIO)||(mode == AUDIO_HID)||(mode == AUDIO_MIC_HID))
	{
		OTG_DeviceAudioInit();
	}
	OTG_DeviceModeSel(mode, 0x1000, 0x8234 + magic_num);
	OTG_DeviceInit();
	NVIC_EnableIRQ(Usb_IRQn);
}

#define AUDIO_MAX_VOLUME	999

extern MCU_CIRCULAR_CONTEXT SpeakerCircularBuf;
extern uint8_t Setup[];
extern uint8_t Request[];
	
UsbAudioSpeaker_ UsbAudioSpeaker;
UsbAudioSpeaker_ UsbAudioMic;

uint8_t iso_dac_buf[256];
uint8_t iso_adc_buf[256];

#ifdef CFG_APP_CONFIG
uint32_t UsbAduioSampleRate = CFG_PARA_SAMPLE_RATE;
uint32_t UsbAduioMicSampleRate = CFG_PARA_SAMPLE_RATE;

#ifdef CFG_FUNC_MIXER_SRC_EN
ResamplerContext*	ResamplerCt1;
int16_t* SRCOutBuf11;
#endif
#endif

extern void OTG_DeviceSendResp(uint16_t Resp, uint8_t n);
extern uint8_t UsbAudioSourceNum(void);

/**
 * @brief  USB声卡模式下，发送反向控制命令
 * @param  Cmd 反向控制命令
 * @return 1-成功，0-失败
 */
#define AUDIO_STOP        BIT(7) 
#define AUDIO_PP          BIT(6) 

#define AUDIO_MUTE        BIT(4)

#define AUDIO_NEXT        BIT(2) 
#define AUDIO_PREV        BIT(3) 

#define AUDIO_VOL_UP      BIT(0) 
#define AUDIO_VOL_DN      BIT(1)

/////////////////////////
void PCAudioStop(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_STOP);
}
void PCAudioPP(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_PP);
}
void PCAudioNext(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_NEXT);
}
void PCAudioPrev(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_PREV);
}

void PCAudioVolUp(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_VOL_UP);
}

void PCAudioVolDn(void)
{
	OTG_DeviceAudioSendPcCmd(AUDIO_VOL_DN);
}

bool OTG_DeviceAudioSendPcCmd(uint8_t Cmd)
{
	OTG_DeviceInterruptSend(0x01,&Cmd, 1,1000);
	Cmd = 0;
	OTG_DeviceInterruptSend(0x01,&Cmd, 1,1000);
	return TRUE;
}

//转采样直接在中断中处理，转采样时间大约是180us。
//注意一下需要4字节对齐
void OnDeviceAudioRcvIsoPacket(void)
{
#ifdef CFG_FUNC_MIXER_SRC_EN
	int32_t SRCDoneLen; 			//SRC之后存放于outbuf的数据长度
	//int32_t MaxLen = MAX_FRAME_SAMPLES;
#endif
	uint32_t Len;
	OTG_DeviceISOReceive(DEVICE_ISO_OUT_EP, iso_dac_buf, 256, &Len);
#ifndef CFG_APP_CONFIG
//	AudioDAC_DataSet(DAC0, iso_dac_buf, Len/4);
#else
#ifdef CFG_FUNC_MIXER_SRC_EN
	if(UsbAduioSampleRate != CFG_PARA_SAMPLE_RATE)
	{
		SRCDoneLen = resampler_apply(ResamplerCt1, (int16_t*)iso_dac_buf, (int16_t*)SRCOutBuf11, Len / 4);
		MCUCircular_PutData(&SpeakerCircularBuf, SRCOutBuf11, SRCDoneLen * 2 * 2);
	}
	else
	{
		MCUCircular_PutData(&SpeakerCircularBuf, iso_dac_buf,Len);
	}
#else
	MCUCircular_PutData(&SpeakerCircularBuf, iso_dac_buf,Len);
#endif
#endif
}
extern volatile uint32_t gSysTick;
void OnDeviceAudioSendIsoPacket(void)
{
	uint32_t RealLen;
	if(UsbAudioMic.SampleRate == 44100 && UsbAudioMic.LastTick % 10 == 0)
	{
		RealLen = 45 * 4;
	}
	else
	{
		RealLen = 4 * (UsbAudioMic.SampleRate / 1000);
	}
	UsbAudioMic.LastTick = gSysTick;
	if(MCUCircular_GetDataLen(&OtgMicFifo) < RealLen)
	{
		memset(iso_adc_buf, 0, RealLen);
	}
	else
	{
		MCUCircular_GetData(&OtgMicFifo, iso_adc_buf, RealLen);
	}

	OTG_DeviceISOSend(DEVICE_ISO_IN_EP,iso_adc_buf,RealLen);

}

//声卡Speaker音量控制接口

void OTG_DeviceAudioInit()
{
	UsbAudioSpeaker.LastTick = 0;
	UsbAudioSpeaker.AltSet = 0;
	UsbAudioSpeaker.LeftVol = AUDIO_MAX_VOLUME / 10;
	UsbAudioSpeaker.RightVol = AUDIO_MAX_VOLUME / 10;
	UsbAudioSpeaker.MuteFlag = FALSE;
	UsbAudioSpeaker.SampleRate  = CFG_SAMPLE_RATE;

	UsbAudioMic.LastTick = 0;
	UsbAudioMic.AltSet = 0;
	UsbAudioMic.LeftVol = AUDIO_MAX_VOLUME;
	UsbAudioMic.RightVol = AUDIO_MAX_VOLUME;
	UsbAudioMic.MuteFlag = FALSE;
	UsbAudioMic.SampleRate = CFG_SAMPLE_RATE;
}

void OTG_DeviceAudioRequest(void)
{
	//AUDIO控制接口组件ID号定义（必须与device_stor_audio_request.c中的定义保持一致！）
	#define AUDIO_SPEAKER_IT_ID		1
	#define AUDIO_SPEAKER_FU_ID		2	//控制MUTE、VOLUME
	#define AUDIO_SPEAKER_OT_ID		3
	#define AUDIO_MIC_IT_ID			4
	#define AUDIO_MIC_FU_ID			5
	#define AUDIO_MIC_SL_ID			6
	#define AUDIO_MIC_OT_ID			7
	
	#define AudioCmd	((Setup[0] << 8) | Setup[1])
	#define Channel		Setup[2]
	#define Control		Setup[3]
	#define Entity		Setup[5]
	
	#define SET_CUR		0x2101
	#define SET_IDLE	0x210A
	#define GET_CUR		0xA181
	#define GET_MIN		0xA182
	#define GET_MAX		0xA183
	#define GET_RES		0xA184
	
	#define SET_CUR_EP	0x2201
	#define GET_CUR_EP	0xA281
	
	//AUDIO类请求处理
	if(AudioCmd == SET_CUR_EP)
	{

		if(Setup[4] == 0x84)
		{
			UsbAudioMic.SampleRate = Request[1]*256 + Request[0];
			OTG_DBG("Set Mic samplerate %d\n", UsbAudioMic.SampleRate);
		}
		else
		{
			UsbAudioSpeaker.SampleRate =Request[1]*256 + Request[0];
			OTG_DBG("Set speaker samplerate %d\n", UsbAudioSpeaker.SampleRate);
		}
		return;
	}
	if(AudioCmd == GET_CUR_EP)
	{
		uint32_t Temp = 0;
		if(Setup[4] == 0x84)
		{
			Temp = UsbAudioMic.SampleRate;
			OTG_DBG("Get Mic samplerate\n");
		}
		else
		{
			Temp =UsbAudioSpeaker.SampleRate;
			OTG_DBG("Get speaker samplerate\n");
		}
		Setup[0] = (Temp>>0 ) & 0x000000FF;
		Setup[1] = (Temp>>8 ) & 0x000000FF;
		Setup[2] = (Temp>>16) & 0x000000FF;
		OTG_DeviceControlSend(Setup,3,3);
		return;
	}

	if((Entity == AUDIO_SPEAKER_FU_ID) && (Control == 0x01))
	{
		//Speaker mute的操作
		if(AudioCmd == GET_CUR)
		{
			Setup[0] = UsbAudioSpeaker.MuteFlag;
			OTG_DeviceControlSend(Setup,1,3);
		}
		else if(AudioCmd == SET_CUR)
		{
			OTG_DBG("Set speaker mute: %d\n", Request[0]);
			UsbAudioSpeaker.MuteFlag = Request[0];
		}
		else
		{
			OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_SPEAKER_FU_ID) && (Control == 0x02))
	{
		//Speaker volume的操作
		if(AudioCmd == GET_MIN)
		{
			OTG_DBG("Get speaker min volume\n");
			OTG_DeviceSendResp(0x0000, 2);
		}
		else if(AudioCmd == GET_MAX)
		{
			OTG_DBG("Get speaker max volume\n");
			OTG_DeviceSendResp(AUDIO_MAX_VOLUME, 2);
		}
		else if(AudioCmd == GET_RES)
		{
			OTG_DBG("Get speaker res volume\n");
			OTG_DeviceSendResp(0x0001, 2);
		}
		else if(AudioCmd == GET_CUR)
		{
			uint32_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = UsbAudioSpeaker.LeftVol;
			}
			else
			{
				Vol = UsbAudioSpeaker.RightVol;
			}
			OTG_DeviceSendResp(Vol, 2);
		}
		else if(AudioCmd == SET_CUR)
		{
			uint32_t Temp = 0;
			Temp = (Request[1] & 0x7F) * 256 + Request[0];
			if(Setup[2] == 0x01)
			{
				UsbAudioSpeaker.LeftVol = Temp;
			}
			else
			{
				UsbAudioSpeaker.RightVol = Temp;
			}
		}
		else
		{
			OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_MIC_FU_ID) && (Control == 0x01))
	{
		//Mic mute的操作
		if(AudioCmd == GET_CUR)
		{
			OTG_DeviceSendResp(UsbAudioMic.MuteFlag, 1);
			OTG_DBG("Get mic mute: %d\n", (int)UsbAudioMic.MuteFlag);
		}
		else if(AudioCmd == SET_CUR)
		{
			OTG_DBG("Set mic mute: %d\n", Request[0]);
			UsbAudioMic.MuteFlag = Request[0];
		}
		else
		{
			OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if((Entity == AUDIO_MIC_FU_ID) && (Control == 0x02))
	{
		//Mic volume的操作
		if(AudioCmd == GET_MIN)
		{
			OTG_DBG("Get mic min volume\n");
			OTG_DeviceSendResp(0x0000, 2);
		}
		else if(AudioCmd == GET_MAX)
		{
			OTG_DBG("Get mic max volume\n");
			OTG_DeviceSendResp(AUDIO_MAX_VOLUME*4, 2);	//此处乘以4的原因请看本文件开头的注释说明
		}
		else if(AudioCmd == GET_RES)
		{
			OTG_DBG("Get mic res volume\n");
			OTG_DeviceSendResp(0x0001, 2);
		}
		else if(AudioCmd == GET_CUR)
		{
			uint32_t Vol = 0;
			if(Channel == 0x01)
			{
				Vol = UsbAudioMic.LeftVol;
			}
			else
			{
				Vol = UsbAudioMic.RightVol;
			}
			OTG_DeviceSendResp(Vol*4, 2);
		}
		else if(AudioCmd == SET_CUR)
		{
			uint32_t Vol = ((Request[1] & 0x7F) * 256 + Request[0])/4;;
			if(Setup[2] == 0x01)
			{
				UsbAudioMic.LeftVol = Vol;
			}
			else
			{
				UsbAudioMic.RightVol = Vol;
			}
		}
		else
		{
			OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(Entity == AUDIO_MIC_SL_ID)
	{
		//Selector的操作
		if(AudioCmd == GET_CUR)
		{
			OTG_DBG("Get selector: 1\n");
			OTG_DeviceSendResp(0x01, 1);
		}
		else
		{
			OTG_DBG("%s %d\n",__FILE__,__LINE__);
		}
	}
	else if(AudioCmd == SET_IDLE)
	{
		OTG_DBG("Set idle\n");
	}	
	else
	{
		//其他AUDIO类的输入请求
		OTG_DBG("Unkown command! %02X %02X %02X %02X %02X %02X %02X %02X\n", Setup[0], Setup[1], Setup[2], Setup[3], Setup[4], Setup[5], Setup[6], Setup[7]);
		OTG_DeviceSendResp(0x0000, 1);
	}
}

#endif

