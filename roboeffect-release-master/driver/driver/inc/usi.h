/**
 *******************************************************************************
 * @file    usi.h
 * @brief	The driver of usi(7816) module
 *
 * @author  shengqi_zhao
 * @version V1.0.0
 *
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *******************************************************************************
 */
/**
 * @addtogroup USI
 * @{
 * @defgroup usi usi.h
 * @{
 */
 
#ifndef __USI_H__
#define __USI_H__

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus 


/**
 * @brief usi module init
 *
 * @param[in]   none
 * @return  	none
 */
void USI_Init(void);


/**
 * @brief  the mcu initial sequence
 *
 * @param[in]   mbuf 	buffer store the ATR param
 * @return	  	reset succeed if return zero
 */
uint8_t USI_CpuCardReset(uint8_t *mbuf);



/**
 * @brief       baud rate negotiation
 * @param[in]   none
 * @return		succeed if zero
 */
int USI_PPS(void);


/**
 * @brief  send ADPU to ic,and recieve thereponse data
 *
 * @param[in] snd       ADPU sent buffer
 * @param[in] send_len  length of ADPU by bytes
 * @param[in] rec       recieve buffer
 * @param[in] recv_len  the expected length of response packet
 *
 * return	  the returned length in fact
 */
uint8_t	USI_CpuProtocol(uint8_t  *snd, uint32_t send_len,uint8_t  *rec, uint32_t recv_len);


/**
 * @brief recieve one data from ic
 *
 * @param[in]	timeout  timeout
 * @return	    the data recieved,if timeout return -1
 */
int8_t USI_RecieveData(uint32_t timeout);


/**
 * @brief send one data to ic
 *
 * @param[in]  data
 * @return	   none
 */
void USI_SendData(uint8_t data);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif //__USI_H__

/**
 * @}
 * @}
 */
 

