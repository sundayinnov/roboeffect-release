/**
  *****************************************************************************
  * @file:			spim.c
  * @author			Lilu
  * @version		V1.0.0
  * @data			2014-11-20
  * @Brief			SPI Master driver interface
  ******************************************************************************
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MVSILICON SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
  * INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2013 MVSilicon </center></h2>
  */

/**
 * @addtogroup SPIM
 * @{
 * @defgroup spim spim.h
 * @{
 */
 
 
#ifndef __SPIM_H__
#define __SPIM_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

#define	SPIM_CLK_RATE_0	0x0			/**< SPI master clock rate seclet  0*/
#define	SPIM_CLK_RATE_1	0x1			/**< SPI master clock rate seclet  1*/
#define	SPIM_CLK_RATE_2	0x2			/**< SPI master clock rate seclet  2*/
#define	SPIM_CLK_RATE_3 0x3			/**< SPI master clock rate seclet  3*/
#define	SPIM_CLK_RATE_4	0x4			/**< SPI master clock rate seclet  4*/
#define	SPIM_CLK_RATE_5	0x5			/**< SPI master clock rate seclet  5*/
#define	SPIM_CLK_RATE_6	0x6			/**< SPI master clock rate seclet  6*/



#define	SPIM_PORT0_A5A6A7			0
#define	SPIM_PORT1_A25A26A27		1
#define	SPIM_PORT2_A29A30A31		2




/**
 * err code define
 */
typedef enum _SPI_MASTER_ERR_CODE
{
    ERR_SPIM_TIME_OUT = -255,			/**<function execute time out*/
    ERR_SPIM_DATALEN_OUT_OF_RANGE,		/**<data len is out of range < 0*/
    SPIM_NONE_ERR = 0,
} SPI_MASTER_ERR_CODE;





/**
 * @brief
 *		初始化SPIM模块
 * @param	Mode
 *				0 - CPOL = 0 & CPHA = 0, idle为低电平,第一个边沿采样(上升沿)
 *				1 - CPOL = 0 & CPHA = 1, idle为低电平,第二个边沿采样(下降沿)
 *				2 - CPOL = 1 & CPHA = 0, idle为高电平,第一个边沿采样(下降沿)
 *				3 - CPOL = 1 & CPHA = 1, idle为高电平,第一个边沿采样(上升沿)
 * @param	ClkRate 	Spim输出时钟速率设置:0 ~ 11
 * 				spim_clockrate = spim_clock / (2 ^ (ClkRate+1)),其中spim_clock默认60MHz
 * @return
 *		初始化成功返回TRUE
 * @note
 */
bool SPIM_Init(uint8_t Mode, uint8_t ClkRate);


/**
 * @brief
 *		SPIM时钟分频系数设置
 * @param	ClkDiv 	Spim时钟分频系数:1 ~ 14
 * 				spim_clock = system_clock / (ClkDiv + 1),其中system_clock默认为120M,ClkDiv默认为1
 * @return
 *		分频比设置成功返回TRUE
 * @note
 */
bool SPIM_ClockDivSet(uint8_t ClkDiv);

//--------------------------------MCU MODE-------------------------------------
/**
 * @brief
 *		MCU模式发送数据
 * @param	SendBuf		发送数据首地址
 * @param	Length		发送数据长度（单位：Byte）
 * @return
 *		返回错误号
 * @note
 */
SPI_MASTER_ERR_CODE SPIM_Send(uint8_t* SendBuf, uint32_t Length);

/**
 * @brief
 *		MCU模式接收数据
 * @param	RecvBuf		接收数据首地址
 * @param	Length		接收数据长度（单位：Byte）
 * @return
 *		返回错误号
 * @note
 */
SPI_MASTER_ERR_CODE SPIM_Recv(uint8_t* RecvBuf, uint32_t Length);

/**
 * @brief
 *		MCU模式发送1字节数据
 * @param	SendByte  单字节数据
 * @return
 *		无
 */
void SPIM_SendByte(uint8_t SendByte);

/**
 * @brief
 *		MCU模式接收1字节数据
 * @param	无
 * @return
 *		返回接收的单字节数据
 */
uint8_t SPIM_RecvByte(void);

//--------------------------------DMA MODE-------------------------------------
/**
 * @brief
 *		DMA模式发送数据
 * @param	SendBuf	数据缓冲区首地址
 * @param	Length	数据长度（单位：Byte），最大65536 byte
 * @return
 *		返回错误号
 * @Note
 */
SPI_MASTER_ERR_CODE SPIM_DMA_Send(uint8_t* SendBuf, uint32_t Length);

/**
 * @brief
 *		DMA模式接收数据
 * @param	RecvBuf 数据首地址
 * @param	Length	数据长度（单位：Byte），最大65536 byte
 * @return
 *		返回错误号
 * @note
 */
SPI_MASTER_ERR_CODE SPIM_DMA_Recv(uint8_t* RecvBuf, uint32_t Length);

/**
 * @brief
 *		启动DMA模式发送数据，非阻塞
 * @param	SendBuf	数据缓冲区首地址
 * @param	Length	数据长度（单位：Byte），最大65536 byte
 * @return
 *		无
 * @note SPIM中断标志置位标志发送数据完成，可使用查询或中断方式
 */
void SPIM_DMA_StartSend(uint8_t* SendBuf, uint32_t Length);

/**
 * @brief
 *		启动DMA模式接收数据，非阻塞
 * @param	RecvBuf 数据首地址
 * @param	Length	数据长度（单位：Byte），最大65536 byte
 * @return
 *		无
 * @note DMA D标志置位标志接收数据完成，可使用查询或中断方式
 */
void SPIM_DMA_StartRecv(uint8_t* RecvBuf, uint32_t Length);
/**
 * @brief
 *		中断使能
 * @param	无
 * @return
 *		无
 */
void SPIM_IntEn(void);

/**
 * @brief
 *		获取中断标志
 * @param	无
 * @return
 *		返回中断标志
 */
bool SPIM_GetIntFlag(void);

/**
 * @brief
 *		清除中断标志
 * @param	无
 * @return
 *		无
 */
void SPIM_ClrIntFlag(void);

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__SPIM_H__

/**
 * @}
 * @}
 */
