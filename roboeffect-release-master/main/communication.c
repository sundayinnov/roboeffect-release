/**
 **************************************************************************************
 * @file    communication.c
 * @brief
 *
 * @author
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include <core_d1088.h>
#include "irqn.h"
#include "sys.h"
#include "clk.h"
#include "uarts.h"
#include "dma.h"
#include "audio_adc.h"
#include "dac.h"
#include "i2s.h"
#include "i2c.h"
#include "gpio.h"
#include "watchdog.h"
#include "timer.h"
#include "timeout.h"
#include "uarts_interface.h"
#include "audio_effect_library.h"
#include "roboeffect_api.h"
#include "communication.h"
#include "user_config.h"

#include "roboeffect_prot.h"
#include "user_effect_flow_demo.h"
#include "roboeffect_adapt.h"

#if CFG_COMMUNICATION_ENABLE

char AudioLibVer[] = AUDIO_EFFECT_LIBRARY_VERSION;
char RoboeffectLibVer[] = ROBOEFFECT_LIB_VER;

uint8_t  tx_buf[256] = {0};

extern uint8_t *context_memory;
extern uint32_t cpu_mips;
extern int16_t cpu_core_clk;

extern uint8_t  hid_rx_buf[256];
extern uint8_t  hid_tx_buf[256];

uint8_t need_update_status = 0x00;

extern roboeffect_effect_list_info *g_user_effect_list;
extern const roboeffect_effect_steps_table *g_user_effect_steps;
extern const unsigned char *g_user_effect_parameters;

extern bool need_switch_mode;

extern int32_t roboeffect_size, roboeffect_size_max, one_effect_size;



extern int connect_mode;
extern bool robo_init_err;

//for i2c
#define ROBO_I2C_SLAVE_ADDR 0x5A
#define ROBO_I2C_BUF_MAX_LEN 512
MemHandle iic_recv_handle;
uint16_t iic_rx_index = 0;
uint16_t iic_rx_len = 0x7fff;

extern uint8_t hid_tx_buf[256];

uint8_t robo_iic_recv_temp_buf[ROBO_I2C_BUF_MAX_LEN/2] = {0};
uint8_t robo_iic_recv_buf[ROBO_I2C_BUF_MAX_LEN] = {0};
uint32_t robo_iic_recv_offset = 0;
uint32_t robo_iic_send_offset = 0;


extern uint8_t s_tx_buf[512];
extern uint8_t s_rx_buf[512];

const uint32_t SupportSampleRateList[]={
	8000,
	11025,
	12000,
	16000,
	22050,
	24000,
	32000,
	44100,
	48000,
	//i2s//////////////////
	88200,
	96000,
	176400,
	192000,
};

const uint16_t HPCList[3] = {
	0xffe, //  Attenuation -1.5db at 20Hz at 48k sample rate.
	0xFFC, //  Attenuation -1.5db at 40Hz at 48k sample rate.
	0xFFD, //  Attenuation -1.5db at 40Hz at 32k sample rate.
};

#ifdef NEED_ENCRYPTED
bool is_encrypt_lock = FALSE;
#else
bool is_encrypt_lock = TRUE;
#endif

ControlVariablesContext gCtrlVars;

const roboeffect_adapt_device_table *g_adapter;
const roboeffect_adapt_device_node *adapt_get_item(const roboeffect_adapt_device_table *table, char *name);

int32_t comm_ret_sample_rate_enum(uint32_t samplerate)
{
	unsigned int i;
	for(i=0; i<(sizeof(SupportSampleRateList)/sizeof(uint32_t)); i++)
	{
		if(SupportSampleRateList[i] == samplerate)
			return i;
	}
	return 7;//default 44100
}

static int16_t *get_param_by_raw_data(uint8_t addr, uint8_t *parameters)
{
	uint8_t *ptr = parameters;//start from the first item

	while(((*ptr) >= 0x03) && ((*ptr) <= 0x0d))
	{
		if(*ptr == addr)
		{
			return (int16_t*)(ptr + 2);
		}
		else
		{
			ptr += *(ptr + 1) + 2;
		}
	}

	return NULL;
}


uint8_t  hid_rx_buf[256];
uint8_t  hid_tx_buf[256];

volatile uint32_t ota_upgrade_count = 0;

#define     ADR_FLASH_BOOT_FLAGE                       (0x4000100C)
typedef struct _ST_FLASH_BOOT_FLAGE {
	volatile  unsigned long UDisk                      :  1; /**< enable U盘 mode  */
	volatile  unsigned long PC                         :  1; /**< enable PC升级 mode  */
	volatile  unsigned long sdcard                     :  2; /**< enable SD卡 mode  */
	volatile  unsigned long updata                     :  1; /**< enable  */
	volatile  unsigned long flag                       :  2; /**<   */
	volatile  unsigned long RSV                        :  1; /**< 保留  */
	volatile  unsigned long ERROR_CODE                 :  8; /**< error code  */
	volatile  unsigned long POR_CODE                 :  8; /**< por code  */
} ST_FLASH_BOOT_FLAGE __ATTRIBUTE__(BITBAND);

#define SREG_FLASH_BOOT_FLAGE                    (*(volatile ST_FLASH_BOOT_FLAGE *) ADR_FLASH_BOOT_FLAGE)

extern void DataCacheInvalidAll(void);
static void boot_to_flashboot(void)
{
	typedef void (*fun)();
	fun jump_fun;
	int i;

	//ready to upgrade by PC tools
	*(uint32_t *)ADR_FLASH_BOOT_FLAGE = 0;

	SREG_FLASH_BOOT_FLAGE.PC = 1;//pc upgrade
	SREG_FLASH_BOOT_FLAGE.updata = 1;//upgrade flag
	SREG_FLASH_BOOT_FLAGE.UDisk = 1;//dummy
	SREG_FLASH_BOOT_FLAGE.sdcard = 3;//dummy

	if(SREG_FLASH_BOOT_FLAGE.updata)
	{
		printf("start_up_grate0...................\n");
		jump_fun = (fun)0;
		WDG_Enable(0);
		DisableIDCache();	//close cache
		DataCacheInvalidAll();//clear D cache
		SysTickDeInit();
		SysTimerIntFlagClear();
		GIE_DISABLE();
		
		//DMA
		for(i=0x40008000;i<0x40008124;)
		{
			*(uint32_t *)i = 0;
			i=i+4;
		}

		//0x40022000	REG_SEPA_RST_CTRL1
		//0x40022004	FUNC_SEPA_RST_CTRL1
		*(uint32_t *)0x40022000 &= ~0x7efffA;//REG reset
		*(uint32_t *)0x40022000 |= 0x7efffA;

		*(uint32_t *)0x40022004 &= ~0xffffbff;//fun reset
		*(uint32_t *)0x40022004 |= 0xffffbff;

		*(uint32_t *)0x40021034 &= ~0x7EFEFE0;//fun reset
		*(uint32_t *)0x40021038 &= ~0xff;//fun reset


		__nds32__mtsr(0, NDS32_SR_INT_MASK2);//中断使能位清零
		__nds32__mtsr(__nds32__mfsr(NDS32_SR_HSP_CTL) & 0, NDS32_SR_HSP_CTL);
		__asm("NOP");
		jump_fun();
		while(1);
	}
}

void run_upgrade_process(void)
{
	if(ota_upgrade_count > 0)
	{
		// DBG("O");
		ota_upgrade_count = ota_upgrade_count - 1;
		if(ota_upgrade_count <= 0)
		{
			printf("[OTA]: begin to switch to flashboot...\n");
			boot_to_flashboot();
		}
	}
}

void HIDUsb_Rx(uint8_t *buf,uint16_t len)
{
	roboeffect_prot_parse_big_block(buf, len);
}

void Communication_Effect_Send(uint8_t *buf, uint32_t len)
{

	if(connect_mode == MODE_HID)
	{
		memset(hid_tx_buf, 0x00, 256);
		memcpy(hid_tx_buf, buf, len);
	}
	else if(connect_mode == MODE_UART)
	{
		DMA_CircularDataPut(PERIPHERAL_ID_UART1_TX, buf, len);
	}
	else if(connect_mode == MODE_I2C)
	{
		memset(hid_tx_buf, 0x00, 256);

		if(len > 2)
		{
			//is a normal data
			memcpy(hid_tx_buf, buf+2, len-3);
			//du_iic_send_offset = len-3;
			robo_iic_send_offset = 0;//real offset
			//DBG("ss: %d\n", du_iic_send_offset);
		}
		else
		{
			//is a rsp
			memcpy(hid_tx_buf, buf, len);
			//du_iic_send_offset = len;
			robo_iic_send_offset = 0;//real offset
		}

	}
}

uint32_t Comm_Form_Rsp_VER_0x00(uint8_t *buffer)
{
	char *pp, *qq;

	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x00;//
	buffer[3]  = 0x0A + 1 + strlen(FIRMWARE_NAME) + 1 + 3;//!old struct + firmware name

#if defined(MINI_SYS_DUAL_BANK)
	buffer[4]  = 0xFE;//
#elif defined(MINI_SYS_FLASH_BOOT) 
	buffer[4]  = 0xFB;//
#else
	buffer[4]  = 0x40;//for new type of AP82 family, please refer to cmd 0x12
#endif

	buffer[5]  = ROBOEFFECT_VER_H;//
	buffer[6]  = ROBOEFFECT_VER_M;//
	buffer[7]  = ROBOEFFECT_VER_L;//

	pp = AudioLibVer;
	buffer[8] = atoi(pp);
	while((int)(pp-AudioLibVer) < (int)strlen(AudioLibVer) && *pp != '.'){pp++;}
	buffer[9] = atoi(++pp);
	while((int)(pp-AudioLibVer) < (int)strlen(AudioLibVer) && *pp != '.'){pp++;}
	buffer[10] = atoi(++pp);

	pp = RoboeffectLibVer;
	buffer[11] = atoi(pp);
	while((int)(pp-RoboeffectLibVer) < (int)strlen(RoboeffectLibVer) && *pp != '.'){pp++;}
	buffer[12] = atoi(++pp);
	while((int)(pp-RoboeffectLibVer) < (int)strlen(RoboeffectLibVer) && *pp != '.'){pp++;}
	buffer[13] = atoi(++pp);

	buffer[14] = strlen(FIRMWARE_NAME);//*firmware name length
	pp = &buffer[15];
	strncpy(pp, FIRMWARE_NAME, buffer[14]);
	pp += buffer[14];


	*pp++ = 0;//not any multi-graphic or parambin supported

	*pp++ = PARAMBIN_VER_H;
	*pp++ = PARAMBIN_VER_M;
	*pp++ = PARAMBIN_VER_L;

	*pp++ = 0x16;
	return buffer[3] + 5;
}

void Communication_Effect_0x00(uint8_t *buf, uint32_t len)
{
	//DBG("%s called. %d\n", __func__, len);
	if(len == 0)
	{
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_VER_0x00(tx_buf));
	}
	else
	{

	}
}


