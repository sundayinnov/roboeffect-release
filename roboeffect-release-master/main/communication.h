/**
 **************************************************************************************
 * @file    communication.h
 * @brief   
 *
 * @author  Castle Cai
 * @version V1.0.0
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__

#include <stdint.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "type.h"
#include "roboeffect_api.h"

#define STREAM_CLIPS_LEN 240
extern uint32_t get_user_effects_script_len(void);
extern uint32_t get_user_effects_script_len_1(void);
extern const uint8_t effect_property_for_display[];

//v3 package 
#define V3_PACKAGE_TYPE_LIB           			0
#define V3_PACKAGE_TYPE_PARAM           		1
#define V3_PACKAGE_TYPE_STREAM          		2
#define V3_PACKAGE_TYPE_PARAMBIN_DATA    			3//parambin such as parambin header, flow block data, sub-type data, etc.
#define V3_PACKAGE_TYPE_FLOW_NAME_QUERY     7
#define V3_PACKAGE_TYPE_FLOW_INDEX_QUERY    8
#define V3_PACKAGE_TYPE_MODE_INDEX_QUERY    6
#define V3_PACKAGE_TYPE_MODE_ALL_NAME    		5
#define V3_PACKAGE_TYPE_MODE_LIB_CRC    		(0x0E) //effect_property_for_display[]'s crc


//old params
#define AGC_ON TRUE //TRUE FALSE
#define PGA_GAIN 44
#define PGA_GAIN_BOOST 4
#define PGA_GAIN_MUSIC 14
#define PGA_GAIN_MUSIC_BOOST 4

/* targetlevel = linspace(-3,-34,32) --> 0:-3, 1:-4, 2:-5, 3:-6, 4:-7, 5:-8, 6:-9, 7:-10, 
	8:-11, 9:-12, 10:-13, 11:-14, 12:-15, 13:-16, 14:-17, 15:-18, 
	16:-19, 17:-20, 18:-21, 19:-22, 20:-23, 21:-24, 22:-25, 23:-26, 
	24:-27, 25:-28, 26:-29, 27:-30, 28:-31, 29:-32, 30:-33, 31:-34
*/
#define AGC_MAXLEVEL 		12

/* targetlevel = linspace(-3,-34,32) -->  0:-3, 1:-4, 2:-5, 3:-6, 4:-7, 5:-8, 6:-9, 7:-10, 
	8:-11, 9:-12, 10:-13, 11:-14, 12:-15, 13:-16, 14:-17, 15:-18, 
	16:-19, 17:-20, 18:-21, 19:-22, 20:-23, 21:-24, 22:-25, 23:-26, 
	24:-27, 25:-28, 26:-29, 27:-30, 28:-31, 29:-32, 30:-33, 31:-34
*/
#define AGC_TARGETLEVEL 	15


/* gain = linspace(39.64,-20.3,64)
0:39.64  1:38.68  2:37.73  3:36.78  4:35.83  5:34.88  6:33.93  7:32.98  8:32.02
9:31.07  10:30.12  11:29.17  12:28.22  13:27.27  14:26.32  15:25.36  16:24.41  17:23.46  18:22.51
19:21.56  20:20.61  21:19.66  22:18.70  23:17.75  24:16.80  25:15.85  26:14.90  27:13.95  28:13
29:12.04  30:11.09  31:10.14  32:9.19  33:8.24  34:7.29  35:6.34  36:5.38  37:4.43  38:3.48  39:2.53
40:1.58  41:0.63  42:-0.32  43:-1.27  44:-2.22  45:-3.17  46:-4.12  47:-5.07  48:-6.02  49:-6.98
50:-7.93  51:-8.88  52:-9.83  53:-10.78  54:-11.73  55:-12.68  56:-13.64  57:-14.59  58:-15.54
59:-16.49  60:-17.44  61:-18.39  62:-19.34  63:-20.30
*/
#define AGC_MAX_GAIN 		4//16
#define AGC_MIN_GAIN 		63
#define AGC_GAIN_OFFSET 	0 //0dB

