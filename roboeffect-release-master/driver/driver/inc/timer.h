/**
 **************************************************************************************
 * @file    timer.h
 * @brief   Timer (Basic Timer 1 & 2, General Timer 3 & 4, Advanced Timer 5 & 6 ) API
 *
 * @author  Grayson Chen
 * @version V1.0.0
 *
 * $Created: 2017-09-28 10:19:30$
 *
 * @copyright Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 **************************************************************************************
 */
 
 /*
 ======================================================================================
                       ##### 如何使用定时器相关的函数API接口 #####
 ======================================================================================
   [..]该驱动程序为P2系列芯片的定时器提供了函数接口
       这些函数按照功能，被划分在了3个头文件中，其中在同一个通道中3种功能为互斥关系：
       
       （##）基本定时功能：timer.h 
       
            (++)选择一个定时器，设置定时时间以及定时模式   --  TimerStart
            (++)可以通过读取中断标志来判断定时是否已到      --  TimerInterruptFlagGet
            (++)在定时器运行过程中，定时器选择是否暂停      --  TimerPause
            (++)在调试模式下，可以选择是否支持暂停             --  TimerSetHaltEnabled
            (++)读取剩余时间                                                   --  TimerGetResidualTime
       
       （##）PWM输出功能： pwm.h
       
            （+）标准输出/强制输出
                (++)将GPIO复用为PWM引脚
                (++)选择一个定时器，选择该定时器下的一个通道
                (++)选择为计数模式（向上计数/向下计数/中间对齐计数）
                (++)选择一种输出类型（强制输出/标准输出1/标准输出2）
                (++)配置PWM频率以及占空比
                (++)是否使用DMA控制占空比
                (++)选择是单独输出还是差分输出
                (++)使能
                (++)支持动态配置占空比
                
             (+) 组合输出
                 同一个定时器中，只支持PWM1和PWM2组合， PWM3和PWM4组合。
                (++)配置两路的计数模式
                (++)配置两路频率，占空比等参数
                (++)配置一路为组合类型输出，另一路配置为标准类型输出
                (++)使能
                 
             (+) 不对称输出
                (++)配置PWM1和PWM2两路占空比相加与PWM3和PWM4两路占空比相加相等
                (++)配置输出类型为不对称输出类型
                (++)使能
                
             
             (+) 死区和断路插入
                (++)配置死区的相关的参数
                (++)使能
                
                (++)配置断路插入的极性
                (++)配置断路插入的滤波器参数
                (++)使能
            
       
       （##）PWC输入功能： pwc.h
       
            (++)复用GPIO引脚为PWC引脚
            (++)选择一个定时器，选择该定时器下的一路PWC输入通道
            (++)选择捕获极性（边缘到边缘/上升沿到上升沿/下降沿到下降沿）
            (++)配置PWC的时间量程
            (++)配置信号输入的滤波器参数
            (++)使能
       
       （##）DMA控制功能： timer.h
       
            在高级定时器中，每个独立的定时器支持4路PWC,PWM以及基本定时功能，但是只能有一个
            为主模式来控制定时器的计数或DMA请求。
            
            (++)选择一个定时器索引只支持TIMER5和TIMER6，选择一个控制源
 **************************************************************************************
 */

/**
 * @addtogroup TIMER
 * @{
 * @defgroup timer timer.h
 * @{
 */

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#include "type.h"

//#define  TIMER_MAX_TIMEOUT_VALUE_IN_US  (23860201)      // 最大可设置的周期值 microsecond(us) = 65535 * 65535 / 180


/**
 * @brief   定时器索引号
 * @note    AP82有6个定时器，其中TIMER1已经用于Systick功能
 */
typedef enum __TIMER_INDEX
{
	TIMER1 = 0,
    TIMER2 = 1,
    TIMER3,
    TIMER4,
    TIMER5,
    TIMER6,

} TIMER_INDEX;




/**
 * @brief   定时器中断信号源选择
 */
typedef enum __TIMER_INTERRUPT_SRC
{
	UPDATE_INTERRUPT_SRC = 1,          //定时器更新中断
	PWC_OVER_RUN_INTERRUPT_SRC = 2,    //PWC溢出中断
	PWC1_CAP_DATA_INTERRUPT_SRC = 4,   //PWC捕获中断
	PWM1_COMP_INTERRUPT_SRC = 64,      //PWM比较中断
	PWC1_OVER_CAP_INTERRUPT_SRC = 1024 //PWC重复捕获中断
} TIMER_INTERRUPT_SRC;

/**
 * @brief   定时器配置错误类型定义
 */
typedef enum __TIMER_ERROR_CODE
{
    TIMER_ERROR_INVALID_TIMER_INDEX = -128,
    TIMER_INTERRUPT_SRC_SEL,
    TIMER_ERROR_OK = 0
}TIMER_ERROR_CODE;


/**
 * @brief      定时器时钟源选择
 *
 * @param[in]  TimerIndex        定时器索引, 只支持TIMER3、TIMER4
 * @param[in]  rcClock           1=RC时钟(12M),0=系统时钟(默认系统时钟)
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_ClockSel(TIMER_INDEX TimerIndex, bool rcClock);
////////////////////////////////////////////////////////////////////////////////////
//
//                      以下是用于定时功能的函数接口
//
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief      配置定时器参数，该函数单位为us
 *
 * @param[in]  TimerIndex        定时器索引, 详见 #TIMER_INDEX
 * @param[in]  usec              定时设置, 单位：微妙(us)
 * @param[in]  SingleShot        0: 连续模式，超时之后，定时器会重新装载计数值.
 *                               1: 单次模式, 超时之后，定时器停止工作。
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */

TIMER_ERROR_CODE Timer_Config(TIMER_INDEX TimerIndex, uint32_t usec, bool SingleShot);

/**
 * @brief      启动定时器
 *
 * @param[in]  TimerIndex        定时器索引, 详见 #TIMER_INDEX
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_Start(TIMER_INDEX TimerIndex);

/**
 * @brief      暂停定时器
 *
 * @param[in]  TimerIndex        定时器索引, 详见 #TIMER_INDEX
 * 			   IsPause           1:暂停TimerIndex计数    0：重新使能TimerIndex计数
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_Pause(TIMER_INDEX TimerIndex, bool IsPause);


/**
 * @brief      只更新定时器的分频比和自动重装值
 *
 * @param[in]  TimerIndex        定时器索引, 详见 #TIMER_INDEX
 * @param[in]  Prescale:         定时器的分频比
 * @param[in]  AutoReload:		 自动重装值
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_TimerClkFreqUpdate(TIMER_INDEX TimerIndex, uint16_t Prescale, uint16_t AutoReload);

/**
 * @brief      当处于调试模式下，定时器是否暂停计数
 *
 * @param[in]  TimerIndex       定时器索引, 详见 #TIMER_INDEX
 * @param[in]  HaltEnable       1: 当处于调试模式时，暂停使能,  
 *                              0: 当处于调试模式时，计数器不会暂停
 *
 * @return    错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE  Timer_SetHaltEnabled(TIMER_INDEX TimerIndex, bool HaltEnable);

/**
 * @brief      获取剩余时间
 *
 * @param[in]  TimerIndex   定时器索引, 详见 #TIMER_INDEX
 *
 * @return     剩余时间.
 */
uint32_t Timer_GetResidualTime(TIMER_INDEX TimerIndex);

////////////////////////////////////////////////////////////////////////////////////
//
//                      以下是定时器中断相关函数
//
////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief      使能中断源
 *
 * @param[in]  TimerIndex      定时器索引, 详见 #TIMER_INDEX
 * @param[in]  IntSrc:         中断源选择，可以选择多个
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterrputSrcEnable(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);
/**
 * @brief      关闭中断源
 *
 * @param[in]  TimerIndex      定时器索引, 详见 #TIMER_INDEX
 * @param[in]  IntSrc:         中断源选择，可以选择多个
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterrputSrcDisable(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);

/**
 * @brief      获取定时器中断标志
 *
 * @param[in]  TimerIndex    定时器索引
 * @param[in]  IntSrc:       中断源选择，可以选择多个#TIMER_INTERRUPT_SRC
 *
 * @return     1: 产生中断  0:无中断产生， TIMER_ERROR_INVALID_TIMER_INDEX--错误的定时器索引
 */
TIMER_ERROR_CODE Timer_InterruptFlagGet(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);

/**
 * @brief      清除定时器中断
 *
 * @param[in]  TimerIndex    定时器索引
 * @param[in]  IntSrc:       中断源选择，只能选择1个 #TIMER_INTERRUPT_SRC
 *
 * @return     错误号, 详见 #TIMER_ERROR_CODE
 */
TIMER_ERROR_CODE Timer_InterruptFlagClear(TIMER_INDEX TimerIndex, TIMER_INTERRUPT_SRC IntSrc);




/**
 * @brief      清除系统定时器中断标识
 *
 * @param	   无
 * @return   无
 */
void SysTimerIntFlagClear(void);

#ifdef  __cplusplus
}
#endif//__cplusplus

#endif//__TIMER_H__

/**
 * @}
 * @}
 */