void Comm_SYS_0x01(int16_t index, int16_t data)
{
	switch(index)
	{
		case 0:////system mode {buf[1] =0 standby mode} {buf[1] =1 sleep mode} value= enum
			break;

		case 1:////system reset {buf[1] =0 standby mode} {buf[1] =1 reset mode} value= enum
			break;

		case 2:////system sample rate enable value= bool
			break;

		case 3:////system sample rate set value= enum
			break;

		case 4:///System MCLK source enable value= bool
			break;

		case 5:///System MCLK source select value= enum
			break;

		case 6:///System MCLK source select value= enum
			break;

		case 7:///System frame size
			// printf("frame size changed: %d\n", data);
			if(data > 512) data = 512;
			g_user_effect_list->frame_size = data;
			need_switch_mode = TRUE;

			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_SYS_0x01(uint8_t *buffer)
{
	int16_t *pp;

	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x01;
	buffer[3]  = 1+8*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = 0;
	*(pp++) = 0;
	*(pp++) = 1;
	*(pp++) = comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
	*(pp++) = 1;
	*(pp++) = 0;
	*(pp++) = 0x00;//
	*(pp++) = g_user_effect_list->frame_size;

	buffer[buffer[3] + 4] = 0x16;

	// printf("samplerate: %d\n", comm_ret_sample_rate_enum(g_user_effect_list->sample_rate));

	return buffer[3] + 5;
}

void Communication_Effect_0x01(uint8_t *buf, uint32_t len)//SYS Param
{
	int16_t *pp, i;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_SYS_0x01(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<8; i++, pp++)
			{
				Comm_SYS_0x01(i, *pp);
			}
		}
		else
		{
			//DBG("PGA0_0x03: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_SYS_0x01(buf[0], *((int16_t*)&buf[1]));
		}

		{
			uint8_t rsp_byte = 0x01;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

#if 0
void Communication_Effect_0x02(void)///systme ram
{
	static uint8_t first_bool = 0x01;
	uint32_t UsedRamSize = 0;
	int16_t cpu_core_clk = Clock_PllFreqGet()/2000000;

	memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x02;
	tx_buf[3]  = 2 + 5*2;//
	tx_buf[4]  = 0xff;

	if(context_memory)
		UsedRamSize = (roboeffect_size_max - roboeffect_get_free_memory_space(context_memory))/1024;
	else
		UsedRamSize = 0;
	
	memcpy(&tx_buf[5], &UsedRamSize, 2);
	memcpy(&tx_buf[7], &cpu_mips, 2);

	tx_buf[9] = first_bool | need_update_status;

	memcpy(&tx_buf[10], &cpu_core_clk, 2);

	UsedRamSize = roboeffect_size_max/1024;
	memcpy(&tx_buf[12], &UsedRamSize, 2);

	// memcpy(&tx_buf[14], &(g_user_effect_list->frame_size), 2);
	if(robo_init_err)
		cpu_core_clk = ROBOEFFECT_EFFECT_INIT_FAILED;
	else if(context_memory)
		cpu_core_clk = roboeffect_get_error_code(context_memory);
	else
		cpu_core_clk = ROBOEFFECT_CONTEXT_MEMORY_ERROR;


	memcpy(&tx_buf[14], &cpu_core_clk, 2);

	tx_buf[16] = 0x16;
	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);

	//force to update status only once
	first_bool = 0x00;
	need_update_status = 0x00;

	// DBG("%d ", roboeffect_get_error_code(context_memory));
}
#else

#define ROBO_0X02_DATA_NUM 5
#pragma pack(1)
typedef struct _robo_0x02_data
{
	//system memory info
	uint8_t sys_mem_info_num;
	uint8_t sys_mem_info_len;

	uint8_t sys_mem_id; //block id for multi-sram, here only one sram
	uint16_t current_mem_used;
	uint16_t max_mem_size;

	//system cpu info
	uint8_t sys_cpu_info_num;
	uint8_t sys_cpu_info_len;

	uint8_t cpu_id;//here only one CPU
	uint16_t cpu_mcps_frq;
	uint16_t cpu_max_frq;

	//roboeffect memory info
	uint8_t robo_mem_info_num;
	uint8_t robo_mem_info_len;

	uint8_t robo_mem_id;//here only one roboeffect
	uint16_t robo_current_mem;
	uint16_t robo_max_mem;

	//roboeffect error code info
	uint8_t robo_err_code_info_num;
	uint8_t robo_err_code_info_len;

	uint8_t robo_err_id;//here only one roboeffect
	int16_t robo_main_err_code;
	uint8_t robo_err_effect_addr;
	int16_t robo_err_effect_code;

	//flush info
	uint8_t flush_code_info_num;
	uint8_t flush_code_info_len;

	uint8_t reserved;
	uint8_t flush_code;
	uint8_t flush_param_code;

} robo_0x02_data;
#pragma pack()

extern uint32_t _end;
void Communication_Effect_0x02(void)///systme ram
{
	static uint8_t first_bool = 0x01, check_sum = 0, *check_ptr;
	uint32_t UsedRamSize = 0;
	int16_t cpu_core_clk = Clock_PllFreqGet()/2000000;
	int16_t robo_err_code;
	roboeffect_effect_err robo_effect_err = {0};

	memset(tx_buf, 0, sizeof(tx_buf));

	tx_buf[0]  = 0xa5;
	tx_buf[1]  = 0x5a;
	tx_buf[2]  = 0x02;
	tx_buf[3]  = 1 + 1 + sizeof(robo_0x02_data) + 1;//
	tx_buf[4]  = 0xfe; //use new protocol with FE
	tx_buf[5]  = ROBO_0X02_DATA_NUM;

	if(context_memory)
		UsedRamSize = (roboeffect_size_max - roboeffect_get_free_memory_space(context_memory))/1024;
	else
		UsedRamSize = 0;

	if(robo_init_err)
		robo_err_code = ROBOEFFECT_EFFECT_INIT_FAILED;
	else if(context_memory)
	{
		robo_err_code = roboeffect_get_error_code(context_memory);
		roboeffect_get_error_code_extended(context_memory, &robo_effect_err);
	}
	else
		robo_err_code = ROBOEFFECT_CONTEXT_MEMORY_ERROR;

	robo_0x02_data *data_ptr = (robo_0x02_data*)&tx_buf[6];

	data_ptr->sys_mem_info_num = 0x01;
	data_ptr->sys_mem_info_len = 5;
	data_ptr->sys_mem_id = 1;
	data_ptr->current_mem_used = (((uint32_t)&_end-(uint32_t)0x20000000) + roboeffect_size_max)/1024;
	data_ptr->max_mem_size = 224;

	data_ptr->sys_cpu_info_num = 0x02;
	data_ptr->sys_cpu_info_len = 5;
	data_ptr->cpu_id = 1;
	data_ptr->cpu_mcps_frq = cpu_mips;
	data_ptr->cpu_max_frq = cpu_core_clk;

	data_ptr->robo_mem_info_num = 0x03;
	data_ptr->robo_mem_info_len = 5;
	data_ptr->robo_mem_id = 1;
	data_ptr->robo_current_mem = UsedRamSize;
	data_ptr->robo_max_mem = ((uint32_t)0x20037FFF - (uint32_t)&_end)/1024;//roboeffect_size_max/1024;

	data_ptr->robo_err_code_info_num = 0x04;
	data_ptr->robo_err_code_info_len = 6;
	data_ptr->robo_err_id = 1;
	data_ptr->robo_main_err_code = robo_err_code;
	data_ptr->robo_err_effect_addr = robo_effect_err.err_effect_addr;
	data_ptr->robo_err_effect_code = robo_effect_err.err_effect_code;

	// printf("%02X, %d\n", robo_effect_err.err_effect_addr, robo_effect_err.err_effect_code);

	data_ptr->flush_code_info_num = 0x05;
	data_ptr->flush_code_info_len = 0x03;

	data_ptr->reserved = 0;
	if(first_bool)
	{
		data_ptr->flush_code = 0x01;//force to flush all
		data_ptr->flush_param_code = 0;
	}
	else if(need_update_status < 0x03)
	{
		data_ptr->flush_code = need_update_status;
		data_ptr->flush_param_code = 0;
	}
	else
	{
		data_ptr->flush_code = 0x03;
		data_ptr->flush_param_code = need_update_status;
	}

	for(check_ptr = &tx_buf[4]; check_ptr < &tx_buf[4 + tx_buf[3]] - 1; check_ptr++)
	{
		// printf("%02X\n", *check_ptr);
		check_sum += *check_ptr;
	}

	tx_buf[3 + tx_buf[3]] = check_sum;
	tx_buf[4 + tx_buf[3]] = 0x16;
	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);

	//force to update status only once
	first_bool = 0x00;
	need_update_status = 0x00;

	// DBG("%d ", roboeffect_get_error_code(context_memory));
}
#endif




bool roboeffect_effect_update_params_entrance(uint8_t addr, uint8_t *buf, uint32_t len)
{
	int i;
	ROBOEFFECT_ERROR_CODE ret;

	if(context_memory == NULL)
		return FALSE;

	tx_buf[0] = 0xA5;
	tx_buf[1] = 0x5A;
	tx_buf[2] = addr;
	if(len == 0)//require parameters
	{
		const int16_t *params;
		if((params = roboeffect_get_effect_parameter(context_memory, addr, 0xff)) != NULL)
		{
			int16_t len = roboeffect_get_effect_parameter_count(context_memory, addr);
			if(len >= 0)//= 0 when only enable/disable existed
			{
				int16_t *pp = (int16_t*)&tx_buf[5];//out parameter 0 is enable/disable

				tx_buf[3] = (len + 1) * 2 + 1;
				tx_buf[4] = 0xff;

				*(pp++) = roboeffect_get_effect_status(context_memory, addr);//on/off
				
				memcpy(pp, params, len * 2);//parameters
				pp += len;

				tx_buf[4 + (len + 1) * 2 + 1] = 0x16;
				Communication_Effect_Send(tx_buf,  tx_buf[3] + 5);
			}
			else
			{
				return FALSE;
			}
		}
		else
		{
			return FALSE;
		}
	}
	else//set parameters
	{   
    if(buf[0] == 0xff)//all parameters
    {
			int16_t enable = buf[1];//[1] is enable/disable
			int16_t *params = (int16_t*)&buf[3];//[3] is parameters beginning
			
			roboeffect_set_effect_parameter(context_memory, addr, 0xff, params);
			roboeffect_enable_effect(context_memory, addr, enable);
		}
		else//one parameter
		{
			if(buf[0] == 0)//only enable/disable
			{
				int16_t enable = buf[1];//[1] is enable/disable

				ret = roboeffect_enable_effect(context_memory, addr, enable);
				// printf("en/dis = %d, %d:%d\n", ret, g_user_effect_list->frame_size, roboeffect_recommend_frame_size_upon_effect_change(context_memory, g_user_effect_list->frame_size, addr, (enable)?(1):(-1)));
			}
			else//other parameter, only one parameter configurated
			{
				int16_t *params = (int16_t*)&buf[1];//[1] is parameter
				int16_t index = buf[0] - 1;//[0] - 1 is parameter index

				// printf("%02X: %d, %d\n", addr, index, params[0]);
				roboeffect_set_effect_parameter(context_memory, addr, index, params);
			}
		}

    Communication_Effect_Send(&addr, 1);

	}

	return TRUE;
}

//CC_TODO: hardware control panel
/********************************hardware control panel********************************/

static void Reset_AGC_ChannelSel(ADC_MODULE ADCModule, uint32_t mode)
{
	AudioADC_AGCChannelSel(ADCModule, 0, 0);
	AudioADC_AGCChannelSel(ADCModule, !!(mode & 0x01), !!(mode & 0x02));
}


//CC_TODO: load_hardware_fr_default
void load_hardware_fr_default(void)
{
	const roboeffect_adapt_device_node *device_node;

	//PGA0 setting
	gCtrlVars.pga0_r_channel = PGA0_INPUT_MIC3;
	gCtrlVars.pga0_l_channel = PGA0_INPUT_MIC4;

	gCtrlVars.pga0_r_gain = PGA_GAIN;
	gCtrlVars.mic3_r_gain = 63 - PGA_GAIN;
	gCtrlVars.pga0_l_gain = PGA_GAIN;
	gCtrlVars.mic4_l_gain = 63 - PGA_GAIN;

	gCtrlVars.pga0_r_boost = PGA_GAIN_BOOST;
	gCtrlVars.pga0_l_boost = PGA_GAIN_BOOST;

	if(gCtrlVars.pga0_r_boost == 4)
		gCtrlVars.mic3_r_boost = 0;
	else
		gCtrlVars.mic3_r_boost = gCtrlVars.pga0_r_boost + 1;
	if(gCtrlVars.pga0_l_boost == 4)
		gCtrlVars.mic4_l_boost = 0;
	else
		gCtrlVars.mic4_l_boost = gCtrlVars.pga0_l_boost + 1;


	gCtrlVars.pga0_zero_cross = 0;
	gCtrlVars.pga0_mode = 0;

	gCtrlVars.pga0_diff_gain_l = 0;
	gCtrlVars.pga0_diff_gain_r = 0;


	//ADC0 setting
	gCtrlVars.adc0_dig_channel_en = 1;//always on
	gCtrlVars.adc0_dig_mute = 0;
	gCtrlVars.adc0_dig_l_mute = 0;
	gCtrlVars.adc0_dig_r_mute = 0;
	gCtrlVars.adc0_dig_l_vol = 0x1000;
	gCtrlVars.adc0_dig_r_vol = 0x1000;

	gCtrlVars.adc0_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.adc0_lr_swap = 0;
	gCtrlVars.adc0_dc_blocker = 0;
	gCtrlVars.adc0_fade_time = 0;
#ifdef EXTERNAL_CLK
	gCtrlVars.adc0_mclk_src = 4;
#else
	gCtrlVars.adc0_mclk_src = 0;
#endif
	gCtrlVars.adc0_dc_blocker_en = TRUE;

	//AGC0 setting
	gCtrlVars.agc0_mode = 0;

	gCtrlVars.agc0_max_level = AGC_MAXLEVEL;
	gCtrlVars.agc0_target_level = AGC_TARGETLEVEL;
	gCtrlVars.agc0_max_gain = 63 - AGC_MAX_GAIN;
	gCtrlVars.agc0_min_gain = 63 - AGC_MIN_GAIN;
	gCtrlVars.agc0_gainoffset = AGC_GAIN_OFFSET;

	gCtrlVars.agc0_fram_time = AGC_FRAME_TIME;
	gCtrlVars.agc0_hold_frames = AGC_HOLD_TIME;
	gCtrlVars.agc0_attack_time = AGC_ATTACK_STEPTIME;
	gCtrlVars.agc0_decay_time = AGC_DECAY_STEPTIME;
	gCtrlVars.agc0_noise_gate_en = AGC_NS_GATE_ENABLE;
	gCtrlVars.agc0_noise_threshold = AGC_NS_THRESHOLD;
	gCtrlVars.agc0_noise_gate_mode = AGC_NS_MODE;
	gCtrlVars.agc0_noise_hold_frames = AGC_NS_HOLD_TIME;

/*----------------------------------------------------------------------------------------------------------*/

	//PGA1 setting
	gCtrlVars.pga1_r_channel = PGA1_INPUT_LINEIN2_RIGHT;
	gCtrlVars.pga1_l_channel = PGA1_INPUT_LINEIN2_LEFT;

	gCtrlVars.pga1_r_gain = PGA_GAIN_MUSIC;
	gCtrlVars.line2_1_r_gain = 63 - PGA_GAIN_MUSIC;
	gCtrlVars.pga1_l_gain = PGA_GAIN_MUSIC;
	gCtrlVars.line2_1_l_gain = 63 - PGA_GAIN_MUSIC;

	gCtrlVars.fm_r_gain = 63 - PGA_GAIN_MUSIC;
	gCtrlVars.fm_l_gain = 63 - PGA_GAIN_MUSIC;

	gCtrlVars.pga1_r_boost = PGA_GAIN_MUSIC_BOOST;
	gCtrlVars.pga1_l_boost = PGA_GAIN_MUSIC_BOOST;


	gCtrlVars.pga1_zero_cross = 0;
	gCtrlVars.pga1_mode = 0;

	// gCtrlVars.pga1_diff_gain_l = 0;
	// gCtrlVars.pga1_diff_gain_r = 0;


	//ADC1 setting
	gCtrlVars.adc1_dig_channel_en = 1;//always on
	gCtrlVars.adc1_dig_mute = 0;
	gCtrlVars.adc1_dig_l_mute = 0;
	gCtrlVars.adc1_dig_r_mute = 0;
	gCtrlVars.adc1_dig_l_vol = 0x1000;
	gCtrlVars.adc1_dig_r_vol = 0x1000;

	gCtrlVars.adc1_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.adc1_lr_swap = 0;
	gCtrlVars.adc1_dc_blocker = 0;
	gCtrlVars.adc1_fade_time = 0;

#ifdef EXTERNAL_CLK
	gCtrlVars.adc1_mclk_src = 4;
#else
	gCtrlVars.adc1_mclk_src = 0;
#endif
	gCtrlVars.adc1_dc_blocker_en = TRUE;


	//AGC0 setting
	gCtrlVars.agc1_mode = 0;
	gCtrlVars.agc1_max_level = AGC_MAXLEVEL;
	gCtrlVars.agc1_target_level = AGC_TARGETLEVEL;
	gCtrlVars.agc1_max_gain = 63 - AGC_MAX_GAIN;
	gCtrlVars.agc1_min_gain = 63 - AGC_MIN_GAIN;
	gCtrlVars.agc1_gainoffset = AGC_GAIN_OFFSET;

	gCtrlVars.agc1_fram_time = AGC_FRAME_TIME;
	gCtrlVars.agc1_hold_frames = AGC_HOLD_TIME;
	gCtrlVars.agc1_attack_time = AGC_ATTACK_STEPTIME;
	gCtrlVars.agc1_decay_time = AGC_DECAY_STEPTIME;
	gCtrlVars.agc1_noise_gate_en = AGC_NS_GATE_ENABLE;
	gCtrlVars.agc1_noise_threshold = AGC_NS_THRESHOLD;
	gCtrlVars.agc1_noise_gate_mode = AGC_NS_MODE;
	gCtrlVars.agc1_noise_hold_frames = AGC_NS_HOLD_TIME;

	//DAC0
	gCtrlVars.dac0_en = 1;//always on
	gCtrlVars.dac0_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.dac0_dig_mute = 0;
	gCtrlVars.dac0_dig_l_mute = 0;
	gCtrlVars.dac0_dig_r_mute = 0;
	gCtrlVars.dac0_dig_l_vol = 0x1000;
	gCtrlVars.dac0_dig_r_vol = 0x1000;

	gCtrlVars.dac0_dither = 0;
	gCtrlVars.dac0_scramble = 0;
	gCtrlVars.dac0_out_mode = 0;
	gCtrlVars.dac0_sample_mode = 0;
	gCtrlVars.dac0_scf_mute = 0;
	gCtrlVars.dac0_fade_time = 0;
	gCtrlVars.dac0_zeros_number = 0;

#ifdef EXTERNAL_CLK
	gCtrlVars.dac0_mclk_src = 4;
#else
	gCtrlVars.dac0_mclk_src = 1;
#endif

	//DAC1
	gCtrlVars.dac1_en = 1;//always on
	gCtrlVars.dac1_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.dac1_dig_mute = 0;
	gCtrlVars.dac1_dig_vol = 0x1000;

	gCtrlVars.dac1_dither = 0;
	gCtrlVars.dac1_scramble = 0;
	gCtrlVars.dac1_out_mode = 0;
	gCtrlVars.dac1_sample_mode = 0;
	gCtrlVars.dac1_scf_mute = 0;
	gCtrlVars.dac1_fade_time = 0;
	gCtrlVars.dac1_zeros_number = 0;
#ifdef EXTERNAL_CLK
	gCtrlVars.dac1_mclk_src = 4;
#else
	gCtrlVars.dac1_mclk_src = 0;
#endif


	//I2S0
	gCtrlVars.i2s0_tx_en = 1;
	gCtrlVars.i2s0_rx_en = 0;
	gCtrlVars.i2s0_sample_rate = 48000;//g_user_effect_list->sample_rate;
#ifdef EXTERNAL_CLK
	gCtrlVars.i2s0_mclk_src = 4;
	gCtrlVars.i2s0_work_mode = 1;
#else
	gCtrlVars.i2s0_mclk_src = 1;
	gCtrlVars.i2s0_work_mode = 0;
#endif

	gCtrlVars.i2s0_format = I2S_FORMAT_I2S;
	if((device_node = adapt_get_item(g_adapter, "SOURCE_I2S0_RX")) != NULL || (device_node = adapt_get_item(g_adapter, "SINK_I2S0_TX")) != NULL)
		gCtrlVars.i2s0_word_len = (device_node->width == BITS_16)?(I2S_LENGTH_16BITS):(I2S_LENGTH_24BITS);
	else
		gCtrlVars.i2s0_word_len = I2S_LENGTH_32BITS;
	
	gCtrlVars.i2s0_mono = FALSE;
	gCtrlVars.i2s0_fade_time = 0;
	gCtrlVars.i2s0_bclk_invert_en = FALSE;
	gCtrlVars.i2s0_lrclk_invert_en = FALSE;

	//I2S1
	gCtrlVars.i2s1_tx_en = 1;
	gCtrlVars.i2s1_rx_en = 1;
	gCtrlVars.i2s1_sample_rate = 48000;//g_user_effect_list->sample_rate;
#ifdef EXTERNAL_CLK
	gCtrlVars.i2s1_mclk_src = 4;
	gCtrlVars.i2s1_work_mode = 1;
#else
	gCtrlVars.i2s1_mclk_src = 1;
	gCtrlVars.i2s1_work_mode = 1;
#endif
	gCtrlVars.i2s1_format = I2S_FORMAT_I2S;
	if((device_node = adapt_get_item(g_adapter, "SOURCE_I2S1_RX")) != NULL || (device_node = adapt_get_item(g_adapter, "SINK_I2S1_TX")) != NULL)
		gCtrlVars.i2s1_word_len = (device_node->width == BITS_16)?(I2S_LENGTH_16BITS):(I2S_LENGTH_24BITS);
	else
		gCtrlVars.i2s1_word_len = I2S_LENGTH_16BITS;

	gCtrlVars.i2s1_mono = FALSE;
	gCtrlVars.i2s1_fade_time = 0;
	gCtrlVars.i2s1_bclk_invert_en = TRUE;
	gCtrlVars.i2s1_lrclk_invert_en = FALSE;

	//SPDIF
	gCtrlVars.spdif_en = 0;
	gCtrlVars.spdif_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.spdif_ch_mode = 0;
	gCtrlVars.spdif_io_sel = 0;

}

//CC_TODO: load_hardware_fr_params_raw
void load_hardware_fr_params_raw(uint8_t *parameters)
{
	int16_t *param_ptr;
	const roboeffect_adapt_device_node *device_node;

	//PGA0 setting
	if((param_ptr = get_param_by_raw_data(0x03, parameters)) == NULL)
		return;

	gCtrlVars.pga0_r_channel = 0;
	gCtrlVars.pga0_l_channel = 0;
	if(param_ptr[6]) gCtrlVars.pga0_l_channel |= PGA0_INPUT_LINEIN1_LEFT;
	if(param_ptr[7]) gCtrlVars.pga0_r_channel |= PGA0_INPUT_LINEIN1_RIGHT;
	if(param_ptr[8]) gCtrlVars.pga0_l_channel |= PGA0_INPUT_MIC4;
	if(param_ptr[9]) gCtrlVars.pga0_r_channel |= PGA0_INPUT_MIC3;
	if(param_ptr[10]) gCtrlVars.pga0_l_channel |= PGA0_INPUT_LINEIN2_LEFT;
	if(param_ptr[11]) gCtrlVars.pga0_r_channel |= PGA0_INPUT_LINEIN2_RIGHT;

	gCtrlVars.line1_l_gain = param_ptr[12];
	gCtrlVars.line1_r_gain = param_ptr[13];
	gCtrlVars.mic4_l_gain = param_ptr[14];
	gCtrlVars.mic3_r_gain =	param_ptr[15];
	gCtrlVars.line2_0_l_gain = param_ptr[16];
	gCtrlVars.line2_0_r_gain = param_ptr[17];

	gCtrlVars.pga0_r_gain = 63 - param_ptr[9];//MIC3
	gCtrlVars.pga0_l_gain = 63 - param_ptr[8];//MIC4

	gCtrlVars.mic3_r_boost = param_ptr[19];
	gCtrlVars.mic4_l_boost = param_ptr[18];

	if(gCtrlVars.mic3_r_boost == 0)
		gCtrlVars.pga0_r_boost = 4;
	else
		gCtrlVars.pga0_r_boost = gCtrlVars.mic3_r_boost - 1;
	if(gCtrlVars.mic4_l_boost == 0)
		gCtrlVars.pga0_l_boost = 4;
	else
		gCtrlVars.pga0_l_boost = gCtrlVars.mic4_l_boost - 1;

	// printf("conv: %d, %d\n", )

	gCtrlVars.pga0_zero_cross = 0;
	gCtrlVars.pga0_mode = param_ptr[20];

	gCtrlVars.pga0_diff_gain_l = param_ptr[21];
	gCtrlVars.pga0_diff_gain_r = param_ptr[22];

#ifdef EXTERNAL_CLK
	gCtrlVars.adc0_mclk_src = 4;
#else
	gCtrlVars.adc0_mclk_src = 0;
#endif

	//ADC0 setting
	if((param_ptr = get_param_by_raw_data(0x04, parameters)) == NULL)
		return;
	
	gCtrlVars.adc0_dig_channel_en = 1;//always on
	gCtrlVars.adc0_dig_mute = param_ptr[1];
	gCtrlVars.adc0_dig_l_mute = !!(gCtrlVars.adc0_dig_mute & 0x01);;
	gCtrlVars.adc0_dig_r_mute = !!(gCtrlVars.adc0_dig_mute & 0x02);
	gCtrlVars.adc0_dig_l_vol = param_ptr[2];
	gCtrlVars.adc0_dig_r_vol = param_ptr[3];

	gCtrlVars.adc0_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.adc0_lr_swap = param_ptr[5];
	gCtrlVars.adc0_dc_blocker = param_ptr[6];
	gCtrlVars.adc0_fade_time = param_ptr[7];
#ifdef EXTERNAL_CLK
	gCtrlVars.adc0_mclk_src = 4;
#else
	gCtrlVars.adc0_mclk_src = 0;
#endif
	gCtrlVars.adc0_dc_blocker_en = param_ptr[9];


	//AGC0 setting
	if((param_ptr = get_param_by_raw_data(0x05, parameters)) == NULL)
		return;

	//AGC0 setting
	gCtrlVars.agc0_mode = (AGC_ON)?(2):(0);

	gCtrlVars.agc0_max_level = param_ptr[1];
	gCtrlVars.agc0_target_level = param_ptr[2];
	gCtrlVars.agc0_max_gain = param_ptr[3];
	gCtrlVars.agc0_min_gain = param_ptr[4];
	gCtrlVars.agc0_gainoffset = param_ptr[5];

	gCtrlVars.agc0_fram_time = param_ptr[6];
	gCtrlVars.agc0_hold_frames = param_ptr[7];
	gCtrlVars.agc0_attack_time = param_ptr[8];
	gCtrlVars.agc0_decay_time = param_ptr[9];
	gCtrlVars.agc0_noise_gate_en = param_ptr[10];
	gCtrlVars.agc0_noise_threshold = param_ptr[11];
	gCtrlVars.agc0_noise_gate_mode = param_ptr[12];
	gCtrlVars.agc0_noise_hold_frames = param_ptr[13];

/*----------------------------------------------------------------------------------------------------------*/

	//PGA1 setting
	if((param_ptr = get_param_by_raw_data(0x06, parameters)) == NULL)
		return;

	gCtrlVars.pga1_r_channel = 0;
	gCtrlVars.pga1_l_channel = 0;
	if(param_ptr[8]) gCtrlVars.pga1_l_channel |= PGA1_INPUT_LINEIN3_LEFT;
	if(param_ptr[9]) gCtrlVars.pga1_r_channel |= PGA1_INPUT_LINEIN3_RIGHT;
	if(param_ptr[10]) gCtrlVars.pga1_l_channel |= PGA1_INPUT_MIC2;
	if(param_ptr[11]) gCtrlVars.pga0_r_channel |= PGA1_INPUT_MIC1;
	if(param_ptr[12]) gCtrlVars.pga1_l_channel |= PGA1_INPUT_LINEIN2_LEFT;
	if(param_ptr[13]) gCtrlVars.pga1_r_channel |= PGA1_INPUT_LINEIN2_RIGHT;
	if(param_ptr[14]) gCtrlVars.pga1_l_channel |= PGA1_INPUT_FMIN_LEFT;
	if(param_ptr[15]) gCtrlVars.pga1_r_channel |= PGA1_INPUT_FMIN_RIGHT;


	gCtrlVars.line3_l_gain = param_ptr[16];
	gCtrlVars.line3_r_gain = param_ptr[17];

	gCtrlVars.mic2_l_gain = param_ptr[18];
	gCtrlVars.mic1_r_gain = param_ptr[19];

	gCtrlVars.line2_1_l_gain = param_ptr[20];
	gCtrlVars.line2_1_r_gain = param_ptr[21];

	gCtrlVars.fm_l_gain = param_ptr[22];
	gCtrlVars.fm_r_gain = param_ptr[23];


	gCtrlVars.pga1_l_gain = 63 - param_ptr[20];//linein2
	gCtrlVars.pga1_r_gain = 63 - param_ptr[21];//linein2

	gCtrlVars.mic2_l_boost = param_ptr[24];//MIC2
	gCtrlVars.mic1_r_boost = param_ptr[25];//MIC1
	if(gCtrlVars.mic1_r_boost == 0)
		gCtrlVars.pga1_r_boost = 4;
	else
		gCtrlVars.pga1_r_boost = gCtrlVars.mic1_r_boost - 1;
	if(gCtrlVars.mic2_l_boost == 0)
		gCtrlVars.pga1_l_boost = 4;
	else
		gCtrlVars.pga1_l_boost = gCtrlVars.mic2_l_boost - 1;

	gCtrlVars.pga1_zero_cross = 0;
	gCtrlVars.pga1_mode = 0;

	// gCtrlVars.pga1_diff_gain_l = 0;
	// gCtrlVars.pga1_diff_gain_r = 0;


	//ADC1 setting
	if((param_ptr = get_param_by_raw_data(0x07, parameters)) == NULL)
		return;

	gCtrlVars.adc1_dig_channel_en = 1;//always on
	gCtrlVars.adc1_dig_mute = param_ptr[1];
	gCtrlVars.adc1_dig_l_mute = !!(gCtrlVars.adc1_dig_mute & 0x01);;
	gCtrlVars.adc1_dig_r_mute = !!(gCtrlVars.adc1_dig_mute & 0x02);
	gCtrlVars.adc1_dig_l_vol = param_ptr[2];
	gCtrlVars.adc1_dig_r_vol = param_ptr[3];

	gCtrlVars.adc1_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.adc1_lr_swap = param_ptr[5];
	gCtrlVars.adc1_dc_blocker = param_ptr[6];
	gCtrlVars.adc1_fade_time = param_ptr[7];
#ifdef EXTERNAL_CLK
	gCtrlVars.adc1_mclk_src = 4;
#else
	gCtrlVars.adc1_mclk_src = 0;
#endif
	gCtrlVars.adc1_dc_blocker_en = param_ptr[9];


	//AGC0 setting
	if((param_ptr = get_param_by_raw_data(0x08, parameters)) == NULL)
		return;

	gCtrlVars.agc1_mode = param_ptr[0];
	gCtrlVars.agc1_max_level = param_ptr[1];
	gCtrlVars.agc1_target_level = param_ptr[2];
	gCtrlVars.agc1_max_gain = param_ptr[3];
	gCtrlVars.agc1_min_gain = param_ptr[4];
	gCtrlVars.agc1_gainoffset = param_ptr[5];

	gCtrlVars.agc1_fram_time = param_ptr[6];
	gCtrlVars.agc1_hold_frames = param_ptr[7];
	gCtrlVars.agc1_attack_time = param_ptr[8];
	gCtrlVars.agc1_decay_time = param_ptr[9];
	gCtrlVars.agc1_noise_gate_en = param_ptr[10];
	gCtrlVars.agc1_noise_threshold = param_ptr[11];
	gCtrlVars.agc1_noise_gate_mode = param_ptr[12];
	gCtrlVars.agc1_noise_hold_frames = param_ptr[13];

	//DAC0
	if((param_ptr = get_param_by_raw_data(0x09, parameters)) == NULL)
		return;

	gCtrlVars.dac0_en = 1;//always on
	gCtrlVars.dac0_dig_mute = param_ptr[2];
	gCtrlVars.dac0_dig_l_mute = !!(gCtrlVars.dac0_dig_mute & 0x01);
	gCtrlVars.dac0_dig_r_mute = !!(gCtrlVars.dac0_dig_mute & 0x02);
	gCtrlVars.dac0_dig_l_vol = param_ptr[3];
	gCtrlVars.dac0_dig_r_vol = param_ptr[4];

	gCtrlVars.dac0_dither = param_ptr[5];
	gCtrlVars.dac0_scramble = param_ptr[6];
	gCtrlVars.dac0_out_mode = param_ptr[7];
	gCtrlVars.dac0_sample_mode = param_ptr[8];
	gCtrlVars.dac0_scf_mute = param_ptr[10];
	gCtrlVars.dac0_fade_time = param_ptr[11];
	gCtrlVars.dac0_zeros_number = param_ptr[12];

#ifdef EXTERNAL_CLK
	gCtrlVars.dac0_mclk_src = 4;
#else
	gCtrlVars.dac0_mclk_src = 0;
#endif


	//DAC1
	if((param_ptr = get_param_by_raw_data(0x0A, parameters)) == NULL)
		return;

	gCtrlVars.dac1_en = 1;//always on
	gCtrlVars.dac1_dig_mute = param_ptr[2];
	gCtrlVars.dac1_dig_vol = param_ptr[3];

	gCtrlVars.dac1_dither = param_ptr[5] << 7;
	gCtrlVars.dac1_scramble = param_ptr[6];
	gCtrlVars.dac1_out_mode = param_ptr[7];
	gCtrlVars.dac1_sample_mode = param_ptr[8];
	gCtrlVars.dac1_scf_mute = param_ptr[10];
	gCtrlVars.dac1_fade_time = param_ptr[11];
	gCtrlVars.dac1_zeros_number = param_ptr[12];
#ifdef EXTERNAL_CLK
	gCtrlVars.dac1_mclk_src = 4;
#else
	gCtrlVars.dac1_mclk_src = 0;
#endif

	if((param_ptr = get_param_by_raw_data(0x0B, parameters)) == NULL)
		return;

	//I2S0
	gCtrlVars.i2s0_tx_en = 0;
	gCtrlVars.i2s0_rx_en = 0;
	gCtrlVars.i2s0_sample_rate = 48000;//g_user_effect_list->sample_rate;
#ifdef EXTERNAL_CLK
	gCtrlVars.i2s0_mclk_src = 4;
	gCtrlVars.i2s0_work_mode = 1;
#else
	gCtrlVars.i2s0_mclk_src = 0;
	gCtrlVars.i2s0_work_mode = 0;
#endif
	gCtrlVars.i2s0_format = I2S_FORMAT_I2S;
	if((device_node = adapt_get_item(g_adapter, "SOURCE_I2S0_RX")) != NULL || (device_node = adapt_get_item(g_adapter, "SINK_I2S0_TX")) != NULL)
		gCtrlVars.i2s0_word_len = (device_node->width == BITS_16)?(I2S_LENGTH_16BITS):(I2S_LENGTH_24BITS);
	else
		gCtrlVars.i2s0_word_len = I2S_LENGTH_24BITS;
	gCtrlVars.i2s0_mono = FALSE;
	gCtrlVars.i2s0_fade_time = 0;
	gCtrlVars.i2s0_bclk_invert_en = FALSE;
	gCtrlVars.i2s0_lrclk_invert_en = FALSE;

	if((param_ptr = get_param_by_raw_data(0x0C, parameters)) == NULL)
		return;

	//I2S1
	gCtrlVars.i2s1_tx_en = 1;
	gCtrlVars.i2s1_rx_en = 1;
	gCtrlVars.i2s1_sample_rate = 48000;//g_user_effect_list->sample_rate;
	
#ifdef EXTERNAL_CLK
	gCtrlVars.i2s1_mclk_src = 4;
	gCtrlVars.i2s1_work_mode = 1;
#else
	gCtrlVars.i2s1_mclk_src = 0;
	gCtrlVars.i2s1_work_mode = 0;
#endif

	gCtrlVars.i2s1_format = I2S_FORMAT_I2S;
	if((device_node = adapt_get_item(g_adapter, "SOURCE_I2S1_RX")) != NULL || (device_node = adapt_get_item(g_adapter, "SINK_I2S1_TX")) != NULL)
		gCtrlVars.i2s1_word_len = (device_node->width == BITS_16)?(I2S_LENGTH_16BITS):(I2S_LENGTH_24BITS);
	else
		gCtrlVars.i2s1_word_len = I2S_LENGTH_24BITS;
	gCtrlVars.i2s1_mono = FALSE;
	gCtrlVars.i2s1_fade_time = 0;
	gCtrlVars.i2s1_bclk_invert_en = FALSE;
	gCtrlVars.i2s1_lrclk_invert_en = FALSE;

	if((param_ptr = get_param_by_raw_data(0x0D, parameters)) == NULL)
		return;

	//SPDIF
	gCtrlVars.spdif_en = 0;
	gCtrlVars.spdif_sample_rate = g_user_effect_list->sample_rate;
	gCtrlVars.spdif_ch_mode = 0;
	gCtrlVars.spdif_io_sel = 0;

}

void misc_pga_channel_set(ADC_MODULE ADCModule, ADC_CHANNEL ChannelSel, uint32_t flag)
{
	//DBG("pga_main: %d,%d,%X\n", ADCModule, ChannelSel, flag);
	if(ADCModule == ADC0_MODULE)
	{
		if(ChannelSel == CHANNEL_LEFT)
		{
			AudioADC_PGASel(ADCModule, ChannelSel,INPUT_NONE);
			if(!!(flag & PGA0_INPUT_LINEIN1_LEFT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN1_LEFT);
			if(!!(flag & PGA0_INPUT_MIC4))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_MIC4);
			if(!!(flag & PGA0_INPUT_LINEIN2_LEFT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN2_LEFT);
		}
		else if(ChannelSel == CHANNEL_RIGHT)
		{
			AudioADC_PGASel(ADCModule, ChannelSel,INPUT_NONE);
			if(!!(flag & PGA0_INPUT_LINEIN1_RIGHT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN1_RIGHT);
			if(!!(flag & PGA0_INPUT_MIC3))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_MIC3);
			if(!!(flag & PGA0_INPUT_LINEIN2_RIGHT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN2_RIGHT);
		}
	}
	else if(ADCModule == ADC1_MODULE)
	{
		if(ChannelSel == CHANNEL_LEFT)
		{
			AudioADC_PGASel(ADCModule, ChannelSel,INPUT_NONE);
			if(!!(flag & PGA1_INPUT_FMIN_LEFT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_FMIN_LEFT);
			if(!!(flag & PGA1_INPUT_LINEIN2_LEFT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN2_LEFT);
			if(!!(flag & PGA1_INPUT_MIC2))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_MIC2);
			if(!!(flag & PGA1_INPUT_LINEIN3_LEFT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN3_LEFT);
		}
		else if(ChannelSel == CHANNEL_RIGHT)
		{
			AudioADC_PGASel(ADCModule, ChannelSel,INPUT_NONE);
			if(!!(flag & PGA1_INPUT_FMIN_RIGHT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_FMIN_RIGHT);
			if(!!(flag & PGA1_INPUT_LINEIN2_RIGHT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN2_RIGHT);
			if(!!(flag & PGA1_INPUT_MIC1))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_MIC1);
			if(!!(flag & PGA1_INPUT_LINEIN3_RIGHT))
				AudioADC_PGASel(ADCModule, ChannelSel,INPUT_LINEIN3_RIGHT);
		}
	}
}

void load_params_to_codec(void)
{
	//PGA0 channel
	misc_pga_channel_set(ADC0_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_r_channel);
	misc_pga_channel_set(ADC0_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_l_channel);

	//PGA0
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_LINEIN1_RIGHT, 63-gCtrlVars.line1_r_gain, gCtrlVars.pga0_r_boost);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_LINEIN1_LEFT, 63-gCtrlVars.line1_l_gain, gCtrlVars.pga0_l_boost);

	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_LINEIN2_RIGHT, 63-gCtrlVars.line2_0_r_gain, gCtrlVars.pga0_r_boost);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_LINEIN2_LEFT, 63-gCtrlVars.line2_0_l_gain, gCtrlVars.pga0_l_boost);

	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_MIC3, 63-gCtrlVars.mic3_r_gain, gCtrlVars.pga0_r_boost);
	AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_MIC4, 63-gCtrlVars.mic4_l_gain, gCtrlVars.pga0_l_boost);
	if(gCtrlVars.pga0_zero_cross==0)
	{
		AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 0, 0);
	}
	else if(gCtrlVars.pga0_zero_cross==1)
	{
		AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 1, 0);
	}
	else if(gCtrlVars.pga0_zero_cross==2)
	{
		AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 0, 1);
	}
	else if(gCtrlVars.pga0_zero_cross==3)
	{
		AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 1, 1);
	}
	if(gCtrlVars.pga0_mode == 0)
		AudioADC_PGAMode(1, 1);
	else if(gCtrlVars.pga0_mode == 1)
		AudioADC_PGAMode(0, 1);
	else if(gCtrlVars.pga0_mode == 2)
		AudioADC_PGAMode(1, 0);
	else if(gCtrlVars.pga0_mode == 3)
		AudioADC_PGAMode(0, 0);


	AudioADC_PGADiffGainSel(gCtrlVars.pga0_diff_gain_l, gCtrlVars.pga0_diff_gain_r);
	AudioADC_PGADiffGainSel(gCtrlVars.pga0_diff_gain_l, gCtrlVars.pga0_diff_gain_r);

	//ADC0
	AudioADC_DigitalMute(ADC0_MODULE, !!gCtrlVars.adc0_dig_l_mute, !!gCtrlVars.adc0_dig_r_mute);
	AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc0_dig_l_vol);
	AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_RIGHT,(uint16_t)gCtrlVars.adc0_dig_r_vol);
	AudioADC_ChannelSwap(ADC0_MODULE, gCtrlVars.adc0_lr_swap);
	AudioADC_HighPassFilterConfig(ADC0_MODULE, HPCList[gCtrlVars.adc0_dc_blocker]);
	if(gCtrlVars.adc0_fade_time==0)
	{
		AudioADC_FadeDisable(ADC0_MODULE);
	}
	else
	{
		AudioADC_FadeEnable(ADC0_MODULE);
		AudioADC_FadeTimeSet(ADC0_MODULE, gCtrlVars.adc0_fade_time);
	}
	if(gCtrlVars.adc0_dc_blocker_en)
	{
		AudioADC_HighPassFilterSet(ADC0_MODULE, TRUE);
	}
	else
	{
		AudioADC_HighPassFilterSet(ADC0_MODULE, FALSE);
	}

	//AGC0
	// AudioADC_AGCChannelSel(ADC0_MODULE, !!(gCtrlVars.agc0_mode & 0x01), !!(gCtrlVars.agc0_mode & 0x02));//active it after
	AudioADC_AGCMaxLevel(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_max_level);
	AudioADC_AGCTargetLevel(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_target_level);
	AudioADC_AGCMaxGain(ADC0_MODULE,(uint8_t)(63 - gCtrlVars.agc0_max_gain));
	AudioADC_AGCMinGain(ADC0_MODULE,(uint8_t)(63 - gCtrlVars.agc0_min_gain));
	AudioADC_AGCGainOffset(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_gainoffset);
	AudioADC_AGCFrameTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_fram_time);
	AudioADC_AGCHoldFrames(ADC0_MODULE,(uint32_t)gCtrlVars.agc0_hold_frames);
	AudioADC_AGCAttackStepTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_attack_time);
	AudioADC_AGCDecayStepTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_decay_time);

	AudioADC_AGCNoiseGateEnable(ADC0_MODULE,(bool)gCtrlVars.agc0_noise_gate_en);
	if(gCtrlVars.agc0_noise_gate_en)
	{
		AudioADC_AGCNoiseThreshold(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_noise_threshold);
		AudioADC_AGCNoiseGateMode(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_noise_gate_mode);
		AudioADC_AGCNoiseHoldFrames(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_noise_hold_frames);
	}
	
	Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);

	//PGA1 channel
	misc_pga_channel_set(ADC1_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_r_channel);
	misc_pga_channel_set(ADC1_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_l_channel);

	if(gCtrlVars.pga1_r_channel & PGA1_INPUT_LINEIN3_RIGHT)
		gCtrlVars.mic1_r_gain = gCtrlVars.line3_r_gain;
	else
		gCtrlVars.line3_r_gain = gCtrlVars.mic1_r_gain;

	if(gCtrlVars.pga1_l_channel & PGA1_INPUT_LINEIN3_LEFT)
		gCtrlVars.mic2_l_gain = gCtrlVars.line3_l_gain;
	else
		gCtrlVars.line3_l_gain = gCtrlVars.mic2_l_gain;

	//PGA1
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_LINEIN3_RIGHT, 63-gCtrlVars.line3_r_gain, gCtrlVars.pga1_r_boost);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_LINEIN3_LEFT, 63-gCtrlVars.line3_l_gain, gCtrlVars.pga1_l_boost);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_LINEIN2_RIGHT, 63-gCtrlVars.line2_1_r_gain, gCtrlVars.pga1_r_boost);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_LINEIN2_LEFT, 63-gCtrlVars.line2_1_l_gain, gCtrlVars.pga1_l_boost);

	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_FMIN_RIGHT, 63-gCtrlVars.fm_r_gain, gCtrlVars.pga1_r_boost);
	AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_FMIN_LEFT, 63-gCtrlVars.fm_l_gain, gCtrlVars.pga1_l_boost);

	// AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_MIC1, 63-gCtrlVars.mic1_r_gain, gCtrlVars.pga1_r_boost);
	// AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_MIC2, 63-gCtrlVars.mic2_l_gain, gCtrlVars.pga1_l_boost);

	if(gCtrlVars.pga1_zero_cross==0)
	{
		AudioADC_PGAZeroCrossEnable(ADC1_MODULE, 0, 0);
	}
	else if(gCtrlVars.pga1_zero_cross==1)
	{
		AudioADC_PGAZeroCrossEnable(ADC1_MODULE, 1, 0);
	}
	else if(gCtrlVars.pga1_zero_cross==2)
	{
		AudioADC_PGAZeroCrossEnable(ADC1_MODULE, 0, 1);
	}
	else if(gCtrlVars.pga1_zero_cross==3)
	{
		AudioADC_PGAZeroCrossEnable(ADC1_MODULE, 1, 1);
	}

	//ADC1
	AudioADC_DigitalMute(ADC1_MODULE, !!gCtrlVars.adc1_dig_l_mute, !!gCtrlVars.adc1_dig_r_mute);
	AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc1_dig_l_vol);
	AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_RIGHT,(uint16_t)gCtrlVars.adc1_dig_r_vol);
	AudioADC_ChannelSwap(ADC1_MODULE, gCtrlVars.adc1_lr_swap);
	AudioADC_HighPassFilterConfig(ADC1_MODULE, HPCList[gCtrlVars.adc1_dc_blocker]);
	if(gCtrlVars.adc1_fade_time==0)
	{
		AudioADC_FadeDisable(ADC1_MODULE);
	}
	else
	{
		AudioADC_FadeEnable(ADC1_MODULE);
		AudioADC_FadeTimeSet(ADC1_MODULE, gCtrlVars.adc1_fade_time);
	}
	if(gCtrlVars.adc1_dc_blocker_en)
	{
		AudioADC_HighPassFilterSet(ADC1_MODULE, TRUE);
	}
	else
	{
		AudioADC_HighPassFilterSet(ADC1_MODULE, FALSE);
	}

	//AGC1
	// AudioADC_AGCChannelSel(ADC1_MODULE, !!(gCtrlVars.agc1_mode & 0x01), !!(gCtrlVars.agc1_mode & 0x02));//active it after
	AudioADC_AGCMaxLevel(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_max_level);
	AudioADC_AGCTargetLevel(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_target_level);
	AudioADC_AGCMaxGain(ADC1_MODULE,(uint8_t)(63 - gCtrlVars.agc1_max_gain));
	AudioADC_AGCMinGain(ADC1_MODULE,(uint8_t)(63 - gCtrlVars.agc1_min_gain));
	AudioADC_AGCGainOffset(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_gainoffset);
	AudioADC_AGCFrameTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_fram_time);
	AudioADC_AGCHoldFrames(ADC1_MODULE,(uint32_t)gCtrlVars.agc1_hold_frames);
	AudioADC_AGCAttackStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_attack_time);
	AudioADC_AGCDecayStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_decay_time);
	
	AudioADC_AGCNoiseGateEnable(ADC1_MODULE,(bool)gCtrlVars.agc1_noise_gate_en);
	if(gCtrlVars.agc1_noise_gate_en)
	{
		AudioADC_AGCNoiseThreshold(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_noise_threshold);
		AudioADC_AGCNoiseGateMode(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_noise_gate_mode);
		AudioADC_AGCNoiseHoldFrames(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_noise_hold_frames);
	}
	Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);

	//DAC0
	AudioDAC_DigitalMute(DAC0, gCtrlVars.dac0_dig_l_mute, gCtrlVars.dac0_dig_r_mute);
	AudioDAC_VolSet(DAC0, gCtrlVars.dac0_dig_l_vol, gCtrlVars.dac0_dig_r_vol);
	if(gCtrlVars.dac0_dither)
	{
		AudioDAC_DitherEnable(DAC0);
		AudioDAC_DitherPowSet(DAC0,gCtrlVars.dac0_dither);
	}
	else
	{
		AudioDAC_DitherDisable(DAC0);
	}

	if(gCtrlVars.dac0_scramble == 0)
	{
		AudioDAC_ScrambleDisable(DAC0);
	}
	else
	{
		AudioDAC_ScrambleEnable(DAC0);
		AudioDAC_ScrambleModeSet(DAC0,(SCRAMBLE_MODULE)gCtrlVars.dac0_scramble);
	}

	AudioDAC_DoutModeSet((gCtrlVars.dac0_out_mode == 0)?(0):(gCtrlVars.dac0_out_mode+1));
	AudioDAC_EdgeSet(DAC0, gCtrlVars.dac0_sample_mode);
	AudioDAC_SCFMute(DAC0, gCtrlVars.dac0_scf_mute&0x01, (gCtrlVars.dac0_scf_mute&0x02)>>1);
	if(gCtrlVars.dac0_fade_time == 0)
	{
		AudioDAC_FadeDisable(DAC0);
	}
	else
	{
		AudioDAC_FadeEnable(DAC0);
		AudioDAC_FadeTimeSet(DAC0,gCtrlVars.dac0_fade_time);
	}
	AudioDAC_ZeroNumSet(DAC0, gCtrlVars.dac0_zeros_number);


	//DAC1
	AudioDAC_DigitalMute(DAC1, gCtrlVars.dac1_dig_mute, gCtrlVars.dac1_dig_mute);
	AudioDAC_VolSet(DAC1, gCtrlVars.dac1_dig_vol, gCtrlVars.dac1_dig_vol);
	AudioDAC_VolSet(DAC1, gCtrlVars.dac1_dig_vol, gCtrlVars.dac1_dig_vol);
	if(gCtrlVars.dac1_dither)
	{
		AudioDAC_DitherEnable(DAC1);
		AudioDAC_DitherPowSet(DAC1,gCtrlVars.dac1_dither);
	}
	else
	{
		AudioDAC_DitherDisable(DAC1);
	}

	if(gCtrlVars.dac1_scramble == 0)
	{
		AudioDAC_ScrambleDisable(DAC1);
	}
	else
	{
		AudioDAC_ScrambleEnable(DAC1);
		AudioDAC_ScrambleModeSet(DAC1,(SCRAMBLE_MODULE)gCtrlVars.dac1_scramble);
	}
	AudioDAC_EdgeSet(DAC1, gCtrlVars.dac1_sample_mode);
	AudioDAC_SCFMute(DAC1, gCtrlVars.dac1_scf_mute&0x01, (gCtrlVars.dac1_scf_mute&0x02)>>1);
	if(gCtrlVars.dac1_fade_time == 0)
	{
		AudioDAC_FadeDisable(DAC1);
	}
	else
	{
		AudioDAC_FadeEnable(DAC1);
		AudioDAC_FadeTimeSet(DAC1,gCtrlVars.dac1_fade_time);
	}
	AudioDAC_ZeroNumSet(DAC1, gCtrlVars.dac1_zeros_number);

	//no I2S, no SPDIF

}

