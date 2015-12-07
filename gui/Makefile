#
# Copyright(C)2014-2015 Shaomin (Samuel) Zhang <smicn@foxmail.com>
#

COMPILE = 
CC     	= $(COMPILE)g++
AR     	= $(COMPILE)ar
LD     	= $(COMPILE)ld
OBJCOPY	= $(COMPILE)objcopy
NM      = $(COMPILE)nm
OBJDUMP	= $(COMPILE)objdump
CHMOD  	= chmod
STRIP   = $(COMPILE)strip
PWD     = $(shell pwd)/..

CFLAGS += -DHAVE_CONFIG_H -DLINUX
CFLAGS += -g -O2 -fpermissive
CFLAGS += -I./inc

#LDFLAGS += -Wl,-rpath,./lib,-rpath
LDFLAGS += -L./lib -lftk -lfreetype -lpng -ljpeg -lz
LDFLAGS += -lX11 -lrt -lpthread -ldl

GUI_SRCS := ftkui_receiver.c \
		ftkui_localcar.c \
		ftkui_controller.c \
		ftkui_main.c

NONUI_SRCS := ftkui_receiver.c \
		ftkui_localcar.c \
		ftkui_controller.c \
		nonui_main.c
		
all : $(OBJS)
	$(CC) $(CFLAGS) $(NONUI_SRCS) -o test $(LDFLAGS)
	$(CC) $(CFLAGS) -DLOCAL_SIMULATOR $(GUI_SRCS) -o simulator $(LDFLAGS)
	$(CC) $(CFLAGS) $(GUI_SRCS) -o observer $(LDFLAGS)

clean:
	rm -rf *.o *.out

