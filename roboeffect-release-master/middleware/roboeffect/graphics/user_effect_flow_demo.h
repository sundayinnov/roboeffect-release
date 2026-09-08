/***************************************************
 * @file     user_effect_flow_demo.h  
 * @brief   auto generated  
 * @author  ACPWorkbench: 5.9.0
 * @version V1.2.0 
 * @Created 2026-09-08T09:33:53 

 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/


#ifndef __USER_EFFECT_FLOW_DEMO_H__
#define __USER_EFFECT_FLOW_DEMO_H__

#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"

#define DEMO_ROBOEFFECT_LIB_VER "2.47.1"

typedef enum _DEMO_roboeffect_io_enum
{
    DEMO_SOURCE_AEC_NS_OUT,
    DEMO_SOURCE_I2S1_RX,

    DEMO_SINK_I2S1_TX,
    DEMO_SINK_USB_OUT,
    DEMO_SINK_I2S0_TX,
} DEMO_roboeffect_io_enum;


typedef enum _DEMO_roboeffect_effect_list_enum{

    DEMO_gain_control0_ADDR = 0x81,
    DEMO_upmix_1to2_0_ADDR = 0x82,
    DEMO_gain_control1_ADDR = 0x83,
    DEMO_COUNT_ADDR,

} DEMO_roboeffect_effect_list_enum;

extern const char chart_name_demo[];

extern const unsigned char user_effects_script_demo[];

extern roboeffect_effect_list_info user_effect_list_demo;

extern const roboeffect_effect_steps_table user_effect_steps_demo;

extern uint32_t get_user_effects_script_len_demo(void);

extern char *parameter_group_name_demo[1];
extern const unsigned char user_effect_parameters_demo_NewMode[];
extern const unsigned char user_module_parameters_demo_NewMode[];
#endif/*__USER_EFFECT_FLOW_DEMO_H__*/
