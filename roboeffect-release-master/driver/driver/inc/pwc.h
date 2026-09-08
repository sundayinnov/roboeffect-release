/**
 **************************************************************************************
 * @file    pwc.h
 * @brief   Pulse Width Capture (Reuse with General Timer 3&4 ) API
 *
 * @author  Grayson Chen
 * @version V1.0.0
 *
 * $Created: 2017-09-29 13:25:30$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */

/**
 * @addtogroup PWC
 * @{
 * @defgroup pwc pwc.h
 * @{
 */

#ifndef __PWC_H__
#define __PWC_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"
#include "timer.h"

#define PWC_MAX_TIMESCALE   0xffff
    
//PWC 输出通道GPIO复用关系
    

#define   TIMER3_PWC_A23_A27_B0_B2   0    //TIMER3的PWC引脚可复用在A23, A27, B0或者B2

#define   TIMER4_PWC_A24_A28_B1_B3   1    //TIMER4的PWC引脚可复用在A24, A28, B1或者B3
    


/**
 * PWM IO Mode select definition
 */ 
typedef enum __PWC_IO_MODE
{
    PWC_IO_MODE_NONE = 0,    //还原为GPIO
    PWC_IO_MODE_IN  = 1     //直接输出   
}PWC_IO_MODE;
    


/**
 * @brief  PWC输入极性
 */
typedef enum __PWC_POLARITY
{
    PWC_POLARITY_BOTH     = 1,  /**< 双边沿捕获 */
    PWC_POLARITY_RAISING  = 2,  /**< 上升沿捕获 */
    PWC_POLARITY_FALLING  = 3   /**< 下降沿捕获 */

} PWC_POLARITY;

/**
 * @brief  PWC捕获模式
 */
typedef enum _PWC_CAPTURE_MODE
{
	PWC_CAPTURE_CONTINUES = 0,	/**< 连续捕获 */

	PWC_CAPTURE_ONCE = 1		/**< 只捕获一次 */

} PWC_CAPTURE_MODE;

/**
 * @brief  PWC输入滤波器采样时钟分频比
 */
typedef enum __PWC_FILTER_CLK_DIV
{   
    FILTER_CLK_DIV1 = 0,//不分频,等于系统时钟，即Fsys
    FILTER_CLK_DIV2,    //系统时钟的2分频，即 Fsys / 2 
    FILTER_CLK_DIV3,    //系统时钟的4分频，即 Fsys / 4
    FILTER_CLK_DIV4,    //系统时钟的8分频，即 Fsys / 8
    
}PWC_FILTER_CLK_DIV;


/**
 * @brief  PWC错误号定义
 */
typedef enum __PWC_ERROR_CODE
{
    PWC_ERROR_INVALID_PWC_INDEX = -128,
    PWC_ERROR_INVALID_PWC_POLARITY,
    PWC_ERROR_OK = 0
}PWC_ERROR_CODE;


/** 
 * @brief  PWC初始化结构体定义
 */  
typedef struct __PWC_StructInit
{
    uint8_t  SingleGet;          //捕获次数： 1 -- 只读取1次， 0 -- 连续读取
    
    uint16_t TimeScale;          //PWC测量的量程(时钟分频系数)，取值范围[1,65535], 即可以测量的范围（1/Fsys * TimeScale  ~  65535/Fsys * TimeScale）
    
    uint8_t  DMAReqEnable;       //是否使能DMA将读取的数据搬运MEM中，参数取值： 1 -- 使能DMA请求； 0 -- 禁能DMA请求
    
    uint8_t  FilterTime;         //滤波时间，范围：1/Fpwc ~ 128/Fpwc
    
    uint8_t  Polarity;           //捕获极性，参数取值范围：边缘到边缘， 上升沿到上升沿，下降沿到下降沿，详细请见 #PWC_POLARITY

           
}PWC_StructInit;  

/**
 * @brief PWC IOCTROL 命令
 */
