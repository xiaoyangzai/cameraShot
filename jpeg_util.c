/*Copyright George Peter Staplin 2003*/

/*You may use/copy/modify/distribute this software for any purpose
 *provided that I am given credit and you don't sue me for any bugs.
 */

/*Please contact me using GeorgePS@XMission.com if you like this, or
 *have questions.
 */
		
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <jpeglib.h>
#include <jerror.h>

#include <time.h>

#include "jpeg_util.h"
#ifndef u_char
#define u_char unsigned char
#endif		



static void jpeg_error_exit (j_common_ptr cinfo) {
  cinfo->err->output_message (cinfo);
  exit (EXIT_FAILURE);
}


/*This returns an array for a 24 bit image.*/
uint8_t *decode_jpeg (const char *filename, uint32_t *width,uint32_t *height,uint32_t *bpp,memonry_pool_t *pool)
{
	register JSAMPARRAY lineBuf;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr err_mgr;
	int bytesPerPix;
	FILE *inFile;
	uint8_t *retBuf;
	
	inFile = fopen (filename, "rb");
	if (NULL == inFile)	{ 
		fprintf (stderr, "Error Open %s: %s\n",filename, strerror(errno));
		return NULL;
	}

	cinfo.err = jpeg_std_error (&err_mgr);
	err_mgr.error_exit = jpeg_error_exit;	

	jpeg_create_decompress (&cinfo);
	jpeg_stdio_src (&cinfo, inFile);
	jpeg_read_header (&cinfo, 1);
	cinfo.do_fancy_upsampling = 0;
	cinfo.do_block_smoothing = 0;
	jpeg_start_decompress (&cinfo);

	*width = cinfo.output_width;
	*height = cinfo.output_height;

	short w,h;
	short *widthPtr = &w;
	short *heightPtr = &h;
	w = *width;
	h = *height;
	
	bytesPerPix = cinfo.output_components;
	*bpp = bytesPerPix;

	lineBuf = cinfo.mem->alloc_sarray ((j_common_ptr) &cinfo, JPOOL_IMAGE, (*widthPtr * bytesPerPix), 1);
	retBuf = (uint8_t *)memonry_pool_alloc(pool,3 * (*widthPtr * *heightPtr));
		
	if (NULL == retBuf) {
		perror (NULL);
		return NULL;
	}
		

	if (3 == bytesPerPix) {
		uint32_t y;
			
		for (y = 0; y < cinfo.output_height; ++y)	{	
			jpeg_read_scanlines (&cinfo, lineBuf, 1);
			memcpy ((retBuf + y * *widthPtr * 3),lineBuf[0],3 * *widthPtr);
		}
	}
	else if (1 == bytesPerPix) { 
		unsigned int col;
		uint32_t lineOffset = (*widthPtr * 3);
		uint32_t lineBufIndex;
		uint32_t x ;
		uint32_t y;
						
		for (y = 0; y < cinfo.output_height; ++y) {
			jpeg_read_scanlines (&cinfo, lineBuf, 1);
				
			lineBufIndex = 0;
			for (x = 0; x < lineOffset; ++x) {
				col = lineBuf[0][lineBufIndex];
			
				retBuf[(lineOffset * y) + x] = col;
				++x;
				retBuf[(lineOffset * y) + x] = col;
				++x;
				retBuf[(lineOffset * y) + x] = col;
				
				++lineBufIndex;
			}			
		}
	}
	else {
		fprintf (stderr, "Error: the number of color channels is %d.  This program only handles 1 or 3\n", bytesPerPix);
		return NULL;
	}
	jpeg_finish_decompress (&cinfo);
	jpeg_destroy_decompress (&cinfo);
	fclose (inFile);
	return retBuf;
}

uint32_t * rgb24to32(uint8_t *buf24, uint32_t width,uint32_t height,memonry_pool_t *pool)
{
	int w = width;
	int h = height;
	
	uint32_t *buf =(uint32_t*)memonry_pool_alloc(pool,w * h * 4);
	if (buf == NULL) 
		return NULL;

	/* FIXME: why? */
	int i;
	for(i = 0; i < w * h; ++i){
		*((uint8_t *)&buf[i] + 2) = buf24[i * 3 + 0];
		*((uint8_t *)&buf[i] + 1) = buf24[i * 3 + 1];
		*((uint8_t *)&buf[i] + 0) = buf24[i * 3 + 2];
		*((uint8_t *)&buf[i] + 3) = 0x00;
	}
	return buf;
}

uint16_t * rgb24to16(uint8_t *buf24, uint32_t width,uint32_t height,memonry_pool_t*pool)
{
	uint16_t *buf16 = NULL;
	
	uint32_t r24,g24,b24;
	uint32_t r16,g16,b16;
	uint16_t rgb16;
	uint32_t i;
	uint32_t w = width,h = height;
	if ((buf16 = (uint16_t *)memonry_pool_alloc(pool,w * h * 2)) == NULL)
		return NULL;

	for (i = 0 ;i < w * h ; i++)	{

		r24 = buf24[i * 3 + 0];
		g24 = buf24[i * 3 + 1];
		b24 = buf24[i * 3 + 2];	

		b16 = (b24 * 0x1f) / 0xff;
		g16 = (g24 * 0x3f) / 0xff;
		r16 = (r24 * 0x1f) / 0xff;
	
		rgb16 = b16 | (g16 <<5) | (r16 <<11);
	
		buf16[i] = rgb16;
	}
	return buf16;
}


