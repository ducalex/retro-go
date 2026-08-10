
CROSS_COMPILE = /mythtv/media/devel/toolchains/open2x/gcc-4.1.1-glibc-2.3.6/bin/arm-open2x-linux-
LDFLAGS = -static

CXX = $(CROSS_COMPILE)gcc
CPP = $(CROSS_COMPILE)g++
LD = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip

CXXFLAGS = -I/opt/open2x/gcc-4.1.1-glibc-2.3.6/include -Wall -Werror -Os -fomit-frame-pointer
LIBS = -L/opt/open2x/gcc-4.1.1-glibc-2.3.6/lib

CODE940_TARGET = code940.gpe
CODE940_OBJS = main.o fmopl_core.o

all : $(CODE940_TARGET)

$(CODE940_TARGET) : $(CODE940_OBJS)
	$(LD) -marmelf -e code940 -Ttext 0x0 $(CODE940_OBJS)  -o $(CODE940_TARGET)
	$(CROSS_COMPILE)objcopy -O binary code940.gpe code940.bin
	ls -l code940.bin

main.o: main.c
	$(CXX) $(CXXFLAGS) -O0 -c main.c

fmopl_core.o: fmopl_core.c fmopl_core.h fmopl_shared.h memory_layout.h
	$(CPP) $(CXXFLAGS) -Os -c fmopl_core.c
