# Define compilation type
OSTYPE=msys
#OSTYPE=oda320
#OSTYPE=odgcw

PRGNAME     = race-od

# define regarding OS, which compiler to use
ifeq "$(OSTYPE)" "msys"	
EXESUFFIX = .exe
TOOLCHAIN = /c/MinGW32
CC          = gcc
CCP         = g++
LD          = g++
else
ifeq "$(OSTYPE)" "oda320"	
TOOLCHAIN = /opt/opendingux-toolchain/usr
else
TOOLCHAIN = /opt/gcw0-toolchain/usr
endif
EXESUFFIX = .dge
CC  = $(TOOLCHAIN)/bin/mipsel-linux-gcc
CCP = $(TOOLCHAIN)/bin/mipsel-linux-g++
LD  = $(TOOLCHAIN)/bin/mipsel-linux-g++
endif

# add SDL dependencies
SDL_LIB     = $(TOOLCHAIN)/lib
SDL_INCLUDE = $(TOOLCHAIN)/include

# change compilation / linking flag options
ifeq "$(OSTYPE)" "msys"	
F_OPTS = -fpermissive -fno-exceptions -fno-rtti
CC_OPTS = -O2 -g $(F_OPTS)
CFLAGS = -I$(SDL_INCLUDE) -DZ80 -DTARGET_OD -D_MAX_PATH=2048 -DHOST_FPS=60 -DNOUNCRYPT $(CC_OPTS)
CXXFLAGS=$(CFLAGS) 
LDFLAGS     = -L$(SDL_LIB) -lmingw32 -lSDLmain -lSDL -lz -mwindows
else
F_OPTS = -falign-functions -falign-loops -falign-labels -falign-jumps \
	-ffast-math -fsingle-precision-constant -funsafe-math-optimizations \
	-fomit-frame-pointer -fno-builtin -fno-common \
	-fstrict-aliasing  -fexpensive-optimizations \
	-finline -finline-functions -fpeel-loops -fno-exceptions -fno-rtti -fpermissive
ifeq "$(OSTYPE)" "oda320"
CC_OPTS		= -O2 -mips32 -msoft-float -G0 -DNOUNCRYPT  $(F_OPTS)
else
CC_OPTS		= -O2 -mips32 -mhard-float -G0 -DNOUNCRYPT  $(F_OPTS)
endif
CFLAGS		= -I$(SDL_INCLUDE) -D_OPENDINGUX_ -DZ80 -DTARGET_OD -D_MAX_PATH=2048 -DHOST_FPS=60 $(CC_OPTS)
CXXFLAGS	= $(CFLAGS) 
LDFLAGS		= -L$(SDL_LIB) $(CC_OPTS) -lSDL -lz
endif

# Files to be compiled
SRCDIR    = ./emu ./opendingux .
VPATH     = $(SRCDIR)
SRC_C   = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
SRC_CP   = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.cpp))
OBJ_C   = $(notdir $(patsubst %.c, %.o, $(SRC_C)))
OBJ_CP   = $(notdir $(patsubst %.cpp, %.o, $(SRC_CP)))
OBJS     = $(OBJ_C) $(OBJ_CP)

# Rules to make executable
$(PRGNAME)$(EXESUFFIX): $(OBJS)  
ifeq "$(OSTYPE)" "msys"	
	$(LD) $(CFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^ $(LDFLAGS)
else
	$(LD) $(LDFLAGS) -o $(PRGNAME)$(EXESUFFIX) $^
endif

$(OBJ_C) : %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ_CP) : %.o : %.cpp
	$(CCP) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(PRGNAME)$(EXESUFFIX) *.o
