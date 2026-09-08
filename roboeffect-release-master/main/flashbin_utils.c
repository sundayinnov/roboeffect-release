/**
 **************************************************************************************
 * @file    flashbin_utils.c
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

#include "audio_effect_library.h"
#include "roboeffect_api.h"
#include "communication.h"
#include "user_config.h"

#include "roboeffect_prot.h"
#include "user_effect_flow_demo.h"
#include "roboeffect_adapt.h"
#include "user_defined_effect_api.h"

#define PARAM_SUB_TYPE(index) ((0x01 << 8) | (index))
#define PRESET_SUB_TYPE(index) ((0x02 << 8) | (index))

extern uint8_t  tx_buf[];

extern roboeffect_effect_list_info *g_user_effect_list;
extern const roboeffect_effect_steps_table *g_user_effect_steps;
extern const unsigned char *g_user_effect_parameters;

int32_t comm_ret_sample_rate_enum(uint32_t samplerate);
void Communication_Effect_Send(uint8_t *buf, uint32_t len);

extern uint16_t sizeof_effect_property_for_display(void);
extern uint32_t get_user_effects_script_len_demo(void);


extern uint16_t g_flow_index;
extern uint16_t g_param_mode_index;

extern char *graph_name;

typedef struct _comm_status_table
{
	uint8_t old_cmd;
	int32_t total_len;
	int32_t remain_len;
	const uint8_t *data;
	uint8_t package_id;
}comm_status_table;

static comm_status_table g_status_machine = {0xff, 0, 0, 0, 0};

static void init_status_matchin(comm_status_table *machine)
{
	memset(machine, 0x00, sizeof(comm_status_table));
	machine->old_cmd = 0xff;
}

void roboeffect_effect_enquiry_stream(uint8_t *buf, uint32_t tlen)
{
	uint8_t *temp;

  temp = tx_buf;

	if(buf[0] == V3_PACKAGE_TYPE_PARAM)//query params
	{

		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = ROBOEFFECT_START_ADDR;
		temp[3] = 10 + 1 + strlen(graph_name);//len including EOM
		temp[4] = 0x00;//package id
		temp[5] = 0xFF;
		temp[6] = 0x00;
		temp[7] = buf[0];//type

		temp[8] = 0x01;//graph count

		*(uint16_t *)&temp[9] = g_user_effect_list->frame_size;
		*(uint16_t *)&temp[11] = comm_ret_sample_rate_enum(g_user_effect_list->sample_rate);
		*(uint8_t *)&temp[13] = strlen(graph_name);
		strcpy(&temp[14], graph_name);

		
		temp[13 + 1 + strlen(graph_name)] = 0x01;//EOM
		temp[14 + 1 + strlen(graph_name)] = 0x16;
		Communication_Effect_Send(temp, temp[3] + 5);
	}
	else if(buf[0] == V3_PACKAGE_TYPE_FLOW_INDEX_QUERY || buf[0] == V3_PACKAGE_TYPE_MODE_INDEX_QUERY)//query/set flow chart index or parameter mode index
	{
		if(tlen == 1)//query
		{
			temp[0] = 0xA5;
			temp[1] = 0x5A;
			temp[2] = ROBOEFFECT_START_ADDR;
			temp[3] = 7;
			temp[4] = 0x00;//package id
			temp[5] = 0xFF;
			temp[6] = 0x00;
			temp[7] = buf[0];//type

			temp[8] = 0x00;
			if(buf[0] == V3_PACKAGE_TYPE_FLOW_INDEX_QUERY)
				temp[9] = g_flow_index;
			else if(buf[0] == V3_PACKAGE_TYPE_MODE_INDEX_QUERY)
				temp[9] = g_param_mode_index;

			temp[10] = 0x01;//EOM
			temp[11] = 0x16;

			Communication_Effect_Send(temp, temp[3] + 5);
		}
		else if(tlen == 2)//set
		{
			if(buf[0] == V3_PACKAGE_TYPE_FLOW_INDEX_QUERY)
				g_flow_index = buf[1];
			else if(buf[0] == V3_PACKAGE_TYPE_MODE_INDEX_QUERY)
				g_param_mode_index = buf[1];
			
			//todo: setup switch flag to run a new flow-chart or a new parameter mode

		}
	}
	else if(buf[0] == V3_PACKAGE_TYPE_FLOW_NAME_QUERY)
	{

	}
	else if(buf[0] == V3_PACKAGE_TYPE_LIB || buf[0] == V3_PACKAGE_TYPE_STREAM || buf[0] == V3_PACKAGE_TYPE_PARAMBIN_DATA || buf[0] == V3_PACKAGE_TYPE_MODE_ALL_NAME)//enquiry stream
	{
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = ROBOEFFECT_START_ADDR;
		//len later

		if(g_status_machine.old_cmd != buf[0])//if not get EOM, a new cmd is alway the first package
		{
			uint16_t len;

			init_status_matchin(&g_status_machine);
			g_status_machine.old_cmd = buf[0];

			//package type dispatch
			if(buf[0] == V3_PACKAGE_TYPE_LIB)
			{
				g_status_machine.data = effect_property_for_display;
				g_status_machine.total_len = sizeof_effect_property_for_display();
				// printf("first audio effect info\n");
			}
			else if(buf[0] == V3_PACKAGE_TYPE_STREAM)
			{
				g_status_machine.data = user_effects_script_demo;
				g_status_machine.total_len = (uint16_t)get_user_effects_script_len_demo();
				// printf("first script\n");
			}
			else if(buf[0] == V3_PACKAGE_TYPE_PARAMBIN_DATA)//flash bin process
			{
				return;
			}
			else if(buf[0] == V3_PACKAGE_TYPE_MODE_ALL_NAME)//all mode name under current flow-chart
			{
				return;
			}
			else
			{
				//no such command existed.
				return;
			}

			g_status_machine.remain_len = g_status_machine.total_len;//init remain_len
			if(g_status_machine.remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = g_status_machine.remain_len;
			
			temp[3] = len + 7;

			//[package_id, 0xFF, 0x00, type, len_H, len_L, data...data, EOM]
			temp[4] = g_status_machine.package_id;
			temp[5] = 0xFF;
			temp[6] = 0x00;
			temp[7] = buf[0];//message type

			*((uint16_t*)(&temp[8])) = g_status_machine.total_len;

			memcpy(&temp[10], g_status_machine.data, len);

			g_status_machine.remain_len -= len;
			g_status_machine.data += len;

			if(g_status_machine.remain_len > 0)
			{
				temp[10 + len] = 0x00;//not EOM
				g_status_machine.package_id ++;
			}
			else
			{
				temp[10 + len] = 0x01;//EOM
				init_status_matchin(&g_status_machine);//reset for next field
			}
			
			temp[10 + len + 1] = 0x16;

			Communication_Effect_Send(temp, len + 12);
		}
		else
		{
			uint16_t len;
			if(g_status_machine.remain_len >= STREAM_CLIPS_LEN)
				len = STREAM_CLIPS_LEN;
			else
				len = g_status_machine.remain_len;

			temp[3] = len + 2;
			temp[4] = g_status_machine.package_id;
			memcpy(&temp[5], g_status_machine.data, len);

			g_status_machine.remain_len -= len;
			g_status_machine.data += len;

			if(g_status_machine.remain_len > 0)
			{
				temp[5 + len] = 0x00;//not EOM
				g_status_machine.package_id ++;
			}
			else
			{
				temp[5 + len] = 0x01;//EOM
				init_status_matchin(&g_status_machine);//reset for next field

				//switch to next graph
			}
			
			temp[5 + len + 1] = 0x16;

			Communication_Effect_Send(temp, len + 7);
		}
	}
	else if(buf[0] == V3_PACKAGE_TYPE_MODE_LIB_CRC)
	{
		uint32_t crc32_temp = EFFECT_PROPERTY_BIN_CRC32;
		temp[0] = 0xA5;
		temp[1] = 0x5A;
		temp[2] = 0x80;
		temp[3] = 0x08;//const
		temp[4] = 0x00;//package id
		temp[5] = 0xFF;
		temp[6] = 0x00;//
		temp[7] = 0x0E;

		memcpy(&(temp[8]), &crc32_temp, 4);
		tx_buf[4 + tx_buf[3]] = 0x16;

		Communication_Effect_Send(temp, temp[3] + 5);

	}
}