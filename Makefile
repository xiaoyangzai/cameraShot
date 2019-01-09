all=camerashot
SOURCE=$(wildcard *.c)
OBJS=$(patsubst %.c,%.o,$(SOURCE))
LIBS=-L./libjpeg/lib -ljpeg
CFLAGS=-I./libjpeg/include -DDEBUG
CC=gcc

.PHONY:clean

$(all) : $(OBJS)
	$(CC) $^ -o $@ $(LIBS) $(CFLAGS)

clean:
	-rm $(all) $(OBJS) -rf



