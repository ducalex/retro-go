AUDIOLIB ?= audiolib.a

AR ?= ar
CC ?= gcc
RANLIB ?= ranlib

ARFLAGS ?= rcT
CFLAGS ?= -g -O2
CFLAGS += -Wall
CFLAGS += $(shell sdl-config --cflags)

OBJS :=
OBJS += fx_man.o
OBJS += dsl.o
OBJS += ll_man.o
OBJS += multivoc.o
OBJS += mv_mix.o
OBJS += mvreverb.o
OBJS += nodpmi.o
OBJS += pitch.o
OBJS += user.o
OBJS += usrhooks.o

$(AUDIOLIB): $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(AUDIOLIB) $(OBJS)
