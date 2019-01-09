#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>        
#include <linux/videodev2.h>
#include "memory_pool.h"
#include "jpeg_util.h"
#include "video_util.h"

#define SYS_ERR(s) do{    \
	fprintf(stderr,"[%s:%d] %s:%s\n",__FILE__,__LINE__,s,strerror(errno));\
	exit(-1);\
}while(0)


int init_v4l2_device(VideoV4l2 *video,int frame_buff_count,memonry_pool_t*pool)
{
	assert(video != NULL);
	assert(pool != NULL);
	int ret;
	//获取摄像头信息
    struct v4l2_capability cap;
    ret = ioctl(video->videofd, VIDIOC_QUERYCAP, &cap);
	if(ret < 0)
		SYS_ERR("query the information failed");
#ifdef DEBUG
    printf("========= Capability Informations: ==========\n");
    printf(" driver: %s\n", cap.driver);
    printf(" card: %s\n", cap.card);
    printf(" bus_info: %s\n", cap.bus_info);
    printf(" version: %08X\n", cap.version);
    printf(" capabilities: %08X\n", cap.capabilities);
	printf("=============================================\n");
	//获取摄像头支持的视频格式
	struct v4l2_fmtdesc fmtdesc;
	fmtdesc.index = 0; //form number
	fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;//frame type  
	 printf("========= Video Format Informations: ==========\n");
	while(ioctl(video->videofd,VIDIOC_ENUM_FMT,&fmtdesc) != -1){  
        //if(fmtdesc.pixelformat && fmt.fmt.pix.pixelformat){
            printf("VIDIOC_ENUM_FMT pixelformat:%s\n",fmtdesc.description);
        //}
        fmtdesc.index ++;
    }
	printf("===============================================\n");
#endif
	//获取摄像头视频格式

    struct v4l2_format fmt;
	fmt.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ret = ioctl(video->videofd, VIDIOC_G_FMT, &fmt);
	if(ret < 0)
		SYS_ERR("ioctl failed");

#ifdef DEBUG
	printf("=========== Stream Format Informations: ========\n");
    printf(" type: %d\n", fmt.type);
    printf(" width: %d\n", fmt.fmt.pix.width);
    printf(" height: %d\n", fmt.fmt.pix.height);
#endif
	video->width = fmt.fmt.pix.width;
	video->height = fmt.fmt.pix.height;
    char fmtstr[8];
    memset(fmtstr, 0, 8);
    memcpy(fmtstr, &fmt.fmt.pix.pixelformat, 4);
	if(strcmp(fmtstr,"V4L2_PIX_FMT_RGB24") == 0)
		video->support_rgb24_flag = 1;
	else
		video->support_rgb24_flag = 0;
#ifdef DEBUG
    printf(" pixelformat: %s\n", fmtstr);
    printf(" field: %d\n", fmt.fmt.pix.field);
    printf(" bytesperline: %d\n", fmt.fmt.pix.bytesperline);
    printf(" sizeimage: %d\n", fmt.fmt.pix.sizeimage);
    printf(" colorspace: %d\n", fmt.fmt.pix.colorspace);
    printf(" priv: %d\n", fmt.fmt.pix.priv);
    printf(" raw_date: %s\n", fmt.fmt.raw_data);
	printf("===============================================\n");
#endif
	//设置视频格式
	memset(&fmt, 0, sizeof(fmt));
    fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width       = video->width;
    fmt.fmt.pix.height      = video->height;
	if(video->support_rgb24_flag)
    	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
	else
    	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;

    fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
    ret = ioctl(video->videofd, VIDIOC_S_FMT, &fmt);
    if (ret < 0)
		SYS_ERR("ioctl format failed");

	//从内存池中申请空间并映射
	video->frame_buff_count = frame_buff_count;
	video->framebuf = (VideoBuffer*)memonry_pool_alloc(pool,frame_buff_count* sizeof(VideoBuffer));
	if(video->framebuf == NULL)
	{
		printf("Allocate memony failed from memony pool\n");
		exit(-1);
	}

	allocate_memory(video);
	start_capture_frame(video->videofd);
	return 0;
}

int start_capture_frame(int fd)
{
	enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    int ret = ioctl(fd, VIDIOC_STREAMON, &type);
    if (ret < 0)
		SYS_ERR("ioctl failed");
	return 0;
}

int allocate_memory(VideoV4l2 *video)
{
	assert(video != NULL);
	struct v4l2_requestbuffers reqbuf;
    
    reqbuf.count = video->frame_buff_count;
    reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbuf.memory = V4L2_MEMORY_MMAP;
    
    int ret = ioctl(video->videofd , VIDIOC_REQBUFS, &reqbuf);
    if(ret < 0) 
		SYS_ERR("reqbufs failed");

	int i;
	for (i = 0; i < reqbuf.count; i++) 
    {
        video->buf.index = i;
        video->buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        video->buf.memory = V4L2_MEMORY_MMAP;
        ret = ioctl(video->videofd , VIDIOC_QUERYBUF, &(video->buf));
        if(ret < 0)
			SYS_ERR("querybuf failed");

        // mmap buffer
        video->framebuf[i].length = video->buf.length;
        video->framebuf[i].start = (char *) mmap(0, video->buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, video->videofd, video->buf.m.offset);
        if (video->framebuf[i].start == MAP_FAILED)
			SYS_ERR("mmap failed");
    
        // Queen buffer
        ret = ioctl(video->videofd , VIDIOC_QBUF, &(video->buf));
        if (ret < 0)
			SYS_ERR("ioctl qbuf failed");
    }
	return 0;
}

int holder_next_frame(VideoV4l2 *video,uint8_t *rgb24)
{
	assert(video != NULL);
	assert(rgb24 != NULL);
	int ret = ioctl(video->videofd, VIDIOC_DQBUF, &(video->buf));
    if(ret < 0)
		SYS_ERR("ioctl dqbuf failed");

	//处理原始图像数据
	uint8_t *tmpdata = (uint8_t *)video->framebuf[video->buf.index].start; 
	if(!video->support_rgb24_flag)
		yuyv_to_rgb(tmpdata,rgb24,video->width,video->height);
	else
		memcpy(rgb24,tmpdata,video->width*video->height*3);
	//释放缓冲区
	release_one_frame(video);
	return 0;
}

int release_one_frame(VideoV4l2 *video)
{
    int ret = ioctl(video->videofd, VIDIOC_QBUF, &(video->buf));
    if (ret < 0)
		SYS_ERR("ioctl qbuf failed");
	return 0;
}

int release_memory(VideoV4l2*video)
{
	int i;
	for (i=0; i< video->frame_buff_count; i++) 
        munmap(video->framebuf[i].start, video->framebuf[i].length);

	//free(video->framebuf);
	//close(video->videofd);
	return 0;
}
