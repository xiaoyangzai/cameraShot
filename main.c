#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>           
#include <fcntl.h>            
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <asm/types.h>        
#include <linux/videodev2.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "memory_pool.h"
#include "jpeg_util.h"
#include "video_util.h"

#define SYS_ERR(s) do{    \
	fprintf(stderr,"[%s:%d] %s:%s\n",__FILE__,__LINE__,s,strerror(errno));\
	exit(-1);\
}while(0)


int main(int argc,char *argv[])
{
	if(argc != 3)
	{
		printf("usage: ./client [video device path] [Jpeg Image Path To Save]\n");
		return -1;
	}

	VideoV4l2 video;
	//打开摄像头
	video.videofd = open(argv[1],O_RDWR);
	if(video.videofd < 0)
		SYS_ERR("open video device failed");
	memory_pool_t *pool = memory_pool_create(10*1024*1024);
	if(pool == NULL)
	{
		printf("create memory pool failed\n");
		return -1;
	}
	init_v4l2_device(&video,4,pool);
	
	uint8_t *rgb24 = (uint8_t *)memory_pool_alloc(pool,video.width*video.height*3);
	if(!rgb24)
	{
		printf("allocate memory failed\n");
		return -1;
	}
	printf("holder next fram...\n");
	//uint8_t *resizedata = (uint8_t *)memory_pool_alloc(pool,224*224*3);
	holder_next_frame(&video,rgb24);
	//holder_next_frame(&video,rgb24);

	printf("hold the next frame.\n");
	uint8_t *outbuffer = NULL;
	uint64_t outlen = 0;
	encode_jpeg(rgb24,video.width,video.height,&outbuffer,&outlen,pool);
	int fd = open(argv[2],O_WRONLY|O_CREAT,0666);
	if(fd < 0)
		SYS_ERR("open failed");
	if(write(fd,outbuffer,outlen) != outlen)
		SYS_ERR("weite failed");
	close(fd);
	printf("The camera shot has been saved into %s\n",argv[2]);
	memory_pool_destroy(pool);

	//holder_next_frame(&video,rgb24);
	//scale_rgb24(rgb24,resizedata,video.width,video.height,224,224);
	//printf("R: %d\t G:%d\t B:%d\n",resizedata[0],resizedata[1],resizedata[2]);
		
	return 0;
}
