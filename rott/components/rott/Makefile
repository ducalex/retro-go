# Determine which version to build
# Make sure only one of the following is set to 1 at once
# Triple 0 will build the commercial/registered version

SHAREWARE   ?= 1
SUPERROTT   ?= 0
SITELICENSE ?= 0

CPPFLAGS += -DSHAREWARE=$(SHAREWARE)
CPPFLAGS += -DSUPERROTT=$(SUPERROTT)
CPPFLAGS += -DSITELICENSE=$(SITELICENSE)

ROTT ?= rott

# Regular build flags and rules

CC ?= gcc

CFLAGS ?= -g -O2
CFLAGS += -Wall -Wno-unused
CFLAGS += $(shell sdl-config --cflags)
CFLAGS += $(EXTRACFLAGS)

CPPFLAGS += -DUSE_SDL=1
CPPFLAGS += -DPLATFORM_UNIX=1
CPPFLAGS += $(EXTRACPPFLAGS)

LDFLAGS += $(EXTRALDFLAGS)

LDLIBS += $(shell sdl-config --libs)
LDLIBS += -lSDL_mixer
LDLIBS += $(EXTRALDLIBS)

OBJS :=
OBJS += cin_actr.o
OBJS += cin_efct.o
OBJS += cin_evnt.o
OBJS += cin_glob.o
OBJS += cin_main.o
OBJS += cin_util.o
OBJS += dosutil.o
OBJS += engine.o
OBJS += isr.o
OBJS += modexlib.o
OBJS += rt_actor.o
OBJS += rt_battl.o
OBJS += rt_build.o
OBJS += rt_cfg.o
OBJS += rt_crc.o
OBJS += rt_com.o
OBJS += rt_debug.o
OBJS += rt_dmand.o
OBJS += rt_door.o
OBJS += rt_draw.o
OBJS += rt_floor.o
OBJS += rt_game.o
OBJS += rt_in.o
OBJS += rt_main.o
OBJS += rt_map.o
OBJS += rt_menu.o
OBJS += rt_msg.o
OBJS += rt_net.o
OBJS += rt_playr.o
OBJS += rt_rand.o
OBJS += rt_scale.o
OBJS += rt_sound.o
OBJS += rt_spbal.o
OBJS += rt_sqrt.o
OBJS += rt_stat.o
OBJS += rt_state.o
OBJS += rt_str.o
OBJS += rt_swift.o
OBJS += rt_ted.o
OBJS += rt_util.o
OBJS += rt_view.o
OBJS += rt_vid.o
OBJS += rt_err.o
OBJS += scriplib.o
OBJS += w_wad.o
OBJS += watcom.o
OBJS += z_zone.o
OBJS += byteordr.o
OBJS += dukemusc.o
OBJS += winrott.o

AUDIOLIB := audiolib/audiolib.a

all: $(ROTT)

$(ROTT): $(OBJS) $(AUDIOLIB)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJS): develop.h

$(AUDIOLIB):
	$(MAKE) -C audiolib

tidy: 
	$(RM) $(OBJS) $(ROTT) $(ROTT).exe

clean: tidy
	$(MAKE) -C audiolib $@
