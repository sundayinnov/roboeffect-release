/*
 * retarget.c
 *
 *  Created on: Mar 8, 2017
 *      Author: peter
 */

#include <stdio.h>
#include "uarts_interface.h"
#include "type.h"
#include "remap.h"
#include "user_config.h"
#include "sw_uart.h"

uint8_t DebugPrintPort = 0xff;//UART_PORT0;
uint32_t gSramEndAddr = SRAM_END_ADDR;

uint8_t IsSwUartActedAsUARTFlag = 0;
void EnableSwUartAsUART(uint8_t EnableFlag)
{
	IsSwUartActedAsUARTFlag = EnableFlag;
}

//This is used as dummy function in case that appilcation dont define this function.
__attribute__((weak))
void SwUartSendTest(unsigned char* Buf, unsigned int BufLen)
{


}
__attribute__((used))
int putchar(int c)
{
#if CFG_DEBUG_EN

    if(DebugPrintPort==0xff) return c;

    if(IsSwUartActedAsUARTFlag)
    {
        if((unsigned char)c == '\n')
        {
            const char lfca[2] = "\r\n";
            SwUartSend((unsigned char*)lfca, 2);
        }
        else
        {
            SwUartSend((unsigned char*)&c, 1);
        }
    }
    else
    {
    	if (c == '\n')
    	{
    		UARTS_SendByte(DebugPrintPort, '\r');
    		UARTS_SendByte(DebugPrintPort, '\n');
    	}
    	else
    	{
    		UARTS_SendByte(DebugPrintPort, (uint8_t)c);
    	}
    }

#endif	

	return c;
}

__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{
#if CFG_DEBUG_EN
	int i;
	for (i = 0; i < size; i++)
	{
		putchar(buf[i]);
	}
#endif
}

int DbgUartInit(int Which, unsigned int BaudRate, unsigned char DatumBits, unsigned char Parity, unsigned char StopBits)
{
	DebugPrintPort = Which;

	EnableSwUartAsUART(0);

	if(DebugPrintPort==0xff) return 1;

	if(DebugPrintPort == UART_SW_N0)
	 {
		EnableSwUartAsUART(1);

		//GPIO_RegOneBitSet(GPIO_B_OE, 1);
		//GPIO_RegOneBitClear(GPIO_B_IE, 0);
		if(UART_SW_PORT == GPIO_A_START)
			GPIO_PortAModeSet(UART_SW_PORT_PIN, 0);
		else if(UART_SW_PORT == GPIO_B_START)
			GPIO_PortBModeSet(UART_SW_PORT_PIN, 0);
		SwUartTxInit(UART_SW_PORT ,UART_SW_PORT_PIN , BaudRate);

	   return 1;
	 }
		
	return UARTS_Init(Which, BaudRate, DatumBits,  Parity,  StopBits);
}


