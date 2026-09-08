/**
 *************************************************************************************
 * @file	wav_io.h
 * @brief	Simple routines for reading/writing Microsoft WAVE (".wav") sound file 
 *
 * @author	Zhao Ying (Alfred)
 * @version	V1.2.1
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *
 *************************************************************************************
 */

#ifndef _WAV_IO_H
#define _WAV_IO_H

#include <stdio.h>
#include "typedefine.h"

// the critical information of a WAV file
typedef struct _WavInfo
{
	uint32_t sample_rate;		// sample rate in Hz
	uint32_t num_samples;		// total number of samples contained in WAV file
	uint16_t num_channels;		// number of channels
	uint16_t bits_per_sample;	// bits per sample (8/16/24/32)
} WavInfo;

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
* @brief open the wave (.wav) file for reading purpose
*/
FRESULT wav_read_open(FIL* pfilehandle, const char* path, WavInfo* wi);


/**
* @brief read the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_read(FIL* pfilehandle, int16_t* ptr, uint32_t items);


/**
* @brief read the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_read24(FIL* pfilehandle, int32_t* ptr, uint32_t items);

/**
* @brief close the wave (.wav) file which is previously opened for reading purpose
*/
void wav_read_close(FIL* pfilehandle);


/**
* @brief open the wave (.wav) file for writing purpose
*/
FRESULT wav_write_open(FIL* pfilehandle, const char* path);


/**
* @brief write the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_write(FIL* pfilehandle, int16_t* ptr, uint32_t items);


/**
* @brief write the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_write24(FIL* pfilehandle, int32_t* ptr, uint32_t items);


/**
* @brief close the wave (.wav) file which is previously opened for writing purpose
*/
FRESULT wav_write_close(FIL* pfilehandle, WavInfo* wi);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //_WAV_IO_H