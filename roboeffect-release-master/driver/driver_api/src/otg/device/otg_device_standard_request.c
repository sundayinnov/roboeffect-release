/**
 *****************************************************************************
 * @file     device_stor_audio_request.c
 * @author   owen
 * @version  V1.0.0
 * @date     7-September-2015
 * @brief    device audio and mass-storage module driver interface
 *****************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
 */

#include <string.h>
#include "type.h"
#include "debug.h"
#include "user_config.h"
#include "otg_device_hcd.h"
#include "otg_device_standard_request.h"
#include "otg_device_descriptor.h"
#include "otg_device_audio.h"
#if 1//CFG_USB_EN
//------------------------------------//
void HIDUsb_Tx(uint8_t *buf,uint16_t len);
void HIDUsb_Rx(uint8_t *buf,uint16_t len);

uint8_t *hid_buf;
uint8_t *hid_send;

extern uint8_t hid_rx_buf[256];
extern uint8_t hid_tx_buf[256];

//------------------------------------//


uint8_t const DeviceQualifier[10] = {10,6,0x10,0x01,0,0,0,64,1,0};
extern void OnDeviceAudioRcvIsoPacket(void);
extern void OnDeviceAudioSendIsoPacket(void);

extern void OTG_DeviceAudioRequest(void);

uint8_t DeviceString_LangID[] = {0x04, 0x03, 0x09, 0x04};

uint8_t Setup[8];
uint8_t Request[8];
extern uint32_t usb_device_run;

uint8_t *ConfigDescriptor;
uint8_t *InterfaceNum;
char *gDeviceProductString ="mvsiicon 026 usb audio";		//max length: 32bytes
char *gDeviceString_Manu ="MVSILICON";		//max length: 32bytes
char *gDeviceString_SerialNumber ="20151012";//max length: 32bytes
uint8_t *gDeviceString_Index;

void OTG_Device_SampleRate_Set(uint32_t sample_rate)
{
	uint8_t *data = ConfigDescriptor_Tab(AUDIO_MIC_HID);
	data[97] = sample_rate & 0xff;
	data[98] = (sample_rate>>8) & 0xff;
	data[99] = (sample_rate>>16) & 0xff;
}

void OTG_DeviceModeSel(uint8_t Mode,uint16_t UsbVid,uint16_t UsbPid)
{
    hid_buf             = &hid_rx_buf[0];
	hid_send            = &hid_tx_buf[0];
	
	DeviceDescriptor[8] = UsbVid;
	DeviceDescriptor[9] = UsbVid>>8;
	DeviceDescriptor[10] = UsbPid;
	DeviceDescriptor[11] = UsbPid>>8;
	ConfigDescriptor = (uint8_t *)ConfigDescriptorTab[Mode];
	InterfaceNum = (uint8_t *)InterFaceNumTab[Mode];
	
    #if 1//CFG_COMMUNICATION_BY_USB
	DeviceDescriptor[8] = 0x88;
	DeviceDescriptor[9] = 0x88;
	DeviceDescriptor[10] = 0x34;
	DeviceDescriptor[11] = 0x12;	
	gDeviceString_Index = "MVSILICON";

	gDeviceProductString		    = (uint8_t*)"ROBO-EFFECT-Demo"; 		//max length: 32bytes
	gDeviceString_Manu 		        = (uint8_t*)"MV-SILICON";			    //max length: 32bytes
	gDeviceString_SerialNumber      = (uint8_t*)"20230724";					//max length: 32bytes
	
    #endif

	
}

/**
 * HID报告描述符
 */
 /*
const uint8_t gHidReportDescriptor[] =
{
	0x05, 0x0C, 0x09, 0x01, 0xA1, 0x01, 0x15, 0x00, 0x25, 0x01, 0x09, 0xE9, 0x09, 0xEA, 0x09, 0xB5,
	0x09, 0xB6, 0x75, 0x01, 0x95, 0x04, 0x81, 0x02, 0x09, 0xE2, 0x09, 0xB7, 0x09, 0xCD, 0x95, 0x03,
	0x81, 0x06, 0x05, 0x0B, 0x09, 0x20, 0x95, 0x01, 0x81, 0x06, 0x05, 0x0C, 0x26, 0xFF, 0x00, 0x09,
	0x00, 0x75, 0x08, 0x95, 0x03, 0x81, 0x02, 0x09, 0x00, 0x95, 0x04, 0x91, 0x02, 0xC0

};

uint8_t gHidReportDescriptor1[] =
{
0x06, 0x00, 0xFF,
0x09, 0x01,
0xA1, 0xFF,
0x19, 0x01,
0x29, 0x40,
0x15, 0x00,
0x25, 0xFF,
0x75, 0x08,
0x95, 0x40,
0x91, 0x02,
0xC0
};

const uint8_t keybordreport[65] = {
	0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01,
	0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01,
	0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x06,
	0x75, 0x08, 0x15, 0x00, 0x26, 0xA4, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2A, 0xA4, 0x00, 0x81, 0x00,
	0xC0
};
*/
/**
 * @brief  发送控制传输命令的应答数据
 * @param  Resp 应答数据
 * @param  n 应答数据长度，1 or 2
 * @return NONE
 */
void OTG_DeviceSendResp(uint16_t Resp, uint8_t n)
{
	Resp = CpuToLe16(Resp);
	OTG_DeviceControlSend((uint8_t*)&Resp, n,3);
}

/**
 * @brief  处理获取描述符命令
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceGetDescriptor(void)
{
	uint8_t 	StringBuf[32 * 2 + 2];
	uint8_t*	UsbSendPtr = 0;
	uint16_t	Len = 0;
	switch(Setup[3])
	{
		case USB_DT_DEVICE:
			UsbSendPtr = (uint8_t*)DeviceDescriptor;
			Len = sizeof(DeviceDescriptor);
			break;

		case USB_DT_CONFIG:
			UsbSendPtr = (uint8_t*)ConfigDescriptor;
            Len = UsbSendPtr[3];
            Len = Len<<8;
            Len = Len + UsbSendPtr[2];
			break;

		case USB_DT_STRING:
			if(Setup[2] == 0)			//lang ids
			{
				UsbSendPtr = DeviceString_LangID;
				Len = UsbSendPtr[0];
				break;
			}
			else if(Setup[2] == 1)		//manu
			{
				UsbSendPtr = (uint8_t*)gDeviceString_Manu;
			}
			else if(Setup[2] == 2)		//product
			{
				UsbSendPtr = (uint8_t*)gDeviceProductString;
			}
			else if(Setup[2] == 4)		//debug effect
			{
				UsbSendPtr = gDeviceString_Index;
			}			
			else 						//serial number
			{
				UsbSendPtr = (uint8_t*)gDeviceString_SerialNumber;
			}

			for(Len = 0; Len < 32; Len++)
			{
				if(UsbSendPtr[Len] == '\0')
				{
					break;
				}
				StringBuf[2 + Len * 2 + 0] = UsbSendPtr[Len];
				StringBuf[2 + Len * 2 + 1] = 0x00;
			}

			Len = Len * 2 + 2;
			StringBuf[0] = Len;
			StringBuf[1] = 0x03;
			UsbSendPtr = StringBuf;
			break;

		case USB_DT_INTERFACE:
			//PC默认不会发送该命令
			OTG_DBG("USB_DT_INTERFACE\n");
			break;

		case USB_DT_ENDPOINT:
			//PC默认不会发送该命令
			OTG_DBG("USB_DT_ENDPOINT\n");
			break;
			
		case USB_DT_DEVICE_QUALIFIER:
			UsbSendPtr = (uint8_t*)DeviceQualifier;
			Len = 10;
			break;

		case USB_HID_REPORT:
			if(Setup[4] == InterfaceNum[HID_DATA_INTERFACE_NUM])
			{
				OTG_DBG("HID_DATA_INTERFACE_NUM REPORTR\n");
				UsbSendPtr = (uint8_t*)&HidDataReportDescriptor[0];
				Len = sizeof(HidDataReportDescriptor);
			}
			else if(Setup[4] == InterfaceNum[HID_CTL_INTERFACE_NUM])
			{
				OTG_DBG("HID_CTL_INTERFACE_NUM REPORTR\n");
				UsbSendPtr = (uint8_t*)&AudioCtrlReportDescriptor[0];
				Len = sizeof(AudioCtrlReportDescriptor);
			}
			else
			{
				OTG_DBG("NOT FOUND INTERFACE %d\n",Setup[4]);
			}
			break;

		default:
			OTG_DBG("UsbDeviceSendStall:100\n");
			OTG_DeviceStallSend(DEVICE_CONTROL_EP);
			return;
	}

	if(Len > (Setup[7] * 256 + Setup[6]))
	{

		Len = Setup[7] * 256 + Setup[6];
	}
	OTG_DeviceControlSend((uint8_t*)UsbSendPtr, Len,3);
}
void OTG_DeviceAudioInit();
//extern uint32_t SpeakerRun;
void OTG_DeviceStandardRequest()
{
	uint8_t Resp[8];
	switch(Setup[1])
	{
		case USB_REQ_GET_STATUS:
			OTG_DBG("GetStatus\n");
			//用于获取USB设备接口端点的状态
			Resp[0] = 0x00;
			Resp[1] = 0x00;
			OTG_DeviceControlSend((uint8_t*)&Resp,2,10);
			break;

		case USB_REQ_CLEAR_FEATURE:
			OTG_DBG("ClearFeature\n");
			//用于清除或者禁止USB设备00,接口01,端点02,的某些特性，无数据传输
			break;

		case USB_REQ_SET_FEATURE:
			OTG_DBG("SetFeature\n");
			//用于设置或者使USB设备00,接口01,端点02,的某些特性，无数据传输
			OTG_DeviceStallSend(Setup[4]);
			break;

		case USB_REQ_SET_ADDRESS:
			OTG_DBG("SetAddress\n");
			OTG_DeviceAddressSet(Setup[2] & 0x7F);
			break;

		case USB_REQ_GET_DESCRIPTOR:
			OTG_DBG("GetDescriptor\n");
			OTG_DeviceGetDescriptor();
			break;
		
		case USB_REQ_SET_DESCRIPTOR:
			OTG_DBG("GetDescriptor\n");
			//DeviceGetDescriptor();
			break;

		case USB_REQ_GET_CONFIGURATION:
			OTG_DBG("GetConfiguration\n");
			Resp[0] = 0x01;
			//OtgDeviceControlSend(Resp, 1,3);
			OTG_DeviceControlSend((uint8_t*)&Resp, 1,3);
			break;

		case USB_REQ_SET_CONFIGURATION:
			OTG_DeviceEndpointReset(DEVICE_INT_IN_EP,TYPE_INT_IN);
			OTG_DeviceEndpointReset(DEVICE_BULK_IN_EP,TYPE_BULK_IN);
			OTG_DeviceEndpointReset(DEVICE_BULK_OUT_EP,TYPE_BULK_OUT);
			OTG_DeviceEndpointReset(DEVICE_ISO_IN_EP,TYPE_ISO_IN);
			OTG_DeviceEndpointReset(DEVICE_ISO_OUT_EP,TYPE_ISO_OUT);
#ifdef CFG_APP_USB_AUDIO_MODE_EN
			OTG_EndpointInterruptEnable(DEVICE_ISO_OUT_EP,OnDeviceAudioRcvIsoPacket);
			OTG_EndpointInterruptEnable(DEVICE_ISO_IN_EP,OnDeviceAudioSendIsoPacket);
#endif
			OTG_DeviceISOSend(DEVICE_ISO_IN_EP,0,0);
#ifdef CFG_APP_USB_AUDIO_MODE_EN
			OTG_DeviceAudioInit();
			UsbAudioMic.InitOk = 1;
			UsbAudioSpeaker.InitOk = 1;
#endif
			break;

		case USB_REQ_GET_INTERFACE:
			OTG_DBG("GetInterface\n");
			OTG_DeviceControlSend((uint8_t*)&Resp, 1,3);
			break;

		case USB_REQ_SET_INTERFACE:
			OTG_DBG("SetInterface %d\n",Setup[3]);
#ifdef CFG_APP_USB_AUDIO_MODE_EN
			if(Setup[4] == InterfaceNum[AUDIO_SRM_IN_INTERFACE_NUM])
			{
				OTG_DBG("mic %d",Setup[2]);
				if(UsbAudioMic.AltSet == FALSE)
					OtgMicFifoClear();
				UsbAudioMic.AltSet = Setup[2];
			}
			else if(Setup[4] == InterfaceNum[AUDIO_SRM_OUT_INTERFACE_NUM])
			{
				UsbAudioSpeaker.AltSet = Setup[2];
				OTG_DBG("speaker %d",Setup[2]);
			}
			else
#endif
			{
				OTG_DBG("aaaaa %d",Setup[2]);
			}
			break;

		case USB_REQ_SYNCH_FRAME:
			OTG_DBG("SYNC FRAME\n");
			break;

		default:
			OTG_DBG("UsbDeviceSendStall 006\n");
			OTG_DeviceStallSend(DEVICE_CONTROL_EP);
			break;
	}
}

//设备类请求
void OTG_DeviceClassRequest()
{
	//CLASS 根据接口来区分命令,每个接口下可能有多个命令，也有可能没有命令，根据实际该类的协议而定
	if(Setup[4] == InterfaceNum[MSC_INTERFACE_NUM])
	{
		OTG_DBG("MSC_INTERFACE_NUM\n");
		OTG_DeviceSendResp(0x0000, 1);
	}
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	else if(Setup[4] ==  InterfaceNum[AUDIO_ATL_INTERFACE_NUM])
	{
		OTG_DBG("AUDIO_ATL_INTERFACE_NUM\n");
		OTG_DeviceAudioRequest();
	}
#endif
	else if(Setup[4] == InterfaceNum[AUDIO_SRM_OUT_INTERFACE_NUM])
	{
		OTG_DBG("AUDIO_SRM_OUT_INTERFACE_NUM\n");
	}
	else if(Setup[4] == InterfaceNum[AUDIO_SRM_IN_INTERFACE_NUM])
	{
		OTG_DBG("AUDIO_SRM_IN_INTERFACE_NUM\n");
	}
	else if(Setup[4] == InterfaceNum[HID_CTL_INTERFACE_NUM])
	{
		OTG_DBG("HID_CTL_INTERFACE_NUM\n");
	}
	else if(Setup[4] == InterfaceNum[HID_DATA_INTERFACE_NUM])
	{
		//uint32_t len=0;
		//OTG_DBG("HID_DATA_INTERFACE_NUM\n");hid_send_data();
        /////标准命令
		if((Setup[3] == 0x02)&&(Setup[0] == 0x21))//out
		{
			hid_recive_data();
		}
		else if((Setup[3] == 0x01)&&(Setup[0] == 0xA1))//int
		{
			hid_send_data();
		}
		else if((Setup[3] == 0x03)&&(Setup[0] == 0xA1))//pc_upgrade
		{
			// OTG_DBG("pc_upgrade start 2\n");
			printf("pc_upgrade start 2\n");
			Setup[0] = 0x55;
			OTG_DeviceControlSend(Setup,Setup[7]*256+Setup[6],1);
			extern volatile uint32_t ota_upgrade_count;
			ota_upgrade_count = 6000;
		}
		else
		{
            OTG_DBG("Others Cmd\n");
		}		
	}
#ifdef CFG_APP_USB_AUDIO_MODE_EN
	else
	{
		OTG_DeviceAudioRequest();
	}
#endif
}


//厂商自定义数据处理。
void OTG_DeviceManufacturerRequest()
{
	//厂商自定义数据处理。
}


//未知命令
void OTG_DeviceOtherRequest()
{
	OTG_DBG("UsbDeviceSendStall\n");
	OTG_DeviceStallSend(DEVICE_CONTROL_EP);
}

#include "flash_boot.h"
/**
 * @brief  处理PC发来的控制命令
 * @param  NONE
 * @return NONE
 */
void OTG_DeviceRequestProcess(void)
{
	uint8_t BusEvent = OTG_DeviceBusEventGet();
	uint32_t DataLeng;
	uint8_t ReqType;
	if(BusEvent & 0x04)
	{
		OTG_DeviceAddressSet(0);
		//UsbAudioMic.InitOk = 0;
		//UsbAudioSpeaker.InitOk = 0;
	}
	if(OTG_DeviceSetupReceive(Setup, 8, &DataLeng) != DEVICE_NONE_ERR)
	{
		return;
	}
	//判断方向
	//如果是out 需要尽快接受数据,然后处理数据
	//如果是in，需要先准备数据，然后发送数据
	if((Setup[0]&0x80) == 0)//out
	{
		//判断数据长度是否有效。
		if((Setup[6] != 0) || (Setup[7] != 0))
		{
			if((Setup[0] == 0x21)&&(Setup[1] == 0x9))
			{

			}
			else
			{
				uint32_t temp=0;
				temp = Setup[7]*256 + Setup[6];
				OTG_DeviceControlReceive(Request,temp,&DataLeng,1);
			}
		}
	}
	//DBG("%02X %02X%02X%02X%02X%02X%02X%02X\n",Setup[0],Setup[1],Setup[2],Setup[3],Setup[4],Setup[5],Setup[6],Setup[7]);
	ReqType = (Setup[0]&0x60)>>5;
	//DBG("ReqType:%d\n",ReqType);
	switch(ReqType)
	{
		case 0:
			//标准请求
			OTG_DeviceStandardRequest();
			break;

		case 1:
			//类请求
			OTG_DeviceClassRequest();
			break;

		case 2:
			//厂商请求
			OTG_DeviceManufacturerRequest();
			break;

		case 3:
			//其他请求
			OTG_DeviceOtherRequest();
			break;			
	}
}
//*************************************************//
//*************************************************//
//*************************************************//
//*************************************************//
//*************************************************//
//*************************************************//
//*************************************************//


void HIDUsb_Tx(uint8_t *buf,uint16_t len)
{
#if 1//CFG_COMMUNICATION_BY_USB

     memset(hid_send,0,256);
	 
     memcpy(hid_send, buf, 256);
	 
#endif	 
}


void hid_recive_data()
{
	uint32_t len=0, i,j;

	memset(hid_buf, 0, 256);
	for(i=0;i<256/64;i++)
	{
		OTG_DeviceControlReceive(hid_buf+i*64,64,&len,6);
	}

//-------------------------------------------//
	
//------------------------------------------//	
#if 1//CFG_COMMUNICATION_BY_USB
	HIDUsb_Rx(hid_buf,256);
#endif
//------------------------------------------//
}


void hid_send_data(void)
{
	//DBG("hid_send_data: \n");

	OTG_DeviceControlSend(hid_send,256,6);
}

////
#endif /////end of if CFG_USB_EN
