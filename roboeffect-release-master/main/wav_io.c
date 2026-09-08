/**
 *************************************************************************************
 * @file     wav_io.c
 * @brief    Simple routines for reading/writing Microsoft WAVE (".wav") sound file
 *
 * @author	Zhao Ying (Alfred)
 * @version	V1.2.1
 *
 * &copy; Shanghai Mountain View Silicon Technology Co.,Ltd. All rights reserved.
 *************************************************************************************
 */

#include <stdlib.h>
#include "ff.h"
#include "wav_io.h"

///** wave file header */
//static uint8_t wav_header[44]={
//    'R','I','F','F',//  0 - ChunkID
//    0,0,0,0,        //  4 - ChunkSize (filesize-8)
//    'W','A','V','E',//  8 - Format
//    'f','m','t',' ',// 12 - SubChunk1ID
//    16,0,0,0,       // 16 - SubChunk1Size  // 16 for PCM
//    1,0,            // 20 - AudioFormat (1=Uncompressed)
//    2,0,            // 22 - NumChannels
//    0,0,0,0,        // 24 - SampleRate in Hz
//    0,0,0,0,        // 28 - Byte Rate (SampleRate*NumChannels*(BitsPerSample/8)
//    4,0,            // 32 - BlockAlign (== NumChannels * BitsPerSample/8)
//    16,0,           // 34 - BitsPerSample
//    'd','a','t','a',// 36 - Subchunk2ID
//    0,0,0,0         // 40 - Subchunk2Size
//};

// the size of the following WAV header structure should be 44.
typedef struct _WavFileHeader
{
	uint8_t	chunk_id[4];		//  0 - ChunkID: "RIFF"
	uint32_t chunk_size;		//  4 - ChunkSize: (filesize-8)
	uint8_t format[4];			//  8 - Format: "WAVE"
	uint8_t subchunk1_id[4];	// 12 - SubChunk1ID: "fmt "
	uint32_t subchunk1_size;	// 16 - SubChunk1Size: 16 for PCM (18 o 40 for others)
	uint16_t audio_format;		// 20 - AudioFormat: (1=Uncompressed)
	uint16_t num_channels;		// 22 - NumChannels: 1 or 2
	uint32_t sample_rate;		// 24 - SampleRate in Hz
	uint32_t byte_rate;			// 28 - Byte Rate (SampleRate*NumChannels*(BitsPerSample/8)
	uint16_t block_align;		// 32 - BlockAlign (== NumChannels * BitsPerSample/8)
	uint16_t bits_per_sample;	// 34 - BitsPerSample: 16 bit
	uint8_t subchunk2_id[4];	// 36 - Subchunk2ID: "data"
	uint32_t subchunk2_size;	// 40 - Subchunk2Size: BitsPerSample/8*NumChannels*NumSamples;
} WavFileHeader;

static WavFileHeader wfh;


/**
* @brief open a wave (.wav) file for reading purpose
*/
FRESULT wav_read_open(FIL* pfilehandle, const char* path, WavInfo* wi)
{	
	FRESULT ret;
	UINT br;
	ret = f_open(pfilehandle, path, FA_READ | FA_OPEN_EXISTING);
	if (ret != FR_OK)
		return ret;

	f_read(pfilehandle, &wfh, sizeof(wfh), &br);
	if(br < sizeof(wfh))
		return FR_NOT_ENABLED;
	
	wi->sample_rate = wfh.sample_rate;
	wi->num_channels = wfh.num_channels;
	wi->bits_per_sample =wfh.bits_per_sample;
	wi->num_samples = wfh.subchunk2_size/(wfh.bits_per_sample/8)/wfh.num_channels;

	return ret;
}

/**
* @brief read the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_read(FIL* pfilehandle, int16_t* ptr, uint32_t items)
{
	FRESULT ret;
	UINT br;
	ret = f_read(pfilehandle, ptr, sizeof(int16_t) * items, &br);
	if(ret != FR_OK)
		return 0;
	
	return br/sizeof(int16_t);

}


/**
* @brief read the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_read24(FIL* pfilehandle, int32_t* ptr, uint32_t items)
{
	uint32_t i;
	FRESULT ret;
	UINT br;
	uint8_t p[4] = { 0,0,0,0 };
	for (i = 0; i < items; i++)
	{	
		ret = f_read(pfilehandle, p, sizeof(uint8_t) * 3, &br);
		if (ret == FR_OK && br == 3)
		{
			int32_t t = *(int32_t*)p;
			ptr[i] = t >= 8388608 ? t - 16777216 : t; // convert unsigned to signed
		}
		else
			break;
	}
	return i;
}


/**
* @brief close a wave (.wav) file which is opened for reading purpose
*/
void wav_read_close(FIL* pfilehandle)
{
	if (pfilehandle != NULL)
		f_close(pfilehandle);
}


/**
* @brief open the wave (.wav) file for writing purpose
*/
FRESULT wav_write_open(FIL* pfilehandle, const char* path)
{
	FRESULT ret;
	UINT br;
	ret = f_open(pfilehandle, path, FA_CREATE_ALWAYS | FA_WRITE);

	if (ret != FR_OK)
		return ret; // Cannot open file for writing

	// Just occupy the size of a WAV header. No need to care about its content for the time being.
	ret = f_write(pfilehandle, &wfh, sizeof(wfh), &br);
	if(ret != FR_OK || br < sizeof(wfh))
		return FR_NOT_ENABLED;
    
    return FR_OK;
}


/**
* @brief write the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_write(FIL* pfilehandle, int16_t* ptr, uint32_t items)
{
	FRESULT ret;
	UINT br;
	ret = f_write(pfilehandle, ptr, sizeof(int16_t) * items, &br);
	if(ret != FR_OK || br < sizeof(int16_t) * items)
		return 0;
	return br;
}


/**
* @brief write the wave (.wav) file. The items parameter must be an integer product of the number of channels.
*/
uint32_t wav_write24(FIL* pfilehandle, int32_t* ptr, uint32_t items)
{
	FRESULT ret;
	UINT br;
	uint32_t i;
	for (i = 0; i < items; i++)	
	{
		uint8_t *p = (uint8_t*)(&(ptr[i]));
		ret = f_write(pfilehandle, p, 3, &br);
		if(ret != FR_OK || br < 3)
			break;
	}
	return i;	
}


/**
* @brief close the wave (.wav) file which is previously opened for writing purpose
*/
FRESULT wav_write_close(FIL* pfilehandle, WavInfo* wi)
{
	uint32_t file_size;
	FRESULT ret;
	UINT br;

	if (pfilehandle == NULL)
		return FR_INVALID_OBJECT;
	
	file_size = f_tell(pfilehandle);
	f_lseek(pfilehandle, 0);

	wfh.chunk_id[0]='R'; wfh.chunk_id[1]='I'; wfh.chunk_id[2]='F'; wfh.chunk_id[3]='F';
	wfh.chunk_size = file_size - 8;
	wfh.format[0]='W'; wfh.format[1]='A'; wfh.format[2]='V'; wfh.format[3]='E';
	wfh.subchunk1_id[0]='f'; wfh.subchunk1_id[1]='m'; wfh.subchunk1_id[2]='t'; wfh.subchunk1_id[3]=' ';
	wfh.subchunk1_size = 16;
	wfh.audio_format = 1;
	wfh.num_channels = wi->num_channels;
	wfh.sample_rate = wi->sample_rate;
	wfh.byte_rate = wi->sample_rate*wi->num_channels*wi->bits_per_sample/8;
	wfh.block_align = wi->num_channels*wi->bits_per_sample/8;
	wfh.bits_per_sample = wi->bits_per_sample; // usually 16-bit
	wfh.subchunk2_id[0]='d'; wfh.subchunk2_id[1]='a'; wfh.subchunk2_id[2]='t'; wfh.subchunk2_id[3]='a';
	wfh.subchunk2_size = (wi->bits_per_sample/8)*wi->num_channels*wi->num_samples;

	f_write(pfilehandle, &wfh, sizeof(wfh), &br);
	return f_close(pfilehandle);
}
