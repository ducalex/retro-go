set(ROTT_SOURCES
    cin_actr.c
    cin_efct.c
    cin_evnt.c
    cin_glob.c
    cin_main.c
    cin_util.c
    dosutil.c
    engine.c
    isr.c
    modexlib.c
    rt_actor.c
    rt_battl.c
    rt_build.c
    rt_cfg.c
    rt_crc.c
    rt_com.c
    rt_debug.c
    rt_dmand.c
    rt_door.c
    rt_draw.c
    rt_floor.c
    rt_game.c
    rt_in.c
    rt_main.c
    rt_map.c
    rt_menu.c
    rt_msg.c
    rt_net.c
    rt_playr.c
    rt_rand.c
    rt_scale.c
    rt_sound.c
    rt_spbal.c
    rt_sqrt.c
    rt_stat.c
    rt_state.c
    rt_str.c
    rt_swift.c
    rt_ted.c
    rt_util.c
    rt_view.c
    rt_vid.c
    rt_err.c
    rt_error.c
    scriplib.c
    w_wad.c
    watcom.c
    byteordr.c
    winrott.c
    rg_sdl.c
    rg_memory.c
)

set(AUDIOLIB_SOURCES
    audiolib/fx_man.c
    audiolib/dsl.c
    audiolib/ll_man.c
    audiolib/multivoc.c
    audiolib/mv_mix.c
    audiolib/mvreverb.c
    audiolib/nodpmi.c
    audiolib/pitch.c
    audiolib/user.c
    audiolib/usrhooks.c
    audiolib/task_man.c
    audiolib/dbopl.c
    audiolib/opl.c
    audiolib/oplplayer.c
    audiolib/opl_queue.c
    audiolib/midifile.c
    audiolib/opl_music.c
    audiolib/gmtimbre.c
)

set(COMPONENT_SRCS ${ROTT_SOURCES} ${AUDIOLIB_SOURCES})
set(COMPONENT_ADD_INCLUDEDIRS "." "audiolib")
set(COMPONENT_REQUIRES "retro-go")

register_component()

target_compile_options(${COMPONENT_LIB} PRIVATE
    -Wno-address
    -Wno-misleading-indentation
    -Wno-format-overflow
    -Wno-char-subscripts
    -Wno-missing-field-initializers
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-pointer-sign
    -Wno-maybe-uninitialized
    -Wno-implicit-fallthrough
    -Wno-parentheses
    -Wno-shift-negative-value
    -Wno-enum-compare
    -Wno-int-conversion
    -Wno-cast-function-type
    -Wno-array-bounds
    -Wno-aggressive-loop-optimizations
    -Wno-unknown-pragmas
    -Wno-incompatible-pointer-types
    -Wno-multichar
    -Wno-return-type
    -Wno-unused-value
    -Wno-nonnull
    -Wno-stringop-overflow
    -Wno-format-truncation
)

# Move large structures to PSRAM using linker attributes
# We can use -fdata-sections and a linker script, but applying it per-file is easier for Retro-Go.

rg_setup_compile_options(
    -O2
    -DRETRO_GO=1
    -DUSE_SDL=1
    -DPLATFORM_UNIX=1
    -DNDEBUG
)