int scale_rgb24(uint8_t *old_rgb24,uint8_t* new_rgb24, uint32_t old_w,uint32_t old_h,uint32_t new_w,uint32_t new_h)
{
	uint32_t i,j,dtw,dth;
	

	if (new_rgb24 == NULL)
	{
		fprintf(stderr,"%s\n","bad argument: no memory space for the resized image.");
		return -1;
	}
	
	for(j = 0; j < new_h; j++){
		for(i = 0; i < new_w; i++){

			dtw = i * old_w / new_w;
			dth = j * old_h / new_h;

			new_rgb24[0+i*3 + j*new_w*3] = 
				old_rgb24[0+(dtw+dth*old_w)*3];

			new_rgb24[1+i*3 + j*new_w*3] = 
				old_rgb24[1+(dtw+dth*old_w)*3];
			
			new_rgb24[2+i*3 + j*new_w*3] =
				old_rgb24[2+(dtw+dth*old_w)*3];
		}
	}

	return 0;
}
void write_JPEG_file (char * filename, unsigned char *rgb24,int image_width,int image_height,int quality)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & rgb24[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}

void yuyv_to_rgb(unsigned char* yuyv,unsigned char* rgb24,int w,int h)
{
    int i;
    unsigned char* y0 = yuyv + 0;   
    unsigned char* u0 = yuyv + 1;
    unsigned char* y1 = yuyv + 2;
    unsigned char* v0 = yuyv + 3;
 
    unsigned  char* r0 = rgb24 + 0;
    unsigned  char* g0 = rgb24 + 1;
    unsigned  char* b0 = rgb24 + 2;
    unsigned  char* r1 = rgb24 + 3;
    unsigned  char* g1 = rgb24 + 4;
    unsigned  char* b1 = rgb24 + 5;
   
    float rt0 = 0, gt0 = 0, bt0 = 0, rt1 = 0, gt1 = 0, bt1 = 0;
 
    for(i = 0; i <= (w * h) / 2 ;i++)
    {
        bt0 = 1.164 * (*y0 - 16) + 2.018 * (*u0 - 128); 
        gt0 = 1.164 * (*y0 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt0 = 1.164 * (*y0 - 16) + 1.596 * (*v0 - 128); 
   
    	bt1 = 1.164 * (*y1 - 16) + 2.018 * (*u0 - 128); 
        gt1 = 1.164 * (*y1 - 16) - 0.813 * (*v0 - 128) - 0.394 * (*u0 - 128); 
        rt1 = 1.164 * (*y1 - 16) + 1.596 * (*v0 - 128); 
    
      
       	        if(rt0 > 250)  	rt0 = 255;
		if(rt0< 0)    	rt0 = 0;	
 
		if(gt0 > 250) 	gt0 = 255;
		if(gt0 < 0)	gt0 = 0;	
 
		if(bt0 > 250)	bt0 = 255;
		if(bt0 < 0)	bt0 = 0;	
 
		if(rt1 > 250)	rt1 = 255;
		if(rt1 < 0)	rt1 = 0;	
 
		if(gt1 > 250)	gt1 = 255;
		if(gt1 < 0)	gt1 = 0;	
 
		if(bt1 > 250)	bt1 = 255;
		if(bt1 < 0)	bt1 = 0;	
					
		*r0 = (unsigned char)rt0;
		*g0 = (unsigned char)gt0;
		*b0 = (unsigned char)bt0;
	
		*r1 = (unsigned char)rt1;
		*g1 = (unsigned char)gt1;
		*b1 = (unsigned char)bt1;
 
        yuyv = yuyv + 4;
        rgb24 = rgb24 + 6;
        if(yuyv == NULL)
          break;
 
        y0 = yuyv;
        u0 = yuyv + 1;
        y1 = yuyv + 2;
        v0 = yuyv + 3;
  
        r0 = rgb24 + 0;
        g0 = rgb24 + 1;
        b0 = rgb24 + 2;
        r1 = rgb24 + 3;
        g1 = rgb24 + 4;
        b1 = rgb24 + 5;
    }   
}

/*
	The encode_jpeg function will convert the rgb24 format data specified by rgb24 into the buffer pointed to by outbuffer. The outlen specifies the length of the JPEG image data.

	Description:
		rgb24 	Specifies a image data of RGB format with 3 chinnals.
		width	The width of the RGB image data.
		height 	The height of the RGB image data.
		outbuffer	A pointer to a buffer where the JPEG image data shall bereturned.	
		outlen	A pointer to a buffer specified the length of the stored JPEG image data. 

	Return:
		Upon successfully completion, encode_jpeg shall return the 0. Otherwise, -1 shall be returned. 
*/
int encode_jpeg(uint8_t*rgb24,int width,int height,uint8_t **outbuffer,uint64_t*outlen,memonry_pool_t *pool)
{
	uint8_t *outdata = rgb24;
	struct jpeg_compress_struct cinfo = { 0 };
	struct jpeg_error_mgr jerr;
	JSAMPROW row_ptr[1];
	int row_stride;

	*outbuffer = NULL;
	*outlen = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	jpeg_mem_dest(&cinfo, outbuffer, outlen);
	cinfo.image_width = width;
	cinfo.image_height = height;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_RGB;



	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo,100,TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = width * 3;


	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_ptr[0] = &outdata[cinfo.next_scanline * row_stride];
		jpeg_write_scanlines(&cinfo, row_ptr, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	uint8_t *jpeg_data = (uint8_t *)memonry_pool_alloc(pool,*outlen);
	if(jpeg_data == NULL)
	{
		fprintf(stderr,"memory pool allocate failed\n");
		exit(-1);
	}
	memcpy(jpeg_data,*outbuffer,*outlen);
	free(*outbuffer);
	*outbuffer = jpeg_data;
	return 0;
}
