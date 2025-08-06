DEBUG=0
FRONTEND_SUPPORTS_RGB565=1
FORCE_32BIT_ARCH=0
MMAP_JIT_CACHE=0

UNAME=$(shell uname -a)

# platform
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -s),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -s)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -s)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -s)),)
   platform = win
endif
endif

ifeq ($(platform), unix)
ifeq ($(firstword $(filter x86_64,$(UNAME))),x86_64)
	HAVE_DYNAREC := 1
	CPU_ARCH := x86_32
else ifeq ($(firstword $(filter amd64,$(UNAME))),amd64)
	HAVE_DYNAREC := 1
	CPU_ARCH := x86_32
else ifeq ($(firstword $(filter x86,$(UNAME))),x86)
	FORCE_32BIT_ARCH = 1
	HAVE_DYNAREC := 1
	CPU_ARCH := x86_32
endif
endif

FORCE_32BIT :=

ifeq ($(FORCE_32BIT_ARCH),1)
	FORCE_32BIT := -m32
endif

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
	EXE_EXT = .exe
	system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	system_platform = osx
	arch = intel
	ifeq ($(shell uname -p),powerpc)
		arch = ppc
	endif
	ifeq ($(shell uname -p),arm)
		arch = arm
	endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
	system_platform = win
endif

TARGET_NAME	:= gpsp
GIT_VERSION := "$(shell git rev-parse --short HEAD || echo unknown)"
ifneq ($(GIT_VERSION),"unknown")
	CFLAGS += -DGIT_VERSION=\"$(GIT_VERSION)\"
endif
LIBM		   := -lm
CORE_DIR    := .
LDFLAGS     :=

# Unix
ifeq ($(platform), unix)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
	SHARED := -shared $(FORCE_32BIT) -Wl,--version-script=link.T
	ifneq ($(findstring Haiku,$(shell uname -a)),)
		LIBM :=
	endif
	CFLAGS += $(FORCE_32BIT)
	LDFLAGS += -Wl,--no-undefined
	ifeq ($(HAVE_DYNAREC),1)
		MMAP_JIT_CACHE = 1
	endif

# Linux portable
else ifeq ($(platform), linux-portable)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC -nostdlib
	SHARED := -shared $(FORCE_32BIT) -Wl,--version-script=link.T
	LIBM :=
	CFLAGS += $(FORCE_32BIT)
	ifeq ($(HAVE_DYNAREC),1)
		MMAP_JIT_CACHE = 1
	endif

# OS X
else ifeq ($(platform), osx)
	TARGET := $(TARGET_NAME)_libretro.dylib
	fpic := -fPIC
	ifeq ($(arch),ppc)
		CFLAGS += -DMSB_FIRST -D__ppc__
	endif
	OSXVER = `sw_vers -productVersion | cut -d. -f 2`
	OSX_LT_MAVERICKS = `(( $(OSXVER) <= 9)) && echo "YES"`
	ifeq ($(OSX_LT_MAVERICKS),YES)
		fpic += -mmacosx-version-min=10.1
	endif
	SHARED := -dynamiclib
	ifeq ($(HAVE_DYNAREC),1)
		MMAP_JIT_CACHE = 1
	endif

	ifeq ($(CROSS_COMPILE),1)
		TARGET_RULE   = -target $(LIBRETRO_APPLE_PLATFORM) -isysroot $(LIBRETRO_APPLE_ISYSROOT)
		CFLAGS   += $(TARGET_RULE)
		LDFLAGS  += $(TARGET_RULE)
	endif

	CFLAGS  += $(ARCHFLAGS)
	LDFLAGS += $(ARCHFLAGS)

# iOS
else ifneq (,$(findstring ios,$(platform)))

	TARGET := $(TARGET_NAME)_libretro_ios.dylib
	fpic := -fPIC
	SHARED := -dynamiclib
	CPU_ARCH := arm

	ifeq ($(IOSSDK),)
		IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
	endif

	ifeq ($(platform),ios-arm64)
		CC = cc -arch arm64 -isysroot $(IOSSDK)
		CXX = c++ -arch arm64 -isysroot $(IOSSDK)
	else
		CC = cc -arch armv7 -isysroot $(IOSSDK)
		CXX = c++ -arch armv7 -isysroot $(IOSSDK)
	endif
	CFLAGS += -DIOS -DHAVE_POSIX_MEMALIGN -marm

	ifeq ($(platform),$(filter $(platform),ios9 ios-arm64))
		MINVERSION = -miphoneos-version-min=8.0
	else
		MINVERSION = -miphoneos-version-min=5.0
	endif
	SHARED += $(MINVERSION)
	CC += $(MINVERSION)
	CXX += $(MINVERSION)

# tvOS
else ifeq ($(platform), tvos-arm64)
	TARGET := $(TARGET_NAME)_libretro_tvos.dylib
	fpic := -fPIC
	SHARED := -dynamiclib
	CPU_ARCH := arm
	CFLAGS += -DIOS -DHAVE_POSIX_MEMALIGN -marm

	ifeq ($(IOSSDK),)
		IOSSDK := $(shell xcodebuild -version -sdk appletvos Path)
	endif
   CC = clang -arch arm64 -isysroot $(IOSSDK)
   CXX = clang++ -arch arm64 -isysroot $(IOSSDK)

# iOS Theos
else ifeq ($(platform), theos_ios)
	DEPLOYMENT_IOSVERSION = 5.0
	TARGET = iphone:latest:$(DEPLOYMENT_IOSVERSION)
	ARCHS = armv7 armv7s
	TARGET_IPHONEOS_DEPLOYMENT_VERSION=$(DEPLOYMENT_IOSVERSION)
	THEOS_BUILD_DIR := objs
	include $(THEOS)/makefiles/common.mk

	CFLAGS += -DIOS -DHAVE_POSIX_MEMALIGN -marm
	CPU_ARCH := arm
	LIBRARY_NAME = $(TARGET_NAME)_libretro_ios

# QNX
else ifeq ($(platform), qnx)
	TARGET := $(TARGET_NAME)_libretro_qnx.so
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T
	MMAP_JIT_CACHE = 1
	CPU_ARCH := arm

	CC = qcc -Vgcc_ntoarmv7le
	CXX = qcc -Vgcc_ntoarmv7le
	AR = qcc -Vgcc_ntoarmv7le
	CFLAGS += -D__BLACKBERRY_QNX_
	HAVE_DYNAREC := 1

# Lightweight PS3 Homebrew SDK
else ifeq ($(platform), psl1ght)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
	CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
	AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
	CFLAGS += -DMSB_FIRST -D__ppc__
	STATIC_LINKING = 1
	
# Nintendo Switch (libnx)
else ifeq ($(platform), libnx)
	include $(DEVKITPRO)/libnx/switch_rules
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CFLAGS += -O3 -fomit-frame-pointer -ffast-math -I$(DEVKITPRO)/libnx/include/ -fPIE
	CFLAGS += -specs=$(DEVKITPRO)/libnx/switch.specs
	CFLAGS += -D__SWITCH__ -DHAVE_LIBNX
	CFLAGS += -DARM -D__aarch64__=1 -march=armv8-a -mtune=cortex-a57 -mtp=soft -ffast-math
	STATIC_LINKING=1
	STATIC_LINKING_LINK = 1

# Nintendo Game Cube / Wii / WiiU
else ifneq (,$(filter $(platform), ngc wii wiiu))
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
	CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
	AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
	CFLAGS += -DGEKKO -mcpu=750 -meabi -mhard-float -DHAVE_STRTOF_L
        CFLAGS += -ffunction-sections -fdata-sections -D__wiiu__ -D__wut__
	STATIC_LINKING = 1

# PSP
else ifeq ($(platform), psp1)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = psp-gcc$(EXE_EXT)
	CXX = psp-g++$(EXE_EXT)
	AR = psp-ar$(EXE_EXT)
	CFLAGS += -DPSP -G0 -DMIPS_HAS_R2_INSTS -DSMALL_TRANSLATION_CACHE
	CFLAGS += -I$(shell psp-config --pspsdk-path)/include
	CFLAGS += -march=allegrex -mfp32 -mgp32 -mlong32 -mabi=eabi
	CFLAGS += -fomit-frame-pointer -ffast-math
	CFLAGS += -falign-functions=32 -falign-loops -falign-labels -falign-jumps
	STATIC_LINKING = 1
	HAVE_DYNAREC = 1
	CPU_ARCH := mips

# Vita
else ifeq ($(platform), vita)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = arm-vita-eabi-gcc$(EXE_EXT)
	CXX = arm-vita-eabi-g++$(EXE_EXT)
	AR = arm-vita-eabi-ar$(EXE_EXT)
	CFLAGS += -DVITA -DOVERCLOCK_60FPS
	CFLAGS += -marm -mcpu=cortex-a9 -mfloat-abi=hard
	CFLAGS += -Wall -mword-relocations
	CFLAGS += -fomit-frame-pointer -ffast-math
	CFLAGS += -mword-relocations -fno-unwind-tables -fno-asynchronous-unwind-tables 
	CFLAGS += -ftree-vectorize -fno-optimize-sibling-calls
	ASFLAGS += -mcpu=cortex-a9
	STATIC_LINKING = 1
	HAVE_DYNAREC = 1
	CPU_ARCH := arm

# CTR(3DS)
else ifeq ($(platform), ctr)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITARM)/bin/arm-none-eabi-gcc$(EXE_EXT)
	CXX = $(DEVKITARM)/bin/arm-none-eabi-g++$(EXE_EXT)
	AR = $(DEVKITARM)/bin/arm-none-eabi-ar$(EXE_EXT)
	CFLAGS += -DARM11 -D_3DS
	CFLAGS += -march=armv6k -mtune=mpcore -mfloat-abi=hard
	CFLAGS += -Wall -mword-relocations
	CFLAGS += -fomit-frame-pointer -ffast-math
	CPU_ARCH := arm
	HAVE_DYNAREC = 1
	STATIC_LINKING = 1

# Raspberry Pi 3
else ifeq ($(platform), rpi3)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
	CFLAGS += -marm -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard
	CFLAGS += -fomit-frame-pointer -ffast-math
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	HAVE_DYNAREC = 1

# Raspberry Pi 2
else ifeq ($(platform), rpi2)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
	CFLAGS += -marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
	CFLAGS += -fomit-frame-pointer -ffast-math
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	HAVE_DYNAREC = 1

# Raspberry Pi 1
else ifeq ($(platform), rpi1)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
	CFLAGS += -DARM11
	CFLAGS += -marm -mfpu=vfp -mfloat-abi=hard -march=armv6j
	CFLAGS += -fomit-frame-pointer -ffast-math
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	HAVE_DYNAREC = 1

# Classic Platforms ####################
# Platform affix = classic_<ISA>_<µARCH>
# Help at https://modmyclassic.com/comp

# (armv7 a7, hard point, neon based) ### 
# NESC, SNESC, C64 mini 
else ifeq ($(platform), classic_armv7_a7)
	TARGET := $(TARGET_NAME)_libretro.so
	fpic := -fPIC
    SHARED := -shared -Wl,--version-script=link.T  -Wl,--no-undefined -fPIC
	CFLAGS += -Ofast \
	-flto=4 -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector -fno-ident -fomit-frame-pointer \
	-falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	-fmerge-all-constants -fno-math-errno \
	-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
	ASFLAGS += $(CFLAGS)
	ARCH = arm
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	HAVE_DYNAREC = 1
	ifeq ($(shell echo `$(CC) -dumpversion` "< 4.9" | bc -l), 1)
	  CFLAGS += -march=armv7-a
	else
	  CFLAGS += -march=armv7ve
	  # If gcc is 5.0 or later
	  ifeq ($(shell echo `$(CC) -dumpversion` ">= 5" | bc -l), 1)
	    LDFLAGS += -static-libgcc -static-libstdc++
	  endif
	endif

#######################################

# Xbox 360
else ifeq ($(platform), xenon)
	TARGET := $(TARGET_NAME)_libretro_xenon360.a
	CC = xenon-gcc$(EXE_EXT)
	CXX = xenon-g++$(EXE_EXT)
	AR = xenon-ar$(EXE_EXT)
	CFLAGS += -D__LIBXENON__ -m32 -D__ppc__
	STATIC_LINKING = 1

# Nintendo Game Cube
else ifeq ($(platform), ngc)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
	CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
	AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
	CFLAGS += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST -D__ppc__
	STATIC_LINKING = 1

# Nintendo Wii
else ifeq ($(platform), wii)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
	CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
	AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
	CFLAGS += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST -D__ppc__
	STATIC_LINKING = 1

# aarch64 (armv8)
else ifeq ($(platform), arm64)
	TARGET := $(TARGET_NAME)_libretro.so
	SHARED := -shared -Wl,--version-script=link.T
	fpic := -fPIC
	CFLAGS += -fomit-frame-pointer -ffast-math
	LDFLAGS += -Wl,--no-undefined
	HAVE_DYNAREC := 1
	MMAP_JIT_CACHE = 1
	CPU_ARCH := arm64

# ARM
else ifneq (,$(findstring armv,$(platform)))
	TARGET := $(TARGET_NAME)_libretro.so
	SHARED := -shared -Wl,--version-script=link.T
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	fpic := -fPIC
	ifneq (,$(findstring cortexa5,$(platform)))
		CFLAGS += -marm -mcpu=cortex-a5
		ASFLAGS += -mcpu=cortex-a5
	else ifneq (,$(findstring cortexa8,$(platform)))
		CFLAGS += -marm -mcpu=cortex-a8
		ASFLAGS += -mcpu=cortex-a8
	else ifneq (,$(findstring cortexa9,$(platform)))
		CFLAGS += -marm -mcpu=cortex-a9
		ASFLAGS += -mcpu=cortex-a9
	else ifneq (,$(findstring cortexa15a7,$(platform)))
		CFLAGS += -marm -mcpu=cortex-a15.cortex-a7
		ASFLAGS += -mcpu=cortex-a15.cortex-a7
	else
		CFLAGS += -marm
	endif
	ifneq (,$(findstring softfloat,$(platform)))
		CFLAGS += -mfloat-abi=softfp
		ASFLAGS += -mfloat-abi=softfp
	else ifneq (,$(findstring hardfloat,$(platform)))
		CFLAGS += -mfloat-abi=hard
		ASFLAGS += -mfloat-abi=hard
	endif
	# Dynarec works at least in rpi, take a look at issue #11
	ifeq (,$(findstring no-dynarec,$(platform)))
		HAVE_DYNAREC := 1
	endif
	LDFLAGS += -Wl,--no-undefined

# MIPS
else ifeq ($(platform), mips32)
	TARGET := $(TARGET_NAME)_libretro.so
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=mips32 -mtune=mips32r2 -mhard-float
	CFLAGS += -DMIPS_HAS_R2_INSTS
	HAVE_DYNAREC := 1
	CPU_ARCH := mips

# MIPS64
else ifeq ($(platform), mips64n32)
	TARGET := $(TARGET_NAME)_libretro.so
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=mips64 -mabi=n32 -mhard-float
	HAVE_DYNAREC := 1
	CPU_ARCH := mips

# PS2
else ifeq ($(platform), ps2)
	TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = mips64r5900el-ps2-elf-gcc$(EXE_EXT)
	CXX = mips64r5900el-ps2-elf-g++$(EXE_EXT)
	AR = mips64r5900el-ps2-elf-ar$(EXE_EXT)
	CFLAGS += -fomit-frame-pointer -ffast-math
	CFLAGS += -DPS2 -DUSE_XBGR1555_FORMAT -DSMALL_TRANSLATION_CACHE -DROM_BUFFER_SIZE=16
	CFLAGS += -D_EE -I$(PS2SDK)/ee/include/ -I$(PS2SDK)/common/include/
	HAVE_DYNAREC = 1
	CPU_ARCH := mips
	STATIC_LINKING = 1

# emscripten
else ifeq ($(platform), emscripten)
	TARGET := $(TARGET_NAME)_libretro_$(platform).bc
	STATIC_LINKING = 1

# GCW0 (OD and OD Beta)
else ifeq ($(platform), gcw0)
	TARGET := $(TARGET_NAME)_libretro.so
	CC = /opt/gcw0-toolchain/usr/bin/mipsel-linux-gcc
	CXX = /opt/gcw0-toolchain/usr/bin/mipsel-linux-g++
	AR = /opt/gcw0-toolchain/usr/bin/mipsel-linux-ar
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=mips32 -mtune=mips32r2 -mhard-float
	CFLAGS += -DMIPS_HAS_R2_INSTS
	HAVE_DYNAREC := 1
	CPU_ARCH := mips

# RETROFW
else ifeq ($(platform), retrofw)
	TARGET := $(TARGET_NAME)_libretro.so
	CC = /opt/retrofw-toolchain/usr/bin/mipsel-linux-gcc
	CXX = /opt/retrofw-toolchain/usr/bin/mipsel-linux-g++
	AR = /opt/retrofw-toolchain/usr/bin/mipsel-linux-ar
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=mips32  -mhard-float
	HAVE_DYNAREC := 1
	CPU_ARCH := mips

# RS90
else ifeq ($(platform), rs90)
	TARGET := $(TARGET_NAME)_libretro.so
	CC = /opt/rs90-toolchain/usr/bin/mipsel-linux-gcc
	CXX = /opt/rs90-toolchain/usr/bin/mipsel-linux-g++
	AR = /opt/rs90-toolchain/usr/bin/mipsel-linux-ar
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=mips32 -mtune=mips32
	CFLAGS += -DSMALL_TRANSLATION_CACHE -DROM_BUFFER_SIZE=4
	HAVE_DYNAREC := 1
	CPU_ARCH := mips

# MIYOO
else ifeq ($(platform), miyoo)
	TARGET := $(TARGET_NAME)_libretro.so
	CC = /opt/miyoo/usr/bin/arm-linux-gcc
	CXX = /opt/miyoo/usr/bin/arm-linux-g++
	AR = /opt/miyoo/usr/bin/arm-linux-ar
	SHARED := -shared -nostdlib -Wl,--version-script=link.T
	fpic := -fPIC -DPIC
	CFLAGS += -fomit-frame-pointer -ffast-math -march=armv5te -mtune=arm926ej-s
	CFLAGS += -DSMALL_TRANSLATION_CACHE
	HAVE_DYNAREC := 1
	CPU_ARCH := arm
	

else ifeq ($(platform), miyoomini)
	TARGET := $(TARGET_NAME)_plus_libretro.so
	CC = /opt/miyoomini-toolchain/usr/bin/arm-linux-gcc
	CXX = /opt/miyoomini-toolchain/usr/bin/arm-linux-g++
	AR = /opt/miyoomini-toolchain/usr/bin/arm-linux-ar
	fpic := -fPIC
	SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
	CFLAGS += -Ofast \
	-flto=4 -fwhole-program -fuse-linker-plugin \
	-fdata-sections -ffunction-sections -Wl,--gc-sections \
	-fno-stack-protector -fno-ident -fomit-frame-pointer \
	-falign-functions=1 -falign-jumps=1 -falign-loops=1 \
	-fno-unwind-tables -fno-asynchronous-unwind-tables -fno-unroll-loops \
	-fmerge-all-constants -fno-math-errno \
	-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
	CXXFLAGS = $(CFLAGS) -std=gnu++11
	CPPFLAGS += $(CFLAGS)
	ASFLAGS += $(CFLAGS)
	CPU_ARCH := arm
	MMAP_JIT_CACHE = 1
	HAVE_DYNAREC = 1
	HAVE_NEON = 1
	ARCH = arm
	BUILTIN_GPU = neon

# Windows
else
	TARGET := $(TARGET_NAME)_libretro.dll
	CC ?= gcc
	CXX ?= g++
	SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=link.T
	CFLAGS += -D__WIN32__ -D__WIN32_LIBRETRO__

	# Windows32/64 has dynarec support
	HAVE_DYNAREC := 1
	CPU_ARCH := x86_32
	MMAP_JIT_CACHE = 1
endif

ifeq ($(MMAP_JIT_CACHE), 1)
CFLAGS += -DMMAP_JIT_CACHE
endif

# Add -DTRACE_EVENTS to trace relevant events (IRQs, SMC, etc)
# Add -DTRACE_INSTRUCTIONS to trace instruction execution
# Can add -DTRACE_REGISTERS to additionally print register values
ifeq ($(DEBUG), 1)
	OPTIMIZE      := -O0 -g
else
	OPTIMIZE      := -O3 -DNDEBUG
endif

DEFINES := -DHAVE_STRINGS_H -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DINLINE=inline -Wall

ifeq ($(HAVE_DYNAREC), 1)
DEFINES += -DHAVE_DYNAREC
endif

ifeq ($(CPU_ARCH), arm)
	DEFINES += -DARM_ARCH
else ifeq ($(CPU_ARCH), arm64)
	DEFINES += -DARM64_ARCH
else ifeq ($(CPU_ARCH), mips)
	DEFINES += -DMIPS_ARCH
else ifeq ($(CPU_ARCH), x86_32)
	DEFINES += -DX86_ARCH
endif

include Makefile.common

OBJECTS := $(SOURCES_C:.c=.o) $(SOURCES_ASM:.S=.o) $(SOURCES_CC:.cc=.o)

WARNINGS_DEFINES =
CODE_DEFINES =

COMMON_DEFINES += $(CODE_DEFINES) $(WARNINGS_DEFINES) -DNDEBUG=1 $(fpic)

CFLAGS += $(DEFINES) $(COMMON_DEFINES)

ifeq ($(FRONTEND_SUPPORTS_RGB565), 1)
	CFLAGS += -DFRONTEND_SUPPORTS_RGB565
endif


ifeq ($(platform), ctr)
ifeq ($(HAVE_DYNAREC), 1)
OBJECTS += 3ds/3ds_utils.o 3ds/3ds_cache_utils.o

ifeq ($(strip $(CTRULIB)),)
$(error "Please set CTRULIB in your environment. export CTRULIB=<path to>libctru")
endif

CFLAGS  += -I$(CTRULIB)/include

endif
endif

CXXFLAGS = $(CFLAGS) -fno-rtti -fno-exceptions -std=c++11

ifeq ($(platform), theos_ios)
COMMON_FLAGS := -DIOS $(COMMON_DEFINES) $(INCFLAGS) -I$(THEOS_INCLUDE_PATH) -Wno-error
$(LIBRARY_NAME)_CFLAGS += $(COMMON_FLAGS) $(CFLAGS)
${LIBRARY_NAME}_FILES = $(SOURCES_C) $(SOURCES_ASM) $(SOURCES_CC)
include $(THEOS_MAKE_PATH)/library.mk
else
all: $(TARGET)

# Linking with gcc on purpose, we do not use any libstdc++ dependencies at all, only libc is required.

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CC) $(fpic) $(SHARED) $(INCFLAGS) $(OPTIMIZE) -o $@ $(OBJECTS) $(LIBM) $(LDFLAGS)
endif

cpu_threaded.o: cpu_threaded.c
	$(CC) $(INCFLAGS) $(CFLAGS) $(OPTIMIZE) -Wno-unused-variable -Wno-unused-label -c  -o $@ $<

%.o: %.S
	$(CC) $(ASFLAGS) $(CFLAGS) $(OPTIMIZE) -c -o $@ $<

%.o: %.c
	$(CC) $(INCFLAGS) $(CFLAGS) $(OPTIMIZE) -c  -o $@ $<

%.o: %.cc
	$(CXX) $(INCFLAGS) $(CXXFLAGS) $(OPTIMIZE) -c  -o $@ $<

clean-objs:
	rm -rf $(OBJECTS)

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
endif
