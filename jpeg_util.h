#ifndef __JPEG_DISPALY_H__
#define __JPEG_DISPALY_H__
#include <stdint.h>
#include <stdio.h>
#include <jpeglib.h>
#include <setjmp.h>

/*This returns an array for a 24 bit image.*/
int decode_jpeg (const char *filename, uint32_t *width,uint32_t *height,uint32_t *bpp,uint8_t *outdata);

/*
	The encode_jpeg function will convert the rgb24 format data specified by rgb24 into the buffer pointed to by outbuffer. The outlen specifies the length of the JPEG image data.

	Description:
		rgb24 	Specifies a image data of RGB format with 3 chinnals.
		width	The width of the RGB image data.
		height 	The height of the RGB image data.
		outdata A pointer to the output data buffer.

	Return:
		Upon successfully completion, encode_jpeg shall return the 0. Otherwise, -1 shall be returned. 
*/
int encode_jpeg(uint8_t*rgb24,int width,int height,uint8_t **outbuffer,uint64_t*outlen);


void write_JPEG_file (char * filename, unsigned char *rgb24,int image_width,int image_height,int quality);

int rgb24to32(uint8_t *buf24, uint32_t width,uint32_t height,uint32_t*outdata);

int rgb24to16(uint8_t *buf24, uint32_t width,uint32_t height,uint16_t *outdata);

int scale_rgb24(uint8_t *old_rgb24,uint8_t* new_rgb24, uint32_t old_w,uint32_t old_h,uint32_t new_w,uint32_t new_h);

void yuyv_to_rgb(unsigned char* yuyv,unsigned char* rgb24,int w,int h);

#endif
