/**
 *******************************************************************************
 * @file    powercontroller.h
 * @brief	powercontroller module driver interface

 * @author  Sean
 * @version V1.0.0

 * $Created: 2017-11-13 16:51:05$
 * @Copyright (C) 2017, Shanghai Mountain View Silicon Co.,Ltd. All rights reserved.
 *******************************************************************************
 */

/**
 * @addtogroup POWERCONTROLLER
 * @{
 * @defgroup powercontroller powercontroller.h
 * @{
 */
 
#ifndef __POWERCONTROLLER_H__
#define __POWERCONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

#define     WAKEUP_SOURCE_NONE					(0)		 /**<None*/
#define     WAKEUP_SOURCE0_GPIO					(1 << 0) /**<GPIO wakeup source0 macro*/
#define     WAKEUP_SOURCE1_GPIO					(1 << 1) /**<GPIO wakeup source1 macro*/
#define     WAKEUP_SOURCE2_GPIO					(1 << 2) /**<GPIO wakeup source2 macro*/
#define     WAKEUP_SOURCE3_GPIO					(1 << 3) /**<GPIO wakeup source3 macro*/
#define     WAKEUP_SOURCE4_GPIO					(1 << 4) /**<GPIO wakeup source4 macro*/
#define     WAKEUP_SOURCE5_GPIO					(1 << 5) /**<GPIO wakeup source5 macro*/
#define     WAKEUP_SOURCE6_RTC					(1 << 6) /**<GPIO wakeup source5 macro*/


#define     WAKEUP_GPIOA0          				(0)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOA1          				(1)	/**<GPIO bit1 macro*/
#define     WAKEUP_GPIOA2          				(2)	/**<GPIO bit2 macro*/
#define     WAKEUP_GPIOA3          				(3)	/**<GPIO bit3 macro*/
#define     WAKEUP_GPIOA4          				(4)	/**<GPIO bit4 macro*/
#define     WAKEUP_GPIOA5         	 			(5)	/**<GPIO bit5 macro*/
#define     WAKEUP_GPIOA6          				(6)	/**<GPIO bit6 macro*/
#define     WAKEUP_GPIOA7          				(7)	/**<GPIO bit7 macro*/
#define     WAKEUP_GPIOA8          				(8)	/**<GPIO bit8 macro*/
#define     WAKEUP_GPIOA9          				(9)	/**<GPIO bit9 macro*/
#define     WAKEUP_GPIOA10         				(10)	/**<GPIO bit10 macro*/
#define     WAKEUP_GPIOA11         				(11)	/**<GPIO bit11 macro*/
#define     WAKEUP_GPIOA12         				(12)	/**<GPIO bit12 macro*/
#define     WAKEUP_GPIOA13         				(13)	/**<GPIO bit13 macro*/
#define     WAKEUP_GPIOA14         				(14)	/**<GPIO bit14 macro*/
#define     WAKEUP_GPIOA15         				(15)	/**<GPIO bit15 macro*/
#define     WAKEUP_GPIOA16         				(16)	/**<GPIO bit16 macro*/
#define     WAKEUP_GPIOA17         				(17)	/**<GPIO bit17 macro*/
#define     WAKEUP_GPIOA18         				(18)	/**<GPIO bit18 macro*/
#define     WAKEUP_GPIOA19         				(19)	/**<GPIO bit19 macro*/
#define     WAKEUP_GPIOA20         				(20)	/**<GPIO bit20 macro*/
#define     WAKEUP_GPIOA21         				(21)	/**<GPIO bit21 macro*/
#define     WAKEUP_GPIOA22         				(22)	/**<GPIO bit22 macro*/
#define     WAKEUP_GPIOA23         				(23)	/**<GPIO bit23 macro*/
#define     WAKEUP_GPIOA24         				(24)	/**<GPIO bit24 macro*/
#define     WAKEUP_GPIOA25         				(25)	/**<GPIO bit25 macro*/
#define     WAKEUP_GPIOA26         				(26)	/**<GPIO bit26 macro*/
#define     WAKEUP_GPIOA27         				(27)	/**<GPIO bit27 macro*/
#define     WAKEUP_GPIOA28         				(28)	/**<GPIO bit28 macro*/
#define     WAKEUP_GPIOA29        				(29)	/**<GPIO bit29 macro*/
#define     WAKEUP_GPIOA30        			    (30)	/**<GPIO bit30 macro*/
#define     WAKEUP_GPIOA31         				(31)	/**<GPIO bit31 macro*/

#define     WAKEUP_GPIOB0          				(32)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB1          				(33)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB2          				(34)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB3          				(35)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB4          				(36)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB5          				(37)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB6          				(38)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB7          				(39)	/**<GPIO bit0 macro*/
#define     WAKEUP_GPIOB8          				(40)	/**<GPIO bit0 macro*/

#define     WAKEUP_GPIOC0          				(41)	/**<GPIO bit0 macro*/

#define     WAKEUP_GPIOPOSE          			(1) 	/**posedge trigger*/
#define     WAKEUP_GPIONEGE          			(0) 	/**negedge trigger*/

/**
 * @brief  系统进入sleep模式
 * @param  无
 * @return 无
 */
void Power_GotoSleep(void);

/**
 * @brief  系统进入deepsleep模式
 * @param  无
 * @return 无
 */
void Power_GotoDeepsleep(void);

/**
 * @brief  配置DeepSleep唤醒源
 * @param  source 配置唤醒源：[0~6].[0~5]为GPIO，[6]为RTC
 * @param  gpio   配置gpio唤醒引脚，[0~41],分别对应GPIOA[31:0]、GPIOB[8:0]、GPIOC[0]
 * @param  edge   gpio的边沿触发方式选择：1上升沿0下降沿
 * @note 参数gpio，edge只在source为0~5才有效
 * @return 无
 */
void Power_WakeupSourceSet(uint32_t source, uint32_t gpio, uint8_t edge);

/**
 * @brief  设置某个通道唤醒使能
 * @param  source 唤醒源
 * @return 无
 */
void Power_WakeupEnable(uint8_t source);

/**
 * @brief  设置某个通道唤醒禁止
 * @param  source 唤醒源
 * @return 无
 */
void Power_WakeupDisable(uint8_t source);

/**
 * @brief  获取唤醒通道标志
 * @param  无
 * @return 获取唤醒通道标志
 */
uint32_t Power_WakeupSourceGet(void);

/**
 * @brief  清除唤醒通道标志
 * @param  无
 * @return 无
 */
void Power_WakeupSourceClear(void);

/**
 * @brief  通过唤醒源标志查询gpio唤醒引脚：[0~41],分别对应GPIOA[31:0]、GPIOB[8:0]、GPIOC[0]
 * @param  无
 * @return gpio唤醒引脚
 */
uint32_t Power_WakeupGpioGet(uint32_t source);

/**
 * @brief  通过唤醒源标志查询gpio唤醒的触发边沿
 * @param  无
 * @return 唤醒源的触发边沿      0：下降沿； 1：上升沿
 */
uint8_t Power_WakeupEdgeGet(uint32_t source);

/**
 * @brief  配置限流值
 * @param  Mode = 1，限流值280mA； Mode = 0，限流值60mA
 * @return 无
 */
void Power_CurrentLimitcConfig(uint32_t Mode);

/**
 * @brief  配置限流值
 * @param  PowerMode: 1高电压输入，大于3.6V; 0:低于3.6V
 * @param  IsChipWork:
 * @return 无
 */
void Power_PowerModeConfig(uint32_t PowerMode, bool IsChipWork);

/**
 * @brief  设置Rtc32K闹钟
 * @param  alarm:闹钟时间（单位秒）
 * @param  start:时钟计数初值：单位秒，一般为0
 * @param  stop时钟计数初值：单位秒，一般为0
 * @return 无
 */
void Power_AlarmSet(uint32_t alarm,uint32_t start,uint32_t stop);

/**
 * @brief  配置underpower的LDO电压需要调用此函数
 * @param  src:systerm core src base address0
 * @param  dest:systerm core dest base address0
 * @param  size:systerm core base address0 map size
 * @return 无
 */
void Power_FlashRemap2Sram(uint32_t src, uint32_t dest, uint32_t size);

/**
 * @brief  配置underpower的LDO电压需要调用此函数
 * @param  IsAudoCutRcClk  进入DeepSleep是否关闭RC CLK（TRUE:会关闭；FALSE不会关闭）
 * @return 无
 * @note  O26烧录的初始trim值为：1.0V（芯片个体之间会有稍许差异）
 */
void Power_DeepSleepLDO12Config(bool IsAudoCutRcClk);

/**
 * @brief  配置LDO12电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：1000mV~1300mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~9mV之间；
 */
void Power_LDO12Config(uint32_t value);

/**
 * @brief  配置LDO_33D电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：2930mV~3350mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~40mV之间；
 */
void Power_LDO33DConfig(uint32_t value);

/**
 * @brief  配置LDO_33A电压需要调用此函数
 * @param  value:单位：mV；
 *               范围：2950mV~3600mV
 * @return 无
 * @note  芯片个体之间会有稍许差异和偏差，偏差范围在0mV~20mV之间；
 * @note  33A电压调整之后对33D电压有影响
 */
void Power_LDO33AConfig(uint32_t value);

/**
 * @brief  获取flash port
 * @param  无
 * @return 1：外部flash;  0：内部flash
 * @note   无
 */
uint8_t Power_FlashGetPort(void);


#ifdef  __cplusplus
}
#endif//__cplusplus

#endif //__POWERCONTROLLER_H__

/**
 * @}
 * @}
 */
