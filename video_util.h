#ifndef __VIDEO_UTIL_H__
#define __VIDEO_UTIL_H__
#include <stdint.h>
#include <sys/types.h>
#include <linux/videodev2.h>
#include "memory_pool.h"
typedef struct _VideoBuffer {
    void   *start;
    size_t  length;
}VideoBuffer;

typedef struct _VideoV4l2{
	int videofd;
	int width; 
	int height;
	int support_rgb24_flag;
	int frame_buff_count	;
	VideoBuffer *framebuf;
	struct v4l2_buffer buf;
}VideoV4l2;

int init_v4l2_device(VideoV4l2 *video,int frame_buff_count,memory_pool_t *pool);

int start_capture_frame(int fd);
int allocate_memory(VideoV4l2 *video);
int holder_next_frame(VideoV4l2 *video,uint8_t *data);
int release_one_frame(VideoV4l2 *video);
int release_memory(VideoV4l2*video);
#endif