typedef enum _PWC_IOCTRL_CMD
{
	PWC_DATA_GET = 0,                    /**< PWC数据获取 */
	PWC_DONE_STATUS_GET,                 /**< PWC捕获完成状态获取 */
	PWC_OVER_CAPTURE_STATUS_GET,         /**< PWC重复捕获状态获取 */
	PWC_ERR_STATUS_GET,                  /**< PWC错误状态获取 */

	PWC_OVER_CAPTURE_STATUS_CLR,         /**< PWC重复捕获状态清除 */
	PWC_ERR_STATUS_CLR,                  /**< PWC错误状态清除 */

    PWC_POLARITY_UPDATE,                 /**< PWC输入极性设置 */


} PWC_IOCTRL_CMD;

/**
 * @brief PWC IOCTROL 参数
 */
typedef struct _PWC_IOCTRL_ARG
{

	PWC_POLARITY    PWCPolarity;    /**< PWC输入极性 */

} PWC_IOCTRL_ARG;

/**
 * @brief      将GPIO复用为PWC引脚或还原为GPIO引脚.
 *
 * @param[in]  PWCChSel  PWC通道选择，详见 #PWC 输出通道GPIO复用关系 的宏定义
 * @param[in]  PWCIoSel  例如：当PWCChSel = TIMER3_PWC_A23_A27_B0_B2：
 * 							  0：复用于A23
 * 							  1：复用于A27
 * 							  2：复用于B0
 * 							  3：复用与B2
 * @param[in]  PWCMode   1 - 复用为PWC引脚， 0 - 还原为普通GPIO引脚，详见PWC_IO_MODE
 *
 * @return     无
 */
void PWC_GpioConfig(uint8_t PWCChSel, uint8_t PWCIoSel, uint8_t PWCMode);



/**
 * @brief     选择一个定时器下PWC通道并配置参数
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 * @param[in] PWCParam  PWC初始化参数，详细参考 PWCInfo
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Config(TIMER_INDEX TimerIdx, PWC_StructInit *PWCParam);

/**
 * @brief     使能TimerIdx下的PWC通道进行采集
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Enable(TIMER_INDEX TimerIdx);

/**
 * @brief     关闭TimerIdx下的PWC通道进行采集
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 *
 * @return    错误号：0 - 正确，其他为错误，详细参考PWC_ERROR_CODE
 */
PWC_ERROR_CODE PWC_Disable(TIMER_INDEX TimerIdx);


/**
 * @brief     读取TimerIdx下的一路PWC捕获的数据
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 *
 * @return    0xffff -- 数据溢出，其他：PWC获取到的数据
 */
uint32_t PWC_CaptureValueGet(TIMER_INDEX TimerIdx);



/**
 * @brief     TimerIdx下的PWC操作，建议用下面PWC_IOCTRL_V2接口
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 * @param[in] Cmd  PWC IOCTROL 命令
 * @param[in] Arg  PWC IOCTROL 参数
 *
 * @return    >=0: 正确数值， < 0: 错误返回值
 */
PWC_ERROR_CODE PWC_IOCTRL(TIMER_INDEX TimerIdx, PWC_IOCTRL_CMD Cmd, PWC_IOCTRL_ARG *Arg);

/**
 * @brief     TimerIdx下的PWC操作,V2版本，新增OutData 用于返回get命令的数据，建议用此接口
 *
 * @param[in] TimerIdx  定时器索引号，只支持TIMER3、TIMER4
 * @param[in] Cmd  PWC IOCTROL 命令
 * @param[in] Arg  PWC IOCTROL 参数
 * @param[in] *OutData  存储get命令获取到的数据
 *
 * @return    >=0: 正确数值， < 0: 错误返回值
 */
PWC_ERROR_CODE PWC_IOCTRL_V2(TIMER_INDEX TimerIdx, PWC_IOCTRL_CMD Cmd, PWC_IOCTRL_ARG *Arg, unsigned long *OutData);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__PWC_H__

/**
 * @}
 * @}
 */

