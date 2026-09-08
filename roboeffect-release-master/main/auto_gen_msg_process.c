
/***************************************************
 * @file    auto_gen_msg_process.c                      
 * @brief   auto generated                          
 * @author  auto generated
 * @version V1.1.0                                 
 * @copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 ***************************************************/

#include "stdio.h"
#include "type.h"
#include "roboeffect_config.h"
#include "roboeffect_api.h"

#include "auto_gen_msg_process.h"
#include "user_config.h"
#include "adc_key.h"

#ifdef CFG_FLOWCHART_DEMO_ENABLE
#include "user_effect_flow_demo.h"
#endif

#ifdef CFG_FUNC_AUDIOEFFECT_AUTO_GEN_MSG_PROC

extern void *context_memory;

static inline int16_t NDS32_CLIPS_ADD(int16_t a, int16_t b)
{
    return __nds32__clips((int32_t)a + b, (16)-1);
}

static inline int16_t NDS32_CLIPS_SUB(int16_t a, int16_t b)
{
    return __nds32__clips((int32_t)a - b, (16)-1);
}

int16_t eq1_enable = 0;
int16_t eq1_gain = 0;
int16_t eq1_gain_l = 0;
int16_t gain0 = 0;


#ifdef CFG_FLOWCHART_DEMO_ENABLE
static uint8_t demo_gain_control0_gain0_up_loop_adc_key_1_gain(void)
{
	//param set type is GEAR
	static const int16_t gain0_table[5] = {-7200, -6000, -2400, -1200, -600, };
	if(++gain0 >= 5) gain0 = 0;
	roboeffect_set_effect_parameter(context_memory, DEMO_gain_control0_ADDR, 1, &gain0_table[gain0]);
	return (uint8_t)DEMO_gain_control0_ADDR;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#ifdef CFG_FLOWCHART_DEMO_ENABLE
static uint8_t demo_eq1_eq1_gain_up_stop_adc_key_2_pregain(void)
{
	//param set type is GEAR
	static const int16_t eq1_gain_table[5] = {-2662, -1331, 0, 1536, 2304, };
	if(++eq1_gain >= 5) eq1_gain = 5-1;
	roboeffect_set_effect_parameter(context_memory, DEMO_eq1_ADDR, 0, &eq1_gain_table[eq1_gain]);
	return (uint8_t)DEMO_eq1_ADDR;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#ifdef CFG_FLOWCHART_DEMO_ENABLE
static uint8_t demo_eq1_eq1_gain_down_stop_adc_key_3_pregain(void)
{
	//param set type is GEAR
	static const int16_t eq1_gain_table[5] = {-2662, -1331, 0, 1536, 2304, };
	if(--eq1_gain < 0) eq1_gain = 0;
	roboeffect_set_effect_parameter(context_memory, DEMO_eq1_ADDR, 0, &eq1_gain_table[eq1_gain]);
	return (uint8_t)DEMO_eq1_ADDR;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#ifdef CFG_FLOWCHART_DEMO_ENABLE
static uint8_t demo_eq1_eq1_enable_toggle_adc_key_4_enable(void)
{
	//param set type is ONOFF toggle
	eq1_enable = (eq1_enable == 0) ? 1 : 0;
	roboeffect_enable_effect(context_memory, DEMO_eq1_ADDR, eq1_enable);
	return (uint8_t)DEMO_eq1_ADDR;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#ifdef CFG_FLOWCHART_DEMO_ENABLE
static uint8_t demo_eq1_eq1_gain_l_down_loop_adc_key_5_pregain(void)
{
	//param set type is STEPS
	static const int16_t eq1_gain_l_table[10] = {-24576, -21333, -18091, -14848, -11605, -8363, -5120, -1877, 1365, 4608, };
	if(--eq1_gain_l < 0) eq1_gain_l = 10-1;
	roboeffect_set_effect_parameter(context_memory, DEMO_eq1_ADDR, 0, &eq1_gain_l_table[eq1_gain_l]);
	return (uint8_t)DEMO_eq1_ADDR;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#ifdef CFG_FLOWCHART_DEMO_ENABLE
uint8_t msg_process_demo(int msg)
{
	uint8_t flush_address = 0;
	switch (msg) {
		case ADC_KEY_1:
			flush_address = demo_gain_control0_gain0_up_loop_adc_key_1_gain();
			break;
		case ADC_KEY_2:
			flush_address = demo_eq1_eq1_gain_up_stop_adc_key_2_pregain();
			break;
		case ADC_KEY_3:
			flush_address = demo_eq1_eq1_gain_down_stop_adc_key_3_pregain();
			break;
		case ADC_KEY_4:
			flush_address = demo_eq1_eq1_enable_toggle_adc_key_4_enable();
			break;
		case ADC_KEY_5:
			flush_address = demo_eq1_eq1_gain_l_down_loop_adc_key_5_pregain();
			break;
		default:
			break;
	}
	return flush_address;
}
#endif /*CFG_FLOWCHART_DEMO_ENABLE*/

#endif /*CFG_FUNC_AUDIOEFFECT_AUTO_GEN_MSG_PROC*/