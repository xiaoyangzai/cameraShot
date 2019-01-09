#ifndef __JPEG_DISPALY_H__
#define __JPEG_DISPALY_H__
#include <stdint.h>
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

#include "memory_pool.h"



/*This returns an array for a 24 bit image.*/
uint8_t *decode_jpeg (const char *filename, uint32_t *width,uint32_t *height,uint32_t *bpp,memonry_pool_t *pool);

/*
	The encode_jpeg function will convert the rgb24 format data specified by rgb24 into the buffer pointed to by outbuffer. The outlen specifies the length of the JPEG image data.

	Description:
		rgb24 	Specifies a image data of RGB format with 3 chinnals.
		width	The width of the RGB image data.
		height 	The height of the RGB image data.
		outbuffer	A pointer to a buffer where the JPEG image data shall bereturned.	
		outlen	A pointer to a buffer specified the length of the stored JPEG image data. 
		pool	A pointer to the memory pool.

	Return:
		Upon successfully completion, encode_jpeg shall return the 0. Otherwise, -1 shall be returned. 
*/
int encode_jpeg(uint8_t*rgb24,int width,int height,uint8_t **outbuffer,uint64_t*outlen,memonry_pool_t*pool);


void write_JPEG_file (char * filename, unsigned char *rgb24,int image_width,int image_height,int quality);

uint32_t * rgb24to32(uint8_t *buf24, uint32_t width,uint32_t height,memonry_pool_t *pool);

uint16_t * rgb24to16(uint8_t *buf24, uint32_t width,uint32_t height,memonry_pool_t*pool);

int scale_rgb24(uint8_t *old_rgb24,uint8_t* new_rgb24, uint32_t old_w,uint32_t old_h,uint32_t new_w,uint32_t new_h);

void yuyv_to_rgb(unsigned char* yuyv,unsigned char* rgb24,int w,int h);

#endif
