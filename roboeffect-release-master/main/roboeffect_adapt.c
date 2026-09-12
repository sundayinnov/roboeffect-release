/*###############################################################################
# @file    roboeffect_adapt.c
# @author  castle (Automatic generated)
# @date    N/A
# @brief   
# @attention
#
# THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
# WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
# TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
# INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
# FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
# CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
#
# <h2><center>&copy; COPYRIGHT 2023 MVSilicon </center></h2>
#/
###############################################################################
*/


#include "stdio.h"
#include "type.h"
#include "roboeffect_api.h"
#include "roboeffect_adapt.h"



#include "user_effect_flow_demo.h"
const roboeffect_adapt_device_table demo_adapt_device_table = 
{
	5,
	{
		{
			DEMO_SOURCE_AP82_MIC,//io_id
			BITS_16,//width
			CH_STEREO,//channel
			"SOURCE_AP82_MIC",//name
		},
		{
			DEMO_SOURCE_I2S1_RX_BUF,//io_id
			BITS_16,//width
			CH_MONO,//channel
			"SOURCE_I2S1_RX_BUF",//name
		},
		{
			DEMO_SINK_USB_OUT,//io_id
			BITS_16,//width
			CH_STEREO,//channel
			"SINK_USB_OUT",//name
		},
		{
			DEMO_SINK_I2S1_TX_BUF,//io_id
			BITS_16,//width
			CH_MONO,//channel
			"SINK_I2S1_TX_BUF",//name
		},
		{
			DEMO_SINK_AP82_DAC0,//io_id
			BITS_16,//width
			CH_STEREO,//channel
			"SINK_AP82_DAC0",//name
		},
	}

};