//add display selection
void Comm_PGA0_0x03(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)///ADC0 PGA
	{
		case 6:///line1 Left en?
			if(data == 1)
				gCtrlVars.pga0_l_channel |= PGA0_INPUT_LINEIN1_LEFT;
			else if(data == 0)
				gCtrlVars.pga0_l_channel &= ~PGA0_INPUT_LINEIN1_LEFT;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_l_channel);
			break;

		case 7://line1 Right en?
			if(data == 1)
				gCtrlVars.pga0_r_channel |= PGA0_INPUT_LINEIN1_RIGHT;
			else if(data == 0)
				gCtrlVars.pga0_r_channel &= ~PGA0_INPUT_LINEIN1_RIGHT;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_r_channel);
			break;

		case 8:///mic4 en?
			if(data == 1)
				gCtrlVars.pga0_l_channel |= PGA0_INPUT_MIC4;
			else if(data == 0)
				gCtrlVars.pga0_l_channel &= ~PGA0_INPUT_MIC4;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_l_channel);
			break;

		case 9://mic3 en ?
			if(data == 1)
				gCtrlVars.pga0_r_channel |= PGA0_INPUT_MIC3;
			else if(data == 0)
				gCtrlVars.pga0_r_channel &= ~PGA0_INPUT_MIC3;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_r_channel);
			break;


		case 10://line2 Left en?
			if(data == 1)
				gCtrlVars.pga0_l_channel |= PGA0_INPUT_LINEIN2_LEFT;
			else if(data == 0)
				gCtrlVars.pga0_l_channel &= ~PGA0_INPUT_LINEIN2_LEFT;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_l_channel);
			break;

		case 11://line2 Right en?
			if(data == 1)
				gCtrlVars.pga0_r_channel |= PGA0_INPUT_LINEIN2_RIGHT;
			else if(data == 0)
				gCtrlVars.pga0_r_channel &= ~PGA0_INPUT_LINEIN2_RIGHT;
			misc_pga_channel_set(ADC0_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga0_r_channel);
			break;

		case 12://line1 Left gain
			//DBG("line1 l: %d\n");
			gCtrlVars.line1_l_gain = data > 63? 63 : data;
			gCtrlVars.pga0_l_gain = 63 - gCtrlVars.line1_l_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_LINEIN1_LEFT, (uint16_t)gCtrlVars.pga0_l_gain, gCtrlVars.pga0_l_boost);
			AudioADC_PGADiffGainSel((uint16_t)gCtrlVars.pga0_l_gain, (uint16_t)gCtrlVars.pga0_r_gain);
			break;

		case 13://line1 Right gain
			//DBG("line1 r: %d\n");
			gCtrlVars.line1_r_gain = data > 63? 63 : data;
			gCtrlVars.pga0_r_gain = 63 - gCtrlVars.line1_r_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_LINEIN1_RIGHT, (uint16_t)gCtrlVars.pga0_r_gain, gCtrlVars.pga0_r_boost);
			AudioADC_PGADiffGainSel((uint16_t)gCtrlVars.pga0_l_gain, (uint16_t)gCtrlVars.pga0_r_gain);
			break;

		case 14://mic4 gain
			gCtrlVars.mic4_l_gain = data > 63? 63 : data;
			gCtrlVars.pga0_l_gain = 63 - gCtrlVars.mic4_l_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_MIC4, (uint16_t)gCtrlVars.pga0_l_gain, gCtrlVars.pga0_l_boost);
			break;

		case 15://mic3 gain
			gCtrlVars.mic3_r_gain = data > 63? 63 : data;
			gCtrlVars.pga0_r_gain = 63 - gCtrlVars.mic3_r_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_MIC3, (uint16_t)gCtrlVars.pga0_r_gain, gCtrlVars.pga0_r_boost);
			break;

		case 16:///line2 Left gain
			gCtrlVars.line2_0_l_gain = data > 63? 63 : data;
			gCtrlVars.pga0_l_gain = 63 - gCtrlVars.line2_0_l_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_LINEIN2_LEFT, (uint16_t)gCtrlVars.pga0_l_gain, gCtrlVars.pga0_l_boost);
			break;

		case 17:///line2 Right gain
			gCtrlVars.line2_0_r_gain = data > 63? 63 : data;
			gCtrlVars.pga0_r_gain = 63 - gCtrlVars.line2_0_r_gain;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_LINEIN2_RIGHT, (uint16_t)gCtrlVars.pga0_r_gain, gCtrlVars.pga0_r_boost);
			break;

		case 18://mic4 gain boost
			data = data > 4? 4 : data;
			gCtrlVars.mic4_l_boost =  data;
			if( data == 0) data = 4;
			else data -= 1;
			gCtrlVars.pga0_l_boost = data;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_LEFT, INPUT_MIC4, (uint16_t)gCtrlVars.pga0_l_gain, gCtrlVars.pga0_l_boost);
			// printf("%d, %d\n", gCtrlVars.pga0_l_boost, gCtrlVars.mic4_l_boost);
			break;

		case 19:///mic3 gain boost
			data = data > 4? 4 : data;
			gCtrlVars.mic3_r_boost =  data;
			if( data == 0) data = 4;
			else data -= 1;
			gCtrlVars.pga0_r_boost = data;
			AudioADC_PGAGainSet(ADC0_MODULE, CHANNEL_RIGHT, INPUT_MIC3, (uint16_t)gCtrlVars.pga0_r_gain, gCtrlVars.pga0_r_boost);
			break;

#if 0
		case 20://pga0 zero cross
			gCtrlVars.pga0_zero_cross = data > 3? 3 : data;
			if(gCtrlVars.pga0_zero_cross==0)
			{
				AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 0, 0);
			}
			else if(gCtrlVars.pga0_zero_cross==1)
			{
				AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 1, 0);
			}
			else if(gCtrlVars.pga0_zero_cross==2)
			{
				AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 0, 1);
			}
			else if(gCtrlVars.pga0_zero_cross==3)
			{
				AudioADC_PGAZeroCrossEnable(ADC0_MODULE, 1, 1);
			}
			break;
#endif

		case 20:
			gCtrlVars.pga0_mode = data & 0x03;
			if(gCtrlVars.pga0_mode == 0)
				AudioADC_PGAMode(1, 1);
			else if(gCtrlVars.pga0_mode == 1)
				AudioADC_PGAMode(0, 1);
			else if(gCtrlVars.pga0_mode == 2)
				AudioADC_PGAMode(1, 0);
			else if(gCtrlVars.pga0_mode == 3)
				AudioADC_PGAMode(0, 0);
			break;

		case 21:
			gCtrlVars.pga0_diff_gain_l = data & 0x03;
			AudioADC_PGADiffGainSel(gCtrlVars.pga0_diff_gain_l, gCtrlVars.pga0_diff_gain_r);
			break;

		case 22:
			gCtrlVars.pga0_diff_gain_r = data & 0x03;
			AudioADC_PGADiffGainSel(gCtrlVars.pga0_diff_gain_l, gCtrlVars.pga0_diff_gain_r);
			break;

		default:
			break;
	}
}

//add pga channel display selection
uint32_t Comm_Form_Rsp_PGA0_0x03(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x03;//cmd
	buffer[3]  = 1 + 17*2 + 6*2;//len, 6*2 for display
	buffer[4]  = 0xff;///all paramgs

	pp = (int16_t*)&buffer[5];

	//display selection
	*(pp++) = DISP_LINEIN1_LEFT;//linein1 left
	*(pp++) = DISP_LINEIN1_RIGHT;//linein1 right
	*(pp++) = DISP_MIC4;//MIC4
	*(pp++) = DISP_MIC3;//MIC3
	*(pp++) = DISP_LINEIN2_0_LEFT;//linein2 left
	*(pp++) = DISP_LINEIN2_0_RIGHT;//linein2 right

	//6
	*(pp++) = !!(gCtrlVars.pga0_l_channel & PGA0_INPUT_LINEIN1_LEFT);
	*(pp++) = !!(gCtrlVars.pga0_r_channel & PGA0_INPUT_LINEIN1_RIGHT);
	*(pp++) = !!(gCtrlVars.pga0_l_channel & PGA0_INPUT_MIC4);
	*(pp++) = !!(gCtrlVars.pga0_r_channel & PGA0_INPUT_MIC3);
	*(pp++) = !!(gCtrlVars.pga0_l_channel & PGA0_INPUT_LINEIN2_LEFT);
	*(pp++) = !!(gCtrlVars.pga0_r_channel & PGA0_INPUT_LINEIN2_RIGHT);

	//12
	*(pp++) =  gCtrlVars.line1_l_gain;//Line1 left gain
	*(pp++) =  gCtrlVars.line1_r_gain;//Line1 right gain

	//14
	*(pp++) = gCtrlVars.mic4_l_gain;//mic4 gain
	*(pp++) = gCtrlVars.mic3_r_gain;//mic3 gain

	//16
	*(pp++) = gCtrlVars.line2_0_l_gain;//Line2 left gain
	*(pp++) = gCtrlVars.line2_0_r_gain;//Line2 right gain

	//18
	*(pp++) = gCtrlVars.mic4_l_boost;
	*(pp++) = gCtrlVars.mic3_r_boost;

	//20
	// *(pp++) = gCtrlVars.pga0_zero_cross;////adc0 zero cross gain

	//20
	*(pp++) = gCtrlVars.pga0_mode;///pga mode

	//21
	*(pp++) = gCtrlVars.pga0_diff_gain_l;//Differential Left Gain

	//22
	*(pp++) = gCtrlVars.pga0_diff_gain_r;//Differential Right Gain


	buffer[buffer[3] + 4]  = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x03(uint8_t *buf, uint32_t len)//ADC0 PGA
{
	uint32_t TmpData;
	uint16_t i,k;

	if(len == 0) //ask
	{
		int16_t *pp;
		memset(tx_buf, 0, sizeof(tx_buf));
		//DBG("ADC0 PGA param required %d,%d.\n", gCtrlVars.pga0_l_channel, gCtrlVars.pga0_r_channel);
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_PGA0_0x03(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<23; i++, pp++)
			{
				Comm_PGA0_0x03(i, *pp);
			}
		}
		else
		{
			//DBG("PGA0_0x03: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_PGA0_0x03(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x03;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

void Comm_ADC0_0x04(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)///adc0 digital set
	{
		case 0://ADC0 en?
			// gCtrlVars.adc0_dig_channel_en = data & 0x03;
			//AudioADC_LREnable(ADC0_MODULE, !!(data & 0x01), !!(data & 0x02));
			// efft_fade_out(TRUE);
			break;

		case 1:///ADC0 mute?
			gCtrlVars.adc0_dig_mute = data & 0x03;
			gCtrlVars.adc0_dig_l_mute = !!(gCtrlVars.adc0_dig_mute & 0x01);
			gCtrlVars.adc0_dig_r_mute = !!(gCtrlVars.adc0_dig_mute & 0x02);
			AudioADC_DigitalMute(ADC0_MODULE, !!gCtrlVars.adc0_dig_l_mute, !!gCtrlVars.adc0_dig_r_mute);
			break;

		case 2://adc0 dig vol left
			gCtrlVars.adc0_dig_l_vol = data > 0x3fff? 0x3fff : data;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc0_dig_l_vol);
			break;

		case 3://adc0 dig vol right
			gCtrlVars.adc0_dig_r_vol = data > 0x3fff? 0x3fff : data;
			AudioADC_VolSetChannel(ADC0_MODULE, CHANNEL_RIGHT,(uint16_t)gCtrlVars.adc0_dig_r_vol);
			break;

		case 4://adc0 sample rate
			// if(gCtrlVars.sys_sample_rate_en == 0)
			// {
			// 	gCtrlVars.adc0_sample_rate = SupportSampleRateList[(data > 8)?(7):(data)];
			// 	AudioADC_SampleRateSet(ADC0_MODULE, gCtrlVars.adc0_sample_rate);
			// }
			break;

		case 5://adc0 LR swap
			gCtrlVars.adc0_lr_swap = !!data;
			AudioADC_ChannelSwap(ADC0_MODULE, !!data);
			break;

		case 6://adc0 hight pass
			gCtrlVars.adc0_dc_blocker = (data>2)?(2):(data);
			AudioADC_HighPassFilterConfig(ADC0_MODULE, HPCList[gCtrlVars.adc0_dc_blocker]);
			break;

		case 7://adc0 fade time
			gCtrlVars.adc0_fade_time = data>255?(0):(data);
			if(gCtrlVars.adc0_fade_time==0)
			{
				AudioADC_FadeDisable(ADC0_MODULE);
			}
			else
			{
				AudioADC_FadeEnable(ADC0_MODULE);
				AudioADC_FadeTimeSet(ADC0_MODULE, gCtrlVars.adc0_fade_time);
			}
			break;

		case 8://adc0 mclk src
			// gCtrlVars.adc0_mclk_src = (data> 4)?(0):(data);
			// if(gCtrlVars.sys_mclk_src_en == 0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_ADC0, (MCLK_CLK_SEL)gCtrlVars.adc0_mclk_src);
			// }
			break;

		case 9://hpc0 en
			gCtrlVars.adc0_dc_blocker_en = !!data;
			if(gCtrlVars.adc0_dc_blocker_en)
			{
				AudioADC_HighPassFilterSet(ADC0_MODULE, TRUE);
			}
			else
			{
				AudioADC_HighPassFilterSet(ADC0_MODULE, FALSE);
			}
			break;

		default:
			break;
	}

}

uint32_t Comm_Form_Rsp_ADC0_0x04(uint8_t *buffer)
{
	int16_t *pp;

	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x04;
	buffer[3]  = 1+10*2;//len
	buffer[4]  = 0xff;

	pp = (int16_t*)&buffer[5];
	*(pp++) =  gCtrlVars.adc0_dig_channel_en;
	*(pp++) =  gCtrlVars.adc0_dig_mute;
	*(pp++) =  gCtrlVars.adc0_dig_l_vol;
	*(pp++) =  gCtrlVars.adc0_dig_r_vol;
	*(pp++) =  comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
	*(pp++) =  gCtrlVars.adc0_lr_swap;
	*(pp++) =  gCtrlVars.adc0_dc_blocker;
	*(pp++) =  gCtrlVars.adc0_fade_time;
	*(pp++) =  gCtrlVars.adc0_mclk_src;
	*(pp++) =  gCtrlVars.adc0_dc_blocker_en;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x04(uint8_t *buf, uint32_t len)////ADC0 DIGITAL
{

	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_ADC0_0x04(tx_buf));///25+3*4+1
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<10; i++, pp++)
			{
				Comm_ADC0_0x04(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_ADC0_0x04(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x04;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}




void Comm_AGC0_0x05(int16_t index, int16_t data)
{
	uint32_t TmpData;
	switch(index)//ADC0 AGC
	{
		case 0://AGC {data=0 dis} {data=1 left en} {data=2 right en} {data=3 left+right en}
			gCtrlVars.agc0_mode = data & 0x03;
			AudioADC_AGCChannelSel(ADC0_MODULE, !!(data & 0x01), !!(data & 0x02));
			break;

		case 1://MAX level
			gCtrlVars.agc0_max_level = (data>31)?(31):(data);
			AudioADC_AGCMaxLevel(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_max_level);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 2://target level
			gCtrlVars.agc0_target_level = (data>31)?(31):(data);
			AudioADC_AGCTargetLevel(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_target_level);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 3://max gain
			gCtrlVars.agc0_max_gain = (data>63)?(63):(data);
			AudioADC_AGCMaxGain(ADC0_MODULE,(uint8_t)(63 - gCtrlVars.agc0_max_gain));
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 4://min gain
			gCtrlVars.agc0_min_gain = (data>63)?(63):(data);
			AudioADC_AGCMinGain(ADC0_MODULE,(uint8_t)(63 - gCtrlVars.agc0_min_gain));
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 5://gain offset
			gCtrlVars.agc0_gainoffset = (data>15)?(15):(data);
			AudioADC_AGCGainOffset(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_gainoffset);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 6://frame time
			gCtrlVars.agc0_fram_time = (data>4096)?(4096):(data);
			AudioADC_AGCFrameTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_fram_time);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 7://hold time
			gCtrlVars.agc0_hold_frames = (data>31)?(31):(data);
			AudioADC_AGCHoldFrames(ADC0_MODULE,(uint32_t)gCtrlVars.agc0_hold_frames);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 8://attack time
			gCtrlVars.agc0_attack_time = (data>4096)?(4096):(data);
			AudioADC_AGCAttackStepTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_attack_time);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 9://dacay time
			gCtrlVars.agc0_decay_time = (data>4096)?(4096):(data);
			AudioADC_AGCDecayStepTime(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_decay_time);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 10://nosie gain en
			gCtrlVars.agc0_noise_gate_en = (data>1)?(1):(data);
			AudioADC_AGCNoiseGateEnable(ADC0_MODULE,(bool)gCtrlVars.agc0_noise_gate_en);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 11://nosie thershold
			gCtrlVars.agc0_noise_threshold = (data>31)?(31):(data);
			AudioADC_AGCNoiseThreshold(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_noise_threshold);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 12://nosie gate mode
			gCtrlVars.agc0_noise_gate_mode = (data>1)?(1):(data);
			AudioADC_AGCNoiseGateMode(ADC0_MODULE,(uint8_t)gCtrlVars.agc0_noise_gate_mode);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		case 13://nosie gate hold time
			gCtrlVars.agc0_noise_hold_frames = (data>31)?(31):(data);
			AudioADC_AGCNoiseHoldFrames(ADC0_MODULE,(uint16_t)gCtrlVars.agc0_noise_hold_frames);
			Reset_AGC_ChannelSel(ADC0_MODULE, gCtrlVars.agc0_mode);
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_AGC0_0x05(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x05;
	buffer[3]  = 1+14*2;//len
	buffer[4]  = 0xff;

	pp = (int16_t*)&buffer[5];
	*(pp ++) = gCtrlVars.agc0_mode;
	*(pp ++) = gCtrlVars.agc0_max_level;
	*(pp ++) = gCtrlVars.agc0_target_level;
	*(pp ++) = gCtrlVars.agc0_max_gain;
	*(pp ++) = gCtrlVars.agc0_min_gain;
	*(pp ++) = gCtrlVars.agc0_gainoffset;
	*(pp ++) = gCtrlVars.agc0_fram_time;
	*(pp ++) = gCtrlVars.agc0_hold_frames;
	*(pp ++) = gCtrlVars.agc0_attack_time;
	*(pp ++) = gCtrlVars.agc0_decay_time;
	*(pp ++) = gCtrlVars.agc0_noise_gate_en;
	*(pp ++) = gCtrlVars.agc0_noise_threshold;
	*(pp ++) = gCtrlVars.agc0_noise_gate_mode;
	*(pp ++) = gCtrlVars.agc0_noise_hold_frames;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x05(uint8_t *buf, uint32_t len)///ADC0 AGC
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_AGC0_0x05(tx_buf));
	}
	else
	{
		//efft_fade_out(TRUE);
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<14; i++, pp++)
			{
				Comm_AGC0_0x05(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_AGC0_0x05(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x05;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}

}


void Comm_PGA1_0x06(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)///ADC1 PGA
	{
		case 8:///line3 Left en?
			if(data == 1)
				gCtrlVars.pga1_l_channel |= PGA1_INPUT_LINEIN3_LEFT;
			else if(data == 0)
				gCtrlVars.pga1_l_channel &= ~PGA1_INPUT_LINEIN3_LEFT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_l_channel);
			break;

		case 9://line3 Right en?
			if(data == 1)
				gCtrlVars.pga1_r_channel |= PGA1_INPUT_LINEIN3_RIGHT;
			else if(data == 0)
				gCtrlVars.pga1_r_channel &= ~PGA1_INPUT_LINEIN3_RIGHT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_r_channel);
			break;

		case 10://mic2 en?
			if(data == 1)
				gCtrlVars.pga1_l_channel |= PGA1_INPUT_MIC2;
			else if(data == 0)
				gCtrlVars.pga1_l_channel &= ~PGA1_INPUT_MIC2;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_l_channel);
			break;

		case 11://mic1 en?
			if(data == 1)
				gCtrlVars.pga1_r_channel |= PGA1_INPUT_MIC1;
			else if(data == 0)
				gCtrlVars.pga1_r_channel &= ~PGA1_INPUT_MIC1;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_r_channel);
			break;

		case 12:///line2 Left en?
			if(data == 1)
				gCtrlVars.pga1_l_channel |= PGA1_INPUT_LINEIN2_LEFT;
			else if(data == 0)
				gCtrlVars.pga1_l_channel &= ~PGA1_INPUT_LINEIN2_LEFT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_l_channel);
			break;

		case 13://line2 Right en?
			if(data == 1)
				gCtrlVars.pga1_r_channel |= PGA1_INPUT_LINEIN2_RIGHT;
			else if(data == 0)
				gCtrlVars.pga1_r_channel &= ~PGA1_INPUT_LINEIN2_RIGHT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_r_channel);
			break;

		case 14:///fm Left en?
			if(data == 1)
				gCtrlVars.pga1_l_channel |= PGA1_INPUT_FMIN_LEFT;
			else if(data == 0)
				gCtrlVars.pga1_l_channel &= ~PGA1_INPUT_FMIN_LEFT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_LEFT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_l_channel);
			break;

		case 15://fm Right en?
			if(data == 1)
				gCtrlVars.pga1_r_channel |= PGA1_INPUT_FMIN_RIGHT;
			else if(data == 0)
				gCtrlVars.pga1_r_channel &= ~PGA1_INPUT_FMIN_RIGHT;
			misc_pga_channel_set(ADC1_MODULE, CHANNEL_RIGHT,(AUDIO_ADC_INPUT)gCtrlVars.pga1_r_channel);
			break;

		case 16:///line3 Left gain
			gCtrlVars.line3_l_gain = data > 63? 63 : data;
			gCtrlVars.mic2_l_gain = gCtrlVars.line3_l_gain;
			gCtrlVars.pga1_l_gain = 63 - gCtrlVars.line3_l_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_LINEIN3_LEFT, (uint16_t)gCtrlVars.pga1_l_gain, gCtrlVars.pga1_l_boost);
			// printf("linein3 l gain: %d\n", gCtrlVars.pga1_l_gain);
			break;

		case 17:///line3 Right gain
			gCtrlVars.line3_r_gain = data > 63? 63 : data;
			gCtrlVars.mic1_r_gain = gCtrlVars.line3_r_gain;
			gCtrlVars.pga1_r_gain = 63 - gCtrlVars.line3_r_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_LINEIN3_RIGHT, (uint16_t)gCtrlVars.pga1_r_gain, gCtrlVars.pga1_r_boost);
			// printf("linein3 r gain: %d\n", gCtrlVars.pga1_r_gain);
			break;

		case 18://mic2 gain
			gCtrlVars.mic2_l_gain = data > 63? 63 : data;
			gCtrlVars.line3_l_gain = gCtrlVars.mic2_l_gain;
			gCtrlVars.pga1_l_gain = 63 - gCtrlVars.mic2_l_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_MIC2, (uint16_t)gCtrlVars.pga1_l_gain, gCtrlVars.pga1_l_boost);
			break;

		case 19://mic1 gain
			gCtrlVars.mic1_r_gain = data > 63? 63 : data;
			gCtrlVars.line3_r_gain = gCtrlVars.mic1_r_gain;
			gCtrlVars.pga1_r_gain = 63 - gCtrlVars.mic1_r_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_MIC1, (uint16_t)gCtrlVars.pga1_r_gain, gCtrlVars.pga1_r_boost);
			break;

		case 20:///line2 Left gain
			gCtrlVars.line2_1_l_gain = data > 63? 63 : data;
			gCtrlVars.pga1_l_gain = 63 - gCtrlVars.line2_1_l_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_LINEIN2_LEFT, (uint16_t)gCtrlVars.pga1_l_gain, gCtrlVars.pga1_l_boost);
			break;

		case 21:///line2 Right gain
			gCtrlVars.line2_1_r_gain = data > 63? 63 : data;
			gCtrlVars.pga1_r_gain = 63 - gCtrlVars.line2_1_r_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_LINEIN2_RIGHT, (uint16_t)gCtrlVars.pga1_r_gain, gCtrlVars.pga1_r_boost);
			break;

		case 22:///fm Left gain
			gCtrlVars.fm_l_gain = data > 63? 63 : data;
			gCtrlVars.pga1_l_gain = 63 - gCtrlVars.fm_l_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_FMIN_LEFT, (uint16_t)gCtrlVars.pga1_l_gain, gCtrlVars.pga1_l_boost);
			break;

		case 23:///fm Right gain
			gCtrlVars.fm_r_gain = data > 63? 63 : data;
			gCtrlVars.pga1_r_gain = 63 - gCtrlVars.fm_r_gain;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_FMIN_RIGHT, (uint16_t)gCtrlVars.pga1_r_gain, gCtrlVars.pga1_r_boost);
			break;

		case 24://mic2 gain boost
			data = data > 4? 4 : data;
			gCtrlVars.mic2_l_boost = data;
			if( data == 0) data = 4;
			else data -= 1;
			gCtrlVars.pga1_l_boost = data;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_LEFT, INPUT_MIC2, (uint16_t)gCtrlVars.pga1_l_gain, gCtrlVars.pga1_l_boost);
			break;

		case 25:///mic1 gain boost
			data = data > 4? 4 : data;
			gCtrlVars.mic1_r_boost = data;
			if( data == 0) data = 4;
			else data -= 1;
			gCtrlVars.pga1_r_boost = data;
			AudioADC_PGAGainSet(ADC1_MODULE, CHANNEL_RIGHT, INPUT_MIC1, (uint16_t)gCtrlVars.pga1_r_gain, gCtrlVars.pga1_r_boost);
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_PGA1_0x06(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x06;//cmd
	buffer[3]  = 1 + 18*2 + 8*2;//len
	buffer[4]  = 0xff;///all paramgs

	pp = (int16_t*)&buffer[5];

	//display selection
	*(pp++) = DISP_LINEIN3_LEFT;//linein3 left
	*(pp++) = DISP_LINEIN3_RIGHT;//linein3 right
	*(pp++) = DISP_MIC2;//MIC2
	*(pp++) = DISP_MIC1;//MIC1
	*(pp++) = DISP_LINEIN2_1_LEFT;//linein2 left
	*(pp++) = DISP_LINEIN2_1_RIGHT;//linein2 right
	*(pp++) = DISP_LINEIN4_LEFT;//linein4 left
	*(pp++) = DISP_LINEIN4_RIGHT;//linein4 right
	
	//8
	*(pp++) = !!(gCtrlVars.pga1_l_channel & PGA1_INPUT_LINEIN3_LEFT);
	*(pp++) = !!(gCtrlVars.pga1_r_channel & PGA1_INPUT_LINEIN3_RIGHT);
	*(pp++) = !!(gCtrlVars.pga1_l_channel & PGA1_INPUT_MIC2);
	*(pp++) = !!(gCtrlVars.pga1_r_channel & PGA1_INPUT_MIC1);
	*(pp++) = !!(gCtrlVars.pga1_l_channel & PGA1_INPUT_LINEIN2_LEFT);
	*(pp++) = !!(gCtrlVars.pga1_r_channel & PGA1_INPUT_LINEIN2_RIGHT);
	*(pp++) = !!(gCtrlVars.pga1_l_channel & PGA1_INPUT_FMIN_LEFT);
	*(pp++) = !!(gCtrlVars.pga1_r_channel & PGA1_INPUT_FMIN_RIGHT);

	//16
	*(pp++) =  gCtrlVars.line3_l_gain;//Line3 left gain
	*(pp++) =  gCtrlVars.line3_r_gain;///Line3 right gain

	//18
	*(pp++) =  gCtrlVars.mic2_l_gain;//mic2 gain
	*(pp++) =  gCtrlVars.mic1_r_gain;//mic1 gain

	//20
	*(pp++) = gCtrlVars.line2_1_l_gain;//Line2 left gain
	*(pp++) = gCtrlVars.line2_1_r_gain;//Line2 right gain

	//22
	*(pp++) = gCtrlVars.fm_l_gain;//fm left gain
	*(pp++) = gCtrlVars.fm_r_gain;//fm right gain

	//24
	*(pp++) = gCtrlVars.mic2_l_boost;
	*(pp++) = gCtrlVars.mic1_r_boost;

	buffer[buffer[3] + 4]  = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x06(uint8_t *buf, uint32_t len)//ADC1 PGA
{
	uint32_t TmpData;
	uint16_t i,k;

	if(len == 0) //ask
	{
		int16_t *pp;
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_PGA1_0x06(tx_buf));
		//DBG("ADC1 PGA param required.\n");
	}
	else
	{

		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			// efft_fade_out(FALSE);
			for(i=0; i<26; i++, pp++)
			{
				Comm_PGA1_0x06(i, *pp);
			}
		}
		else
		{
			// if(buf[0]>= 0 && buf[0]<= 7)
			// 	efft_fade_out(FALSE);
			//DBG("PGA1_0x06: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_PGA1_0x06(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x06;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}


void Comm_ADC1_0x07(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)///adc1 digital set
	{
		case 0://ADC1 en?
			// gCtrlVars.adc1_dig_channel_en = data & 0x03;
			//AudioADC_LREnable(ADC1_MODULE, !!(data & 0x01), !!(data & 0x02));
			// efft_fade_out(TRUE);
			break;

		case 1:///ADC1 mute?
			gCtrlVars.adc1_dig_mute = data & 0x03;
			gCtrlVars.adc1_dig_l_mute = gCtrlVars.adc1_dig_mute & 0x01;
			gCtrlVars.adc1_dig_r_mute = (gCtrlVars.adc1_dig_mute & 0x02)>>1;
			AudioADC_DigitalMute(ADC1_MODULE, !!gCtrlVars.adc1_dig_l_mute, !!gCtrlVars.adc1_dig_r_mute);
			break;

		case 2://adc1 dig vol left
			gCtrlVars.adc1_dig_l_vol = data > 0x3fff? 0x3fff : data;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_LEFT,(uint16_t)gCtrlVars.adc1_dig_l_vol);
			break;

		case 3://adc1 dig vol right
			gCtrlVars.adc1_dig_r_vol = data > 0x3fff? 0x3fff : data;
			AudioADC_VolSetChannel(ADC1_MODULE, CHANNEL_RIGHT,(uint16_t)gCtrlVars.adc1_dig_r_vol);
			break;

		case 4://adc1 sample rate
			// if(gCtrlVars.sys_sample_rate_en == 0)
			// {
			// 	gCtrlVars.adc1_sample_rate = SupportSampleRateList[(data > 8)?(7):(data)];
			// 	AudioADC_SampleRateSet(ADC1_MODULE, gCtrlVars.adc1_sample_rate);
			// }
			break;

		case 5://adc1 LR swap
			gCtrlVars.adc1_lr_swap = !!data;
			AudioADC_ChannelSwap(ADC1_MODULE, !!data);
			break;

		case 6://adc1 hight pass
			gCtrlVars.adc1_dc_blocker = (data>2)?(2):(data);
			AudioADC_HighPassFilterConfig(ADC1_MODULE, HPCList[gCtrlVars.adc1_dc_blocker]);
			break;

		case 7://adc1 fade time
			gCtrlVars.adc1_fade_time = data>255?(0):(data);
			if(gCtrlVars.adc1_fade_time==0)
			{
				AudioADC_FadeDisable(ADC1_MODULE);
			}
			else
			{
				AudioADC_FadeEnable(ADC1_MODULE);
				AudioADC_FadeTimeSet(ADC1_MODULE, gCtrlVars.adc1_fade_time);
			}
			break;

		case 8://adc1 mclk src
			// gCtrlVars.adc1_mclk_src = (data> 4)?(0):(data);
			// //-----------------------------------------//
			// if(gCtrlVars.sys_mclk_src_en == 0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_ADC1, (MCLK_CLK_SEL)gCtrlVars.adc1_mclk_src);
			// }
			break;

		case 9://hpc1 en
			gCtrlVars.adc1_dc_blocker_en = !!data;
			if(gCtrlVars.adc1_dc_blocker_en)
			{
				AudioADC_HighPassFilterSet(ADC1_MODULE, TRUE);
			}
			else
			{
				AudioADC_HighPassFilterSet(ADC1_MODULE, FALSE);
			}
			break;

		default:
			break;
		}

}

uint32_t Comm_Form_Rsp_ADC1_0x07(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x07;
	buffer[3]  = 1+10*2;//len
	buffer[4]  = 0xff;

	pp = (int16_t*)&buffer[5];
	*(pp++) =  gCtrlVars.adc1_dig_channel_en;
	*(pp++) =  gCtrlVars.adc1_dig_mute;
	*(pp++) =  gCtrlVars.adc1_dig_l_vol;
	*(pp++) =  gCtrlVars.adc1_dig_r_vol;
	*(pp++) =  comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
	*(pp++) =  gCtrlVars.adc1_lr_swap;
	*(pp++) =  gCtrlVars.adc1_dc_blocker;
	*(pp++) =  gCtrlVars.adc1_fade_time;
	*(pp++) =  gCtrlVars.adc1_mclk_src;
	*(pp++) =  gCtrlVars.adc1_dc_blocker_en;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x07(uint8_t *buf, uint32_t len)///ADC1 DIGITAL
{
	uint16_t i,k;
	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_ADC1_0x07(tx_buf));///25+3*4+1
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<10; i++, pp++)
			{
				Comm_ADC1_0x07(i, *pp);
			}
		}
		else
		{
			//DBG("ADC1_0x07: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_ADC1_0x07(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x07;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

void Comm_AGC1_0x08(int16_t index, int16_t data)
{
	uint32_t TmpData;
	switch(index)//ADC1 AGC
	{
		case 0://AGC {data=0 dis} {data=1 left en} {data=2 right en} {data=3 left+right en}
			gCtrlVars.agc1_mode = data & 0x03;
			AudioADC_AGCChannelSel(ADC1_MODULE, !!(data & 0x01), !!(data & 0x02));
			break;

		case 1://MAX level
			gCtrlVars.agc1_max_level = (data>31)?(31):(data);
			AudioADC_AGCMaxLevel(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_max_level);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 2://target level
			gCtrlVars.agc1_target_level = (data>31)?(31):(data);
			AudioADC_AGCTargetLevel(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_target_level);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 3://max gain
			gCtrlVars.agc1_max_gain = (data>63)?(63):(data);
			AudioADC_AGCMaxGain(ADC1_MODULE,(uint8_t)(63 - gCtrlVars.agc1_max_gain));
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 4://min gain
			gCtrlVars.agc1_min_gain = (data>63)?(63):(data);
			AudioADC_AGCMinGain(ADC1_MODULE,(uint8_t)(63 - gCtrlVars.agc1_min_gain));
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 5://gain offset
			gCtrlVars.agc1_gainoffset = (data>15)?(15):(data);
			AudioADC_AGCGainOffset(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_gainoffset);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 6://frame time
			gCtrlVars.agc1_fram_time = (data>4096)?(4096):(data);
			AudioADC_AGCFrameTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_fram_time);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 7://hold time
			gCtrlVars.agc1_hold_frames = (data>31)?(31):(data);
			AudioADC_AGCHoldFrames(ADC1_MODULE,(uint32_t)gCtrlVars.agc1_hold_frames);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 8://attack time
			gCtrlVars.agc1_attack_time = (data>4096)?(4096):(data);
			AudioADC_AGCAttackStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_attack_time);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 9://dacay time
			gCtrlVars.agc1_decay_time = (data>4096)?(4096):(data);
			AudioADC_AGCDecayStepTime(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_decay_time);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 10://nosie gain en
			gCtrlVars.agc1_noise_gate_en = (data>1)?(1):(data);
			AudioADC_AGCNoiseGateEnable(ADC1_MODULE,(bool)gCtrlVars.agc1_noise_gate_en);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 11://nosie thershold
			gCtrlVars.agc1_noise_threshold = (data>31)?(31):(data);
			AudioADC_AGCNoiseThreshold(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_noise_threshold);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 12://nosie gate mode
			gCtrlVars.agc1_noise_gate_mode = (data>1)?(1):(data);
			AudioADC_AGCNoiseGateMode(ADC1_MODULE,(uint8_t)gCtrlVars.agc1_noise_gate_mode);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		case 13://nosie gate hold time
			gCtrlVars.agc1_noise_hold_frames = (data>31)?(31):(data);
			AudioADC_AGCNoiseHoldFrames(ADC1_MODULE,(uint16_t)gCtrlVars.agc1_noise_hold_frames);
			Reset_AGC_ChannelSel(ADC1_MODULE, gCtrlVars.agc1_mode);
			break;

		default:
			break;
	}

}

uint32_t Comm_Form_Rsp_AGC1_0x08(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x08;
	buffer[3]  = 1+14*2;//len
	buffer[4]  = 0xff;

	pp = (int16_t*)&buffer[5];
	*(pp ++) = gCtrlVars.agc1_mode;
	*(pp ++) = gCtrlVars.agc1_max_level;
	*(pp ++) = gCtrlVars.agc1_target_level;
	*(pp ++) = gCtrlVars.agc1_max_gain;
	*(pp ++) = gCtrlVars.agc1_min_gain;
	*(pp ++) = gCtrlVars.agc1_gainoffset;
	*(pp ++) = gCtrlVars.agc1_fram_time;
	*(pp ++) = gCtrlVars.agc1_hold_frames;
	*(pp ++) = gCtrlVars.agc1_attack_time;
	*(pp ++) = gCtrlVars.agc1_decay_time;
	*(pp ++) = gCtrlVars.agc1_noise_gate_en;
	*(pp ++) = gCtrlVars.agc1_noise_threshold;
	*(pp ++) = gCtrlVars.agc1_noise_gate_mode;
	*(pp ++) = gCtrlVars.agc1_noise_hold_frames;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x08(uint8_t *buf, uint32_t len)////ADC1 AGC
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_AGC1_0x08(tx_buf));
	}
	else
	{
		// efft_fade_out(TRUE);
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<14; i++, pp++)
			{
				Comm_AGC1_0x08(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_AGC1_0x08(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x08;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}




//DAC
void Comm_DAC0_0x09(int16_t index, int16_t data)
{
    uint32_t TmpData;

	switch(index)////DAC0 set
	{
		case 0://DAC0 en
			// gCtrlVars.dac0_en = data;
			// efft_fade_out(TRUE);
			break;

		case 1:
			// if(gCtrlVars.sys_sample_rate_en == 0)
			// {
			// 	gCtrlVars.dac0_sample_rate = (data>8)?(8):(data);
			// 	AudioDAC_SampleRateSet(DAC0,SupportSampleRateList[gCtrlVars.dac0_sample_rate]);
			// }
			break;

		case 2:///dac0 mute
			gCtrlVars.dac0_dig_mute = data;
			gCtrlVars.dac0_dig_l_mute = !!(data & 0x01);
			gCtrlVars.dac0_dig_r_mute = !!(data & 0x02);
			AudioDAC_DigitalMute(DAC0, gCtrlVars.dac0_dig_l_mute, gCtrlVars.dac0_dig_r_mute);
			break;

		case 3:////dac0 L volume
			gCtrlVars.dac0_dig_l_vol = (data>0x3fff)?(0x3fff):(data);
			AudioDAC_VolSet(DAC0, gCtrlVars.dac0_dig_l_vol, gCtrlVars.dac0_dig_r_vol);
			break;

		case 4:////dac0 R volume
			gCtrlVars.dac0_dig_r_vol = (data>0x3fff)?(0x3fff):(data);
			AudioDAC_VolSet(DAC0, gCtrlVars.dac0_dig_l_vol, gCtrlVars.dac0_dig_r_vol);
			break;

		case 5:///DAC0 dither
			gCtrlVars.dac0_dither = data << 7;
			if(gCtrlVars.dac0_dither)
			{
				AudioDAC_DitherEnable(DAC0);
				AudioDAC_DitherPowSet(DAC0,gCtrlVars.dac0_dither);
			}
			else
			{
				AudioDAC_DitherDisable(DAC0);
			}
			break;

		case 6:///dac0 scramble
			gCtrlVars.dac0_scramble = (data>3)?(3):(data);
			if(gCtrlVars.dac0_scramble == 0)
			{
				AudioDAC_ScrambleDisable(DAC0);
			}
			else
			{
				AudioDAC_ScrambleEnable(DAC0);
				AudioDAC_ScrambleModeSet(DAC0,(SCRAMBLE_MODULE)gCtrlVars.dac0_scramble);
			}
			break;

		case 7:///dac0 stere mode
			gCtrlVars.dac0_out_mode = (data>3)?(3):(data);
			//DBG("dac 0 out mode: %d\n", gCtrlVars.dac0_out_mode);
			AudioDAC_DoutModeSet((gCtrlVars.dac0_out_mode == 0)?(0):(gCtrlVars.dac0_out_mode+1));
			break;

		case 8:///dac0 pause
			// gCtrlVars.dac0_pause_en = !!data;
			// if(gCtrlVars.dac0_pause_en)
			// {
			// 	gCtrlVars.dac0_dig_l_mute = 0x01;
			// 	gCtrlVars.dac0_dig_r_mute = 0x01;
			// }
			// else
			// {
			// 	gCtrlVars.dac0_dig_l_mute = 0x00;
			// 	gCtrlVars.dac0_dig_r_mute = 0x00;
			// }
			
			// AudioDAC_DigitalMute(DAC0, gCtrlVars.dac0_dig_l_mute, gCtrlVars.dac0_dig_r_mute);
			break;

		case 9:///dac0 sample mode
			gCtrlVars.dac0_sample_mode = !!data;
			AudioDAC_EdgeSet(DAC0,gCtrlVars.dac0_sample_mode);
			break;

		case 10:///dac0 scf mute
			gCtrlVars.dac0_scf_mute = data & 0x03;
			AudioDAC_SCFMute(DAC0, data&0x01, data&0x02>>1);
			break;

		case 11:///dac0 fade time
			gCtrlVars.dac0_fade_time = data;
			if(gCtrlVars.dac0_fade_time == 0)
			{
				AudioDAC_FadeDisable(DAC0);
			}
			else
			{
				AudioDAC_FadeEnable(DAC0);
				AudioDAC_FadeTimeSet(DAC0,gCtrlVars.dac0_fade_time);
			}
			break;

		case 12:///dac0 zero num
			gCtrlVars.dac0_zeros_number = (data>7)?(7):(data);
			AudioDAC_ZeroNumSet(DAC0, gCtrlVars.dac0_zeros_number);
			break;

		case 13:///dac0 mclk src
			// gCtrlVars.dac0_mclk_src = (data> 4)?(0):(data);
			// if(gCtrlVars.sys_mclk_src_en == 0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_DAC0, (MCLK_CLK_SEL)gCtrlVars.dac0_mclk_src);
			// }
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_DAC0_0x09(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x09;
	buffer[3]  = 1+14*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = gCtrlVars.dac0_en;
	*(pp++) = comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
	*(pp++) = gCtrlVars.dac0_dig_mute;
	*(pp++) = gCtrlVars.dac0_dig_l_vol;
	*(pp++) = gCtrlVars.dac0_dig_r_vol;
	*(pp++) = gCtrlVars.dac0_dither >> 7;
	*(pp++) = gCtrlVars.dac0_scramble;
	*(pp++) = gCtrlVars.dac0_out_mode;
	*(pp++) = gCtrlVars.dac0_pause_en;
	*(pp++) = gCtrlVars.dac0_sample_mode;
	*(pp++) = gCtrlVars.dac0_scf_mute;
	*(pp++) = gCtrlVars.dac0_fade_time;
	*(pp++) = gCtrlVars.dac0_zeros_number;
	*(pp++) = gCtrlVars.dac0_mclk_src;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x09(uint8_t *buf, uint32_t len)///DAC0
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_DAC0_0x09(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<14; i++, pp++)
			{
				Comm_DAC0_0x09(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_DAC0_0x09(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x09;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

void Comm_DAC1_0x0A(int16_t index, int16_t data)
{
    uint32_t TmpData;

	switch(index)////DAC1 set
	{
		case 0://DAC1 en
			// gCtrlVars.dac1_en = !!data;
			// efft_fade_out(TRUE);
			break;

		case 1:
			// if(gCtrlVars.sys_sample_rate_en == 0)
			// {
			// 	gCtrlVars.dac1_sample_rate = (data>8)?(8):(data);
			// 	AudioDAC_SampleRateSet(DAC1,SupportSampleRateList[gCtrlVars.dac1_sample_rate]);
			// }
			break;

		case 2:///dac1 mute
			gCtrlVars.dac1_dig_mute = !!data;
			AudioDAC_DigitalMute(DAC1, gCtrlVars.dac1_dig_mute, gCtrlVars.dac1_dig_mute);
			break;

		case 3:////dac1 L volume
			gCtrlVars.dac1_dig_vol = (data>0x3fff)?(0x3fff):(data);
			AudioDAC_VolSet(DAC1, gCtrlVars.dac1_dig_vol, gCtrlVars.dac1_dig_vol);
			break;

		case 4:////dac1 R volume
			gCtrlVars.dac1_dig_vol = (data>0x3fff)?(0x3fff):(data);
			AudioDAC_VolSet(DAC1, gCtrlVars.dac1_dig_vol, gCtrlVars.dac1_dig_vol);
			break;

		case 5:///DAC1 dither
			gCtrlVars.dac1_dither = data << 7;
			if(gCtrlVars.dac1_dither)
			{
				AudioDAC_DitherEnable(DAC1);
				AudioDAC_DitherPowSet(DAC1,gCtrlVars.dac1_dither);
			}
			else
			{
				AudioDAC_DitherDisable(DAC1);
			}
			break;

		case 6:///dac1 scramble
			gCtrlVars.dac1_scramble = (data>3)?(3):(data);
			if(gCtrlVars.dac1_scramble == 0)
			{
				AudioDAC_ScrambleDisable(DAC1);
			}
			else
			{
				AudioDAC_ScrambleEnable(DAC1);
				AudioDAC_ScrambleModeSet(DAC1,(SCRAMBLE_MODULE)gCtrlVars.dac1_scramble);
			}
			break;

		case 7:///dac1 stere mode
			gCtrlVars.dac1_out_mode = (data>4)?(4):(data);
			break;

		case 8:///dac1 pause
			// gCtrlVars.dac1_pause_en = !!data;
			// if(gCtrlVars.dac1_pause_en)
			// {
			// 	gCtrlVars.dac1_dig_mute = 0x01;
			// }
			// else
			// {
			// 	gCtrlVars.dac1_dig_mute = 0x00;
			// }

			// AudioDAC_DigitalMute(DAC1, gCtrlVars.dac1_dig_mute, gCtrlVars.dac1_dig_mute);
			break;

		case 9:///dac1 sample mode
			gCtrlVars.dac1_sample_mode = !!data;
			AudioDAC_EdgeSet(DAC1, gCtrlVars.dac1_sample_mode);
			break;

		case 10:///dac1 scf mute
			gCtrlVars.dac1_scf_mute = data & 0x03;
			AudioDAC_SCFMute(DAC1, data&0x01, data&0x02);
			break;

		case 11:///dac0 fade time
			gCtrlVars.dac1_fade_time = data;
			if(gCtrlVars.dac1_fade_time == 0)
			{
				AudioDAC_FadeDisable(DAC1);
			}
			else
			{
				AudioDAC_FadeEnable(DAC1);
				AudioDAC_FadeTimeSet(DAC1,gCtrlVars.dac1_fade_time);
			}
			break;

		case 12:///dac0 zero num
			gCtrlVars.dac1_zeros_number = (data>7)?(7):(data);
			AudioDAC_ZeroNumSet(DAC1, gCtrlVars.dac1_zeros_number);
			break;

		case 13:///dac1 mclk src
			// gCtrlVars.dac1_mclk_src =  (data> 4)?(0):(data);
			// if(gCtrlVars.sys_mclk_src_en == 0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_DAC1, (MCLK_CLK_SEL)gCtrlVars.dac1_mclk_src);
			// }
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_DAC1_0x0A(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0a;
	buffer[3]  = 1+14*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = gCtrlVars.dac1_en;
	*(pp++) = comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
	*(pp++) = gCtrlVars.dac1_dig_mute;
	*(pp++) = gCtrlVars.dac1_dig_vol;
	*(pp++) = gCtrlVars.dac1_dig_vol;
	*(pp++) = gCtrlVars.dac1_dither >> 7;
	*(pp++) = gCtrlVars.dac1_scramble;
	*(pp++) = gCtrlVars.dac1_out_mode;
	*(pp++) = gCtrlVars.dac1_pause_en;
	*(pp++) = gCtrlVars.dac1_sample_mode;
	*(pp++) = gCtrlVars.dac1_scf_mute;
	*(pp++) = gCtrlVars.dac1_fade_time;
	*(pp++) = gCtrlVars.dac1_zeros_number;
	*(pp++) = gCtrlVars.dac1_mclk_src;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x0A(uint8_t *buf, uint32_t len)//DACX
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_DAC1_0x0A(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<14; i++, pp++)
			{
				Comm_DAC1_0x0A(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_DAC1_0x0A(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0A;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

//I2S0
void Comm_I2S0_0x0B(int16_t index, int16_t data)
{
    uint32_t TmpData;

	switch(index)////i2s0  set
	{
		case 0:
			// gCtrlVars.i2s0_tx_en = data&0x01;
			break;

		case 1:
			// gCtrlVars.i2s0_rx_en = data&0x01;
			break;

		case 2:///sample rate
			// if(gCtrlVars.sys_sample_rate_en==0)
			// {
			// 	gCtrlVars.i2s0_sample_rate = SupportSampleRateList[(data>12)?(12):(data)];
			// 	I2S_SampleRateSet(AUDIO_I2S0, gCtrlVars.i2s0_sample_rate);
			// }
			break;

		case 3://mclk src
			// gCtrlVars.i2s0_mclk_src = (data>4)?(4):(data);
			// if(gCtrlVars.sys_mclk_src_en ==0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_I2S0, (MCLK_CLK_SEL)gCtrlVars.i2s0_mclk_src);
			// }
			break;

		case 4:///master slave
			// gCtrlVars.i2s0_work_mode = data&0x01;
			// if(gCtrlVars.i2s0_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S0_MODULE, gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S0_MODULE,gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			break;

		case 5:///word lenght
			// gCtrlVars.i2s0_word_len = data&0x01;
			// if(gCtrlVars.i2s0_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S0_MODULE, gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S0_MODULE,gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			break;

		case 6:///stereo mono
			// gCtrlVars.i2s0_mono = data&0x01;
			// I2S_MonoModeSet(I2S0_MODULE,gCtrlVars.i2s0_mono);
			break;

		case 7:///fade time
			// gCtrlVars.i2s0_fade_time = (data>255)?(255):(data);
			// if(gCtrlVars.i2s0_fade_time==0)
			// {
			// 	I2S_FadeDisable(I2S0_MODULE);
			// }
			// else
			// {
			// 	I2S_FadeEnable(I2S0_MODULE);
			// 	I2S_FadeTimeSet(I2S0_MODULE,gCtrlVars.i2s0_fade_time);
			// }
			break;

		case 8:///i2s0 format
			// gCtrlVars.i2s0_format = (data>4)?(4):(data);
			// if(gCtrlVars.i2s0_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S0_MODULE, gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S0_MODULE,gCtrlVars.i2s0_format, gCtrlVars.i2s0_word_len);
			// }
			break;

		case 9:///i2s0 bclk invert
			// gCtrlVars.i2s0_bclk_invert_en = data & 0x01;
			// I2S_BclkInvertSet(I2S0_MODULE,(bool)gCtrlVars.i2s0_bclk_invert_en);
			break;

		case 10:///i2s0 bclk invert
			// gCtrlVars.i2s0_lrclk_invert_en = data & 0x01;
			// I2S_LrclkInvertSet(I2S0_MODULE,(bool)gCtrlVars.i2s0_lrclk_invert_en);
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_I2S0_0x0B(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0b;
	buffer[3]  = 1+11*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = gCtrlVars.i2s0_tx_en;
	*(pp++) = gCtrlVars.i2s0_rx_en;
	*(pp++) = comm_ret_sample_rate_enum(48000);//g_user_effect_list->sample_rate
	*(pp++) = gCtrlVars.i2s0_mclk_src;
	*(pp++) = gCtrlVars.i2s0_work_mode;
	*(pp++) = gCtrlVars.i2s0_word_len;
	*(pp++) = gCtrlVars.i2s0_mono;
	*(pp++) = gCtrlVars.i2s0_fade_time;
	*(pp++) = gCtrlVars.i2s0_format;
	*(pp++) = gCtrlVars.i2s0_bclk_invert_en;
	*(pp++) = gCtrlVars.i2s0_lrclk_invert_en;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x0B(uint8_t *buf, uint32_t len)////I2S0
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));

		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_I2S0_0x0B(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<11; i++, pp++)
			{
				Comm_I2S0_0x0B(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_I2S0_0x0B(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0B;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

//I2S1
void Comm_I2S1_0x0C(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)////i2s0  set
	{
		case 0:
			// gCtrlVars.i2s1_tx_en = data&0x01;
			// if(gCtrlVars.i2s1_tx_en)
			// 	I2S_ModuleTxEnable(I2S1_MODULE);
			// else
			// 	I2S_ModuleTxDisable(I2S1_MODULE);
			break;

		case 1:
			// gCtrlVars.i2s1_rx_en = data&0x01;
			// if(gCtrlVars.i2s1_rx_en)
			// 	I2S_ModuleRxEnable(I2S1_MODULE);
			// else
			// 	I2S_ModuleRxDisable(I2S1_MODULE);
			break;

		case 2:///sample rate
			// if(gCtrlVars.sys_sample_rate_en==0)
			// {
			// 	gCtrlVars.i2s1_sample_rate = SupportSampleRateList[(data>12)?(12):(data)];
			// 	I2S_SampleRateSet(AUDIO_I2S1, gCtrlVars.i2s1_sample_rate);
			// }
			break;

		case 3://mclk src
			// gCtrlVars.i2s1_mclk_src = (data>4)?(4):(data);
			// if(gCtrlVars.sys_mclk_src_en ==0)
			// {
			// 	Clock_AudioMclkSel(AUDIO_I2S1, (MCLK_CLK_SEL)gCtrlVars.i2s1_mclk_src);
			// }
			break;

		case 4:///master slave
			// gCtrlVars.i2s1_work_mode = data&0x01;
			// if(gCtrlVars.i2s1_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S1_MODULE, gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S1_MODULE,gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			break;

		case 5:///word lenght
			// gCtrlVars.i2s1_word_len = data&0x01;
			// if(gCtrlVars.i2s1_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S1_MODULE, gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S1_MODULE,gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			break;

		case 6:///stereo mono
			// gCtrlVars.i2s1_mono = data&0x01;
			// I2S_MonoModeSet(I2S1_MODULE,gCtrlVars.i2s1_mono);
			break;

		case 7:///fade time
			// gCtrlVars.i2s1_fade_time = (data>255)?(255):(data);
			// if(gCtrlVars.i2s1_fade_time==0)
			// {
			// 	I2S_FadeDisable(I2S1_MODULE);
			// }
			// else
			// {
			// 	I2S_FadeEnable(I2S1_MODULE);
			// 	I2S_FadeTimeSet(I2S1_MODULE,gCtrlVars.i2s0_fade_time);
			// }
			break;

		case 8:///i2s1 format
			// gCtrlVars.i2s1_format = (data>4)?(4):(data);
			// if(gCtrlVars.i2s1_work_mode)
			// {
			// 	I2S_SetSlaveMode(I2S1_MODULE, gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			// else
			// {
			// 	I2S_SetMasterMode(I2S1_MODULE,gCtrlVars.i2s1_format, gCtrlVars.i2s1_word_len);
			// }
			break;

		case 9:///i2s1 bclk invert
			// gCtrlVars.i2s1_bclk_invert_en = data & 0x01;
			// I2S_BclkInvertSet(I2S1_MODULE,(bool)gCtrlVars.i2s1_bclk_invert_en);
			break;

		case 10:///i2s0 bclk invert
			// gCtrlVars.i2s1_lrclk_invert_en = data & 0x01;
			// I2S_LrclkInvertSet(I2S1_MODULE,(bool)gCtrlVars.i2s1_lrclk_invert_en);
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_I2S1_0x0C(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0c;
	buffer[3]  = 1+11*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = gCtrlVars.i2s1_tx_en;
	*(pp++) = gCtrlVars.i2s1_rx_en;
	*(pp++) = comm_ret_sample_rate_enum(48000);//g_user_effect_list->sample_rate
	*(pp++) = gCtrlVars.i2s1_mclk_src;
	*(pp++) = gCtrlVars.i2s1_work_mode;
	*(pp++) = gCtrlVars.i2s1_word_len;
	*(pp++) = gCtrlVars.i2s1_mono;
	*(pp++) = gCtrlVars.i2s1_fade_time;
	*(pp++) = gCtrlVars.i2s1_format;
	*(pp++) = gCtrlVars.i2s1_bclk_invert_en;
	*(pp++) = gCtrlVars.i2s1_lrclk_invert_en;

	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x0C(uint8_t *buf, uint32_t len)////I2S1
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));

		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_I2S1_0x0C(tx_buf));
	}
	else
	{
		if(buf[0] == 0xff)
		{
			int16_t *pp = (int16_t*)&buf[1];
			for(i=0; i<11; i++, pp++)
			{
				Comm_I2S1_0x0C(i, *pp);
			}
		}
		else
		{
			//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
			Comm_I2S1_0x0C(buf[0], *((int16_t*)&buf[1]));
		}

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0C;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

//SPDIF
void Comm_SPDIF_0x0D(int16_t index, int16_t data)
{
	uint32_t TmpData;

	switch(index)////i2s0  set
	{
		case 0:
			// gCtrlVars.spdif_en = data & 0x01;
			// if(!is_audio_i2s0_tx_on() && !is_audio_i2s0_rx_on())
			// 	;//spdif_sample_rate = 0;//force to check sample rate
			// else
			// 	gCtrlVars.spdif_en = 0x00;
			break;

		case 1://sample rate
			//only spdif rx, ignore
			//gCtrlVars.spdif_sample_rate = SupportSampleRateList[(data>12)?(12):(data)];
			break;

		case 2:///channel mode
			//gCtrlVars.spdif_ch_mode = (data>2)?(2):(data);
			// gCtrlVars.spdif_ch_mode = 0;//only support stereo
			break;

		case 3://
			// gCtrlVars.spdif_io_sel = (data>1)?(1):(data);
			break;

		default:
			break;
	}
}

uint32_t Comm_Form_Rsp_SPDIF_0x0D(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0d;
	buffer[3]  = 1+5*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = gCtrlVars.spdif_en;
	*(pp++) = gCtrlVars.spdif_sample_rate;
	*(pp++) = gCtrlVars.spdif_ch_mode;
	*(pp++) = gCtrlVars.spdif_io_sel;
	*(pp++) = 0;//gCtrlVars.spdif_lock_status;//
	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x0D(uint8_t *buf, uint32_t len)////SPDIF
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_SPDIF_0x0D(tx_buf));
	}
	else
	{
		// if(buf[0] == 0xff)
		// {
		// 	int16_t *pp = (int16_t*)&buf[1];
		// 	for(i=0; i<4; i++, pp++)
		// 	{
		// 		Comm_SPDIF_0x0D(i, *pp);
		// 	}
		// }
		// else
		// {
		// 	//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
		// 	Comm_SPDIF_0x0D(buf[0], *((int16_t*)&buf[1]));
		// }

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0D;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

//GPIO
void Comm_GPIO_Init(void)
{
	// GPIO_RegOneBitSet(DU_GPIO_0_IE, DU_GPIO_0);
	// GPIO_RegOneBitClear(DU_GPIO_0_OE, DU_GPIO_0);
	// GPIO_RegOneBitClear(DU_GPIO_0_PU, DU_GPIO_0);
	// GPIO_RegOneBitClear(DU_GPIO_0_PD, DU_GPIO_0);
	// GPIO_RegOneBitClear(DU_GPIO_0_DS, DU_GPIO_0);

	// GPIO_RegOneBitSet(DU_GPIO_0_IE, DU_GPIO_1);
	// GPIO_RegOneBitClear(DU_GPIO_0_OE, DU_GPIO_1);
	// GPIO_RegOneBitClear(DU_GPIO_0_PU, DU_GPIO_1);
	// GPIO_RegOneBitClear(DU_GPIO_0_PD, DU_GPIO_1);
	// GPIO_RegOneBitClear(DU_GPIO_0_DS, DU_GPIO_1);
}

void Comm_GPIO_0x0E(int16_t index, int16_t *data)
{
	
}


uint32_t Comm_Form_Rsp_GPIO_0x0E(uint8_t *buffer)
{
	int16_t *pp;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0e;
	buffer[3]  = 1+6*2;
	buffer[4]  = 0xff;
	pp = (int16_t*)&buffer[5];

	*(pp++) = 0;
	*(pp++) = 0;
	*(pp++) = 0;

	*(pp++) = 0;
	*(pp++) = 0;
	*(pp++) = 0;
	buffer[buffer[3] + 4] = 0x16;

	return buffer[3] + 5;
}


void Communication_Effect_0x0E(uint8_t *buf, uint32_t len)////GPIO Config
{
	uint16_t i,k;

	if(len == 0) //ask
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_GPIO_0x0E(tx_buf));
	}
	else
	{
		// if(buf[0] == 0xff)
		// {
		// 	int16_t *pp = (int16_t*)&buf[1];
		// 	for(i=0; i<2; i++, pp+=3)
		// 	{
		// 		Comm_GPIO_0x0E(i, pp);
		// 	}
		// }
		// else
		// {
		// 	//DBG("ADC0_0x04: %d, %d.\n", buf[0], *((int32_t*)&buf[1]));
		// 	Comm_GPIO_0x0E(buf[0], ((int16_t*)&buf[1]));
		// }

		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0E;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}
}

uint32_t Comm_Form_Rsp_GPIO_0x0F(uint8_t *buffer)
{
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0x0f;
	buffer[3]  = 1+2*2;
	buffer[4]  = 0xff;
	*((int16_t*)&buffer[5]) = 0;
	*((int16_t*)&buffer[7]) = 0;
	buffer[9]  = 0x16;
	return buffer[3] + 5;
}


void Communication_Effect_0x0F(uint8_t *buf, uint32_t len)////GPIO Write
{
	if(len == 0)
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_GPIO_0x0F(tx_buf));
	}
	else
	{
		// if(need_rsp)
		{
			uint8_t rsp_byte = 0x0F;
			Communication_Effect_Send(&rsp_byte, 1);
		}
	}

}

void Communication_Effect_0x10(uint8_t *buf, uint32_t len)////GPIO Read
{
	int16_t *pp;
	if(len == 0)
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x10;
		tx_buf[3]  = 1+2*2;
		tx_buf[4]  = 0xff;
		pp = (int16_t*)&tx_buf[5];

		*(pp++) = 0;
		*(pp++) = 0;

		tx_buf[tx_buf[3] + 4] = 0x16;
		Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
		//DBG("gpio status: %d, %d\n", GPIO_RegOneBitGet(DU_GPIO_0_IN, DU_GPIO_0), GPIO_RegOneBitGet(DU_GPIO_1_IN, DU_GPIO_1));
	}
}

void Communication_Effect_0x12(uint8_t *buf, uint32_t len)////GPIO Read
{
	int16_t *pp;
	int8_t *byte_pp;
	if(len == 0)
	{
		memset(tx_buf, 0, sizeof(tx_buf));
		tx_buf[0]  = 0xa5;
		tx_buf[1]  = 0x5a;
		tx_buf[2]  = 0x12;

		tx_buf[3]  = 5;
		byte_pp = (int8_t*)&tx_buf[4];

		*(byte_pp++) = 0;//multi-graphic
		*(byte_pp++) = 0;//multi-mode
		*(byte_pp++) = 0;//support parambin
		*(byte_pp++) = 0;//support graphic download
		*(byte_pp++) = 0;//support preset param download

		tx_buf[tx_buf[3] + 4] = 0x16;
		Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
		//DBG("gpio status: %d, %d\n", GPIO_RegOneBitGet(DU_GPIO_0_IN, DU_GPIO_0), GPIO_RegOneBitGet(DU_GPIO_1_IN, DU_GPIO_1));
	}
}

/********************************hardware control panel********************************/





void Communication_Effect_0x80(uint8_t *buf, uint32_t len)
{
	if(is_encrypt_lock)//!make fun
	{
		roboeffect_effect_enquiry_stream(buf, len);
	}
}

void Communication_Effect_0xfe(uint8_t *buf, uint32_t len)
{
	int32_t ret, i;
	
	tx_buf[0] = 0xa5;
	tx_buf[1] = 0x5a;
	tx_buf[2] = 0xfe;
	tx_buf[3] = 0x01;
	tx_buf[4] = 0x00;
	tx_buf[5] = 0x16;

	Communication_Effect_Send(&tx_buf[2], 1);

	ota_upgrade_count = 6000;
}

#ifdef NEED_ENCRYPTED
static const uint8_t encrypt_key[] = ENCRYPTED_KEY;
#endif

void Communication_Effect_0xff(uint8_t *buf, uint32_t len)
{
	int32_t ret, i;
	
	if(len == 0)//enquiry if encrypted or not
	{
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = 0xff;
		tx_buf[3] = 0x03;
		tx_buf[4] = 0x00;
		tx_buf[5] = ENCRYPTED_FLAG;
		tx_buf[6] = 0x00;
		tx_buf[7] = 0x16;
		Communication_Effect_Send(&tx_buf[0], 8);
	}
#ifdef NEED_ENCRYPTED
	else if(len == 4)
	{
		tx_buf[0] = 0xa5;
		tx_buf[1] = 0x5a;
		tx_buf[2] = 0xff;
		tx_buf[3] = 0x03;
		tx_buf[4] = 0x01;
		tx_buf[5] = 0x00;
		tx_buf[6] = 0x00;
		tx_buf[7] = 0x16;
		// printf("0x%02X:0x%02X\n", buf[0], encrypt_key[3]);
		// printf("0x%02X:0x%02X\n", buf[1], encrypt_key[2]);
		// printf("0x%02X:0x%02X\n", buf[2], encrypt_key[1]);
		// printf("0x%02X:0x%02X\n", buf[3], encrypt_key[0]);
		if(buf[0] == encrypt_key[3] && buf[1] == encrypt_key[2] && buf[2] == encrypt_key[1] && buf[3] == encrypt_key[0])
		{
			is_encrypt_lock = TRUE;
			tx_buf[5] = 0x01;
		}
		else
		{
			is_encrypt_lock = FALSE;
			tx_buf[5] = 0x00;
		}
		Communication_Effect_Send(&tx_buf[0], 8);
	}
#endif/*NEED_ENCRYPTED*/
	
}

static char user_data[USER_DATA_LEN] = {0};
static uint16_t user_data_len = 0;
uint32_t Comm_Form_Rsp_USERDATA_0xFC(uint8_t *buffer)
{
	char chip_id_str[16] = {0};
	uint64_t ChipID;
	buffer[0]  = 0xa5;
	buffer[1]  = 0x5a;
	buffer[2]  = 0xfc;
#if 1
	buffer[3]  = user_data_len;
	memcpy(&buffer[4], user_data, user_data_len);
#else
	Chip_IDGet(&ChipID);
	sprintf(chip_id_str, "ChipID = 0x%llX", ChipID);
	buffer[3]  = strlen(chip_id_str);
	memcpy(&buffer[4], chip_id_str, strlen(chip_id_str));
#endif
	buffer[buffer[3] + 4]  = 0x16;
	return buffer[3] + 5;
}

void hardware_pipe_reset(uint32_t SampleRate, uint32_t FrameSize);


#if 0
void Communication_Effect_0xFB(uint8_t *buf, uint32_t len)
{
	uint8_t rsp_byte = 0xfc;
	uint8_t num = buf[0], *addr = &(buf[1]), out_num = 0;
	uint32_t *ptr_size = (uint32_t *)&(tx_buf[5]);

	tx_buf[0] = 0xa5;
	tx_buf[1] = 0x5a;
	tx_buf[2] = 0xfb;

	for(int i = 0; i < num; i++)
	{
		int32_t t_size = 0, ret = 0;

		//*memory size
		ret = roboeffect_get_effect_size(context_memory, addr[i], &t_size);
		// printf("-->0x%02X: %d\n", addr[i], t_size);
		if(ret < 0) break;//error
		*ptr_size = t_size;
		ptr_size++;

		//*frame size
		t_size = roboeffect_recommend_frame_size_upon_effect_change(context_memory, g_user_effect_list->frame_size, addr[i], 1);
		if(t_size < 0) break;//error
		*ptr_size = t_size;
		ptr_size++;

		out_num++;
	}
	tx_buf[3] = 1 + out_num * 4 * 2;//! *2 for <memory_size> and <frame_size>
	tx_buf[4] = out_num;
	tx_buf[tx_buf[3] + 4] = 0x16;

	// printf("send %d: ", tx_buf[3] + 5);
	// for(int i = 0; i<tx_buf[3] + 5; i++)
	// {
	// 	printf("%02X ", tx_buf[i]);
	// }
	// printf("\n");

	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
}
#else
void Communication_Effect_0xFB(uint8_t *buf, uint32_t len)
{
	uint8_t rsp_byte = 0xfc;
	uint8_t num = buf[0], *addr = &(buf[1]), out_num = 0;
	uint32_t *ptr_size = (uint32_t *)&(tx_buf[5]);

	if(context_memory == NULL)
		return;

	tx_buf[0] = 0xa5;
	tx_buf[1] = 0x5a;
	tx_buf[2] = 0xfb;

	for(int i = 0; i < num; i++)
	{
		int32_t t_size = 0, ret = 0;
		ret = roboeffect_get_effect_size(context_memory, addr[i], &t_size);
		// printf("-->0x%02X: %d\n", addr[i], t_size);
		if(ret < 0) break;//error
		*ptr_size = t_size;
		ptr_size++;
		out_num++;
	}
	tx_buf[3] = 1 + out_num * 4;
	tx_buf[4] = out_num;
	tx_buf[tx_buf[3] + 4] = 0x16;

	// printf("send %d: ", tx_buf[3] + 5);
	// for(int i = 0; i<tx_buf[3] + 5; i++)
	// {
	// 	printf("%02X ", tx_buf[i]);
	// }
	// printf("\n");

	Communication_Effect_Send(tx_buf, tx_buf[3] + 5);
}
#endif

void Communication_Effect_0xFC(uint8_t *buf, uint32_t len)
{
	uint8_t rsp_byte = 0xfc;
	if(len == 0)
	{
		Communication_Effect_Send(tx_buf, Comm_Form_Rsp_USERDATA_0xFC(tx_buf));
	}
	else
	{
		if(buf[0] == 'a')
		{
			need_switch_mode = TRUE;
		}
		else if(buf[0] == 'b')
		{
			need_switch_mode = TRUE;
		}
		else if(buf[0] == 'r')
		{
			//printf("Clear DMA.\n");
			// hardware_pipe_reset(g_user_effect_list->sample_rate, 0);
		}
		memset(user_data, 0x00, USER_DATA_LEN);
		memcpy(user_data, buf, (len>USER_DATA_LEN)?(USER_DATA_LEN):(len));
		user_data_len = (len>USER_DATA_LEN)?(USER_DATA_LEN):(len);
		
		Communication_Effect_Send(&rsp_byte, 1);
	}
}



void Communication_Effect_Config(uint8_t Control, uint8_t *buf, uint32_t len)
{
	// printf("see:%02X\n", Control);
	switch(Control)
	{
		case 0x00:
			Communication_Effect_0x00(buf, len);
			break;

		case 0x01:
			Communication_Effect_0x01(buf, len);
			break;

		case 0x02:
			Communication_Effect_0x02();
			break;

/*----------------------------codec hardware control-----------------------------*/
		case 0x03:
			Communication_Effect_0x03(buf, len);
			break;

		case 0x04:
			Communication_Effect_0x04(buf, len);
			break;

		case 0x05:
			Communication_Effect_0x05(buf, len);
			break;

		case 0x06:
			Communication_Effect_0x06(buf, len);
			break;

		case 0x07:
			Communication_Effect_0x07(buf, len);
			break;

		case 0x08:
			Communication_Effect_0x08(buf, len);
			break;

		case 0x09:
			Communication_Effect_0x09(buf, len);
			break;

		case 0x0A:
			Communication_Effect_0x0A(buf, len);
			break;

		case 0x0B:
			Communication_Effect_0x0B(buf, len);
			break;

		case 0x0C:
			Communication_Effect_0x0C(buf, len);
			break;

		case 0x0D:
			Communication_Effect_0x0D(buf, len);
			break;

		case 0x0E:
			Communication_Effect_0x0E(buf, len);
			break;

		case 0x0F:
			Communication_Effect_0x0F(buf, len);
			break;

		case 0x10:
			Communication_Effect_0x10(buf, len);
			break;

		case 0x12:
			Communication_Effect_0x12(buf, len);
			break;

/*----------------------------codec hardware control-----------------------------*/


		case ROBOEFFECT_START_ADDR:
			if(!robo_init_err)
			{
				Communication_Effect_0x80(buf, len);
			}
			break;

		case 0xfb:
			Communication_Effect_0xFB(buf, len);
			break;

		case 0xfc:
			Communication_Effect_0xFC(buf, len);
			break;

		case 0xfe:
			Communication_Effect_0xfe(buf, len);
			break;

		case 0xff:
			Communication_Effect_0xff(buf, len);
			break;

		default:
			if(!robo_init_err)
			{
				if((Control >= ROBOEFFECT_FIRST_ADDR) && (Control < 0xfb))
				{
					if(is_encrypt_lock)//!make fun
					{
						roboeffect_effect_update_params_entrance(Control, buf, len);
					}
				}
				else
				{

				}
			}
			break;
	}
}


/***************************************I2C****************************************************/

bool robo_iic_is_system_qurey_cmd(uint8_t *buf)
{
	// return FALSE;

	if(buf[0] == 0xFD || buf[0] == 0xFF)
		return FALSE;
		
	if(buf[1] != 0)// len != 0 is not a qurey cmd
		return FALSE;

	return TRUE;
}



__attribute__((section(".iic_isr")))
void I2C_Interrupt(void)
{
	static uint8_t abyte = 0, rw_flag = 0;

	if(I2C_IsAddressMatched())
	{
		//I2C_SlaveConfig();
		if(I2C_SlaveReadWriteGet())
		{
			//DBG("*");
			I2C_IntEn(1);
			//send_buf(SendBuf,DataLen);
			rw_flag=1;
			I2C_SendStart();
			I2C_SendByte(hid_tx_buf[robo_iic_send_offset]);
			I2C_IntClr();
		}
		else
		{
			//DBG("+");
			I2C_IntEn(1);
			I2C_ReceiveByte();
			// DBG("!");
			iic_rx_index = 0;
			iic_rx_len = 0x7fff;
			iic_recv_handle.mem_len = 0;
			iic_recv_handle.p = 0;

			rw_flag = 0;
			I2C_IntClr();
		}
	}
	else
	{
		if(rw_flag)
		{
			if((I2C_ReceiveAcknowledge())|(!I2C_IsBusy()))
			{
				//DBG("GET nack OR stop\n");
				robo_iic_send_offset = 0;
				I2C_SlaveReleaseI2C();
				I2C_ReceiveByte();//dummy read after i2c released
			}
			else
			{
				robo_iic_send_offset++;
				I2C_SendByte(hid_tx_buf[robo_iic_send_offset]);
			}
			//I2C_SendByte(hid_tx_buf[du_iic_send_offset]); no need to send after i2c_release/master_stop
			I2C_IntClr();
		}
		else
		{
			//DBG("+");
			abyte = I2C_ReceiveByte();
			//mv_mwrite(&abyte, 1, 1, &iic_recv_handle);
			robo_iic_recv_temp_buf[iic_rx_index] = abyte;
			if(iic_rx_index++ == 1)//is lenght
			{
				//DBG("l");
				iic_rx_len = abyte;
			}

			//if requirement of parameter, return it in isr;
			if(iic_rx_index == iic_rx_len + 2)
			{
#if 1
				if(robo_iic_is_system_qurey_cmd(robo_iic_recv_temp_buf) || (robo_iic_recv_temp_buf[0] == ROBOEFFECT_START_ADDR && robo_iic_recv_temp_buf[1] == 1))//is read req
				{
					Communication_Effect_Config(robo_iic_recv_temp_buf[0], &robo_iic_recv_temp_buf[2], iic_rx_len);
					//iic_recv_handle.mem_len = 0;
					//iic_recv_handle.p = 0;
				}
				else
#endif
				{
					abyte = 0xA5;
					mv_mwrite(&abyte, 1, 1, &iic_recv_handle);
					abyte = 0x5A;
					mv_mwrite(&abyte, 1, 1, &iic_recv_handle);
					mv_mwrite(robo_iic_recv_temp_buf, 1, iic_rx_len + 2, &iic_recv_handle);
					abyte = 0x16;
					mv_mwrite(&abyte, 1, 1, &iic_recv_handle);
				}
			}
			I2C_IntClr();
		}
	}
	//I2C_IntClr();
}


void IIC_Communication_Init(void)
{
	GPIO_PortBModeSet(GPIOB2, 1); //A13 UART TX
	GPIO_PortBModeSet(GPIOB3, 2); //A14 UART RX
//	GPIO_RegOneBitSet(GPIO_B_IE, GPIOB2);
//	GPIO_RegOneBitClear(GPIO_B_OUT, GPIOB2);
//	GPIO_RegOneBitSet(GPIO_B_IE, GPIOB3);
//	GPIO_RegOneBitClear(GPIO_B_OUT, GPIOB3);
	robo_iic_send_offset = 0;

	I2C_Init(0x20, I2C_PORT_B2_B3, ROBO_I2C_SLAVE_ADDR);//时钟分频/GPIO设置
	// NVIC_SetPriority(I2C_InIRQn, 0);
	NVIC_EnableIRQ(I2C_InIRQn);
	I2C_IntEn(1);
	I2C_SlaveConfig();

	mv_mopen(&iic_recv_handle, robo_iic_recv_buf, ROBO_I2C_BUF_MAX_LEN-1, NULL);
}

void iic_data_entry(void)
{
	int ssize, i;
	//if((ssize = mv_msize(&iic_recv_handle)) >= iic_rx_len + 2)
	if((ssize = mv_msize(&iic_recv_handle)) > 0 )
	{
#if 1
		//DBG("S");
		mv_mread(s_rx_buf, 1, ssize, &iic_recv_handle);
		//communic_buf[0] = 0xA5;
		//communic_buf[1] = 0x5A;
		//communic_buf[iic_rx_len + 4] = 0x16;
		//Communication_Effect_Config(communic_buf[2], &communic_buf[4], iic_rx_len);
#else
		mv_mread(communic_buf, 1, ssize, &iic_recv_handle);
#endif
		roboeffect_prot_parse_big_block(s_rx_buf, ssize);
		
//		DBG("See iic: %d->", ssize);
//		for(i=0; i<ssize; i++)
//			DBG("%X ", communic_buf[i]);
//		DBG("\n");
	}

//	if(I2C_IsArbitrationLost())
//	{
//		DBG("arb\n");
//		I2C_Disable();
//		I2C_Enable();
//	}
}

void uart_data_init(void)
{
	GPIO_PortBModeSet(GPIOB2, 0x03); //B2 UART TX
	GPIO_PortBModeSet(GPIOB3, 0x01); //B3 UART RX
	UARTS_Init(1, 115200, 8, 0, 1);
	// UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 1);
	UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);

	// DMA_CircularConfig(PERIPHERAL_ID_UART1_RX, 0, s_rx_buf, sizeof(s_rx_buf));
	// DMA_ChannelEnable(PERIPHERAL_ID_UART1_RX);

	mv_mopen(&iic_recv_handle, robo_iic_recv_buf, ROBO_I2C_BUF_MAX_LEN-1, NULL);

	UART1_IOCtl(UART_IOCTL_RXINT_SET, 1);//open rx int
	UART1_IOCtl(UART_IOCTL_DMA_RX_EN, 0);
	//NVIC_SetPriority(UART1_IRQn, 0);
	NVIC_EnableIRQ(UART1_IRQn);

	DMA_CircularConfig(PERIPHERAL_ID_UART1_TX, 0, s_tx_buf, sizeof(s_tx_buf));
	DMA_ChannelEnable(PERIPHERAL_ID_UART1_TX);
	UART1_IOCtl(UART_IOCTL_DMA_TX_EN, 1);
}


void uart_data_entry(void)
{
	uint32_t len;

	len = mv_msize(&iic_recv_handle);
	if(len > 0)
	{
		// DBG("read: %d", len);
		mv_mread(s_rx_buf, 1, len, &iic_recv_handle);
		roboeffect_prot_parse_big_block(s_rx_buf, len);
	}
}


__attribute__((section(".driver.isr")))
void UART1_Interrupt(void)
{
	if(UARTS_IOCTL(UART_PORT1, UART_IOCTL_RXSTAT_GET, 1) & 0x01)
	{
		uint8_t rxdata, dmrxdata;
		UARTS_IOCTL(1, UART_IOCTL_RXINT_CLR, 1);
		UART1_RecvByte(&rxdata);
		if(mv_mremain(&iic_recv_handle) == 0)
			mv_mread(&dmrxdata, 1, 1, &iic_recv_handle);
		mv_mwrite(&rxdata, 1, 1, &iic_recv_handle);
		// DBG(".");
	}
}

/***************************************I2C****************************************************/

#endif