#define AGC_FRAME_TIME 		96 //ms
#define AGC_HOLD_TIME 		2 //ms
#define AGC_ATTACK_STEPTIME 32 //ms
#define AGC_DECAY_STEPTIME 	30 //ms
#define AGC_NS_GATE_ENABLE 	FALSE
/* threshold = linspace(-90,-28,32) --> 0:-90, 1:-88, 2:-86, 3:-84, 4:-82, 5:-80, 6:-78, 7:-76, 
	8:-74, 9:-72, 10:-70, 11:-68, 12:-66, 13:-64, 14:-62, 15:-60, 
	16:-58, 17:-56, 18:-54, 19:-52, 20:-50, 21:-48, 22:-46, 23:-44, 24:-42, 
	25:-40, 26:-38, 27:-36, 28:-34, 29:-32, 30:-30, 31:-28 
*/
#define AGC_NS_THRESHOLD 	28 //-34dB
#define AGC_NS_MODE 		1
#define AGC_NS_HOLD_TIME 	10

#define RIGHT_CHANNEL INPUT_MIC3 //INPUT_LINEIN2_RIGHT INPUT_MIC3
#define LEFT_CHANNEL INPUT_MIC4 //INPUT_LINEIN2_LEFT INPUT_MIC4

#define RIGHT_CHANNEL_MUSIC INPUT_LINEIN2_RIGHT //INPUT_LINEIN2_RIGHT INPUT_MIC3
#define LEFT_CHANNEL_MUSIC INPUT_LINEIN2_LEFT //INPUT_LINEIN2_LEFT INPUT_MIC4

#define PGA0_INPUT_LINEIN1_LEFT		(0x0001)
#define PGA0_INPUT_LINEIN2_LEFT		(0x0002)
#define PGA0_INPUT_MIC4				(0x0004)
#define PGA0_INPUT_LINEIN1_RIGHT	(0x0001)
#define PGA0_INPUT_LINEIN2_RIGHT	(0x0002)
#define PGA0_INPUT_MIC3				(0x0004)


#define PGA1_INPUT_LINEIN3_LEFT		(0x0001)
#define PGA1_INPUT_LINEIN2_LEFT		(0x0002)
#define PGA1_INPUT_FMIN_LEFT		(0x0004)
#define PGA1_INPUT_MIC2				(0x0008)
#define PGA1_INPUT_LINEIN3_RIGHT	(0x0001)
#define PGA1_INPUT_LINEIN2_RIGHT	(0x0002)
#define PGA1_INPUT_FMIN_RIGHT		(0x0004)
#define PGA1_INPUT_MIC1				(0x0008)

typedef struct _ControlVariablesContext
{
    //Syncword
    // uint32_t 			SyncWord;
    // uint32_t 			Version;
	// uint32_t 			SamplesPerFrame;

	// //for system control 0x01
	// uint32_t            sys_mode;
	// uint32_t            sys_reset;
	// uint32_t            sys_sample_rate_en;
	// uint32_t 		    sample_rate;
	// uint32_t            sys_mclk_src_en;
	// uint32_t            mclk_source;

	// //for System status 0x02
    // uint32_t 			cpu_mips;
	// uint32_t 			UsedRamSize;

	//for ADC0 PGA      0x03
    volatile uint32_t   pga0_l_channel;//bit flag
    volatile uint32_t   pga0_r_channel;//bit flag
    uint32_t            pga0_l_gain;
    uint32_t            pga0_r_gain;
    uint8_t             pga0_r_boost;
    uint8_t             pga0_l_boost;
	uint32_t            pga0_zero_cross;
	uint32_t            pga0_l_mute;
	uint32_t            pga0_r_mute;
	uint8_t             pga0_mode;//{b0=r,b1=l} 0 for single ended input, 1 for differential input
	uint8_t             pga0_diff_gain_l;
	uint8_t             pga0_diff_gain_r;

    uint32_t            line1_l_gain;
    uint32_t            line1_r_gain;
    uint32_t            line2_0_l_gain;///PGA0 for line2
    uint32_t            line2_0_r_gain;///PGA0 for line2
    uint8_t             mic4_l_gain;
    uint8_t             mic3_r_gain;
    uint8_t             mic4_l_boost;
    uint8_t             mic3_r_boost;


	//for ADC0 DIGITAL  0x04
	uint32_t 		    adc0_sample_rate;
    uint32_t         	adc0_dig_channel_en;
	uint32_t         	adc0_dig_channel_delay_en;
	uint32_t  			adc0_dig_mute;
	uint32_t  		    adc0_dig_l_mute;
	uint32_t  		    adc0_dig_r_mute;
    uint32_t  			adc0_dig_l_vol;
    uint32_t  			adc0_dig_r_vol;
	bool  			    adc0_lr_swap;
	uint16_t            adc0_dc_blocker;
	uint8_t             adc0_fade_time;
	uint32_t            adc0_mclk_src;
	uint32_t            adc0_dc_blocker_en;

	//for AGC0 ADC0     0x05
	uint32_t            agc0_mode;
	uint32_t            agc0_max_level;
	uint32_t            agc0_target_level;
	uint32_t            agc0_max_gain;
	uint32_t            agc0_min_gain;
	uint32_t            agc0_gainoffset;
	uint32_t            agc0_fram_time;
	uint32_t            agc0_hold_frames;//agc0_hold_time;
	uint32_t            agc0_attack_time;
	uint32_t            agc0_decay_time;
	uint32_t            agc0_noise_gate_en;
	uint32_t            agc0_noise_threshold;
	uint32_t            agc0_noise_gate_mode;
	uint32_t            agc0_noise_hold_frames;//agc0_noise_time;

	//for ADC1 PGA      0x06
	volatile uint32_t   pga1_l_channel;//bit flag
    volatile uint32_t  	pga1_r_channel;//bit flag
    uint32_t            pga1_l_gain;
    uint32_t            pga1_r_gain;
    uint8_t             pga1_r_boost;
    uint8_t             pga1_l_boost;
	uint32_t            pga1_zero_cross;
	uint32_t            pga1_l_mute;
	uint32_t            pga1_r_mute;

    uint32_t            line3_l_gain;
    uint32_t            line3_r_gain;
    uint32_t            line2_1_l_gain;///PGA1 for line2
    uint32_t            line2_1_r_gain;///PGA1 for line2
    uint32_t            fm_l_gain;
    uint32_t            fm_r_gain;
    uint8_t             mic2_l_gain;
    uint8_t             mic1_r_gain;
    uint8_t             mic2_l_boost;
    uint8_t             mic1_r_boost;
	uint8_t             pga1_mode;//{b0=r,b1=l} 0 for single ended input, 1 for differential input

	//for ADC1 DIGITAL  0x07
	uint32_t 		    adc1_sample_rate;
    uint32_t         	adc1_dig_channel_en;
    uint32_t         	adc1_dig_channel_delay_en;
	uint32_t  			adc1_dig_mute;
	uint32_t  			adc1_dig_l_mute;
	uint32_t  	        adc1_dig_r_mute;
    uint32_t  			adc1_dig_l_vol;
    uint32_t  			adc1_dig_r_vol;
	bool  			    adc1_lr_swap;
	uint16_t            adc1_dc_blocker;
	uint8_t             adc1_fade_time;
	uint32_t            adc1_mclk_src;
	uint32_t            adc1_dc_blocker_en;

	//for AGC1  ADC1    0x08
	uint32_t            agc1_mode;
	uint32_t            agc1_max_level;
	uint32_t            agc1_target_level;
	uint32_t            agc1_max_gain;
	uint32_t            agc1_min_gain;
	uint32_t            agc1_gainoffset;
	uint32_t            agc1_fram_time;
	uint32_t            agc1_hold_frames;//agc1_hold_time;
	uint32_t            agc1_attack_time;
	uint32_t            agc1_decay_time;
	uint32_t            agc1_noise_gate_en;
	uint32_t            agc1_noise_threshold;
	uint32_t            agc1_noise_gate_mode;
	uint32_t            agc1_noise_hold_frames;//agc1_noise_time;

	//for DAC0          0x09
	uint32_t            dac0_en;
	uint32_t            dac0_delay_en;
	uint32_t            dac0_sample_rate;
	uint32_t            dac0_dig_mute;
	uint32_t            dac0_dig_l_mute;
	uint32_t            dac0_dig_r_mute;
	uint32_t            dac0_dig_l_vol;
	uint32_t            dac0_dig_r_vol;
	uint32_t            dac0_dither;
	uint32_t            dac0_scramble;
	uint8_t             dac0_out_mode;
	uint8_t             dac0_pause_en;
	uint8_t             dac0_sample_mode;
	uint32_t            dac0_scf_mute;
	uint8_t             dac0_fade_time;
	uint8_t             dac0_zeros_number;
	uint32_t            dac0_mclk_src;

	//for DAC1          0x0a
	uint32_t            dac1_en;
	uint32_t            dac1_delay_en;
	uint32_t            dac1_sample_rate;
	uint32_t            dac1_dig_mute;
	uint32_t            dac1_dig_vol;
	uint32_t            dac1_dither;
	uint32_t            dac1_scramble;
	uint8_t             dac1_out_mode;
	uint8_t             dac1_pause_en;
	uint8_t             dac1_sample_mode;
	uint32_t            dac1_scf_mute;
	uint8_t             dac1_fade_time;
	uint8_t             dac1_zeros_number;
	uint32_t            dac1_mclk_src;

	//for i2s0          0x0b
    uint32_t  			i2s0_dig_vol;
	uint32_t            i2s0_tx_en;
	uint32_t            i2s0_tx_delay_en;
	uint32_t            i2s0_rx_en;
	uint32_t            i2s0_rx_delay_en;
	uint32_t            i2s0_sample_rate;
	uint32_t            i2s0_mclk_src;
	uint32_t            i2s0_work_mode;
	uint32_t            i2s0_word_len;
	bool                i2s0_mono;
	uint32_t            i2s0_fade_time;
	uint32_t            i2s0_format;
	uint32_t            i2s0_bclk_invert_en;
	uint32_t            i2s0_lrclk_invert_en;

	//for i2s1          0x0c
    uint32_t  			i2s1_dig_vol;
	uint32_t            i2s1_tx_en;
	uint32_t            i2s1_tx_delay_en;
	uint32_t            i2s1_rx_en;
	uint32_t            i2s1_rx_delay_en;
	uint32_t            i2s1_sample_rate;
	uint32_t            i2s1_mclk_src;
	uint32_t            i2s1_work_mode;
	uint32_t            i2s1_word_len;
	bool                i2s1_mono;
	uint32_t            i2s1_fade_time;
	uint32_t            i2s1_format;
	uint32_t            i2s1_bclk_invert_en;
	uint32_t            i2s1_lrclk_invert_en;

	//for spdif
    //uint32_t 		    spdif_dig_vol;
	uint32_t			spdif_en;//0 for disable, 1 for rx enable, 2 for tx enable
	uint32_t			spdif_delay_en;//0 for disable, 1 for rx enable, 2 for tx enable
	uint32_t			spdif_sample_rate;//index
	uint32_t			spdif_ch_mode;
	uint32_t			spdif_io_sel;
	uint32_t			spdif_lock_status;//0 for unlock, 1 for lock

}ControlVariablesContext;

//codec display on/off
#define DISP_LINEIN1_LEFT 	(0)
#define DISP_LINEIN1_RIGHT 	(0)
#define DISP_MIC4 					(1)
#define DISP_MIC3 					(1)
#define DISP_LINEIN2_0_LEFT 	(0)
#define DISP_LINEIN2_0_RIGHT 	(0)
#define DISP_LINEIN3_LEFT 	(1)
#define DISP_LINEIN3_RIGHT 	(1)
#define DISP_MIC2 					(0)
#define DISP_MIC1 					(0)
#define DISP_LINEIN2_1_LEFT 	(1)
#define DISP_LINEIN2_1_RIGHT 	(1)
#define DISP_LINEIN4_LEFT 	(0)
#define DISP_LINEIN4_RIGHT 	(0)


typedef enum _du_func_capability
{
	DU_CAP_MULTI_MODE = 0x01,
	DU_CAP_MULTI_GRAPHIC = 0x02,
	DU_CAP_HAS_PARAMBIN = 0x04,
	DU_CAP_DOWNLOAD_GRAPHIC = 0x08,
	DU_CAP_DOWNLOAD_PARAM = 0x10,

} du_func_capability;
extern ControlVariablesContext gCtrlVars;

void load_hardware_fr_default(void);
void load_hardware_fr_params_raw(uint8_t *parameters);
void load_params_to_codec(void);

void run_upgrade_process(void);

void roboeffect_effect_enquiry_stream(uint8_t *buf, uint32_t tlen);

#endif/*__COMMUNICATION_H__*/