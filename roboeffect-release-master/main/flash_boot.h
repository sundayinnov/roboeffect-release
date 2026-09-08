#ifndef __FLASH_BOOT_H__
#define __FLASH_BOOT_H__



/*
bulid time:Tue Mar 07 10:32:30 2023
*/
#define FLASH_BOOT_EN      1

//flash boot uart tx only
//BOOT_UART_OPEN,open:0x55,close:0xff
#define BOOT_UART_OPEN     0xFF

//BOOT_UART_TX_PIN,only use GPIOA13 or GPIOB7
#define BOOT_UART_TX_A13   0x00
#define BOOT_UART_TX_B7    0x01
#define BOOT_UART_TX_PIN    BOOT_UART_TX_B7

#define CONST_DATA_ADDR    0xD0000
#define USER_DATA_ADDR     0x1D0000
#define AUDIO_EFFECT_ADDR  0x1e0000

#if FLASH_BOOT_EN
extern const unsigned char flash_data[];
#endif


#endif

