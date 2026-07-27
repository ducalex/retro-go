set(COMPONENT_SRCS
    "fx_man.c"
    "dsl.c"
    "ll_man.c"
    "multivoc.c"
    "mv_mix.c"
    "mvreverb.c"
    "nodpmi.c"
    "pitch.c"
    "user.c"
    "usrhooks.c"
    "gmtimbre.c"
    "opl_music.c"
    "midifile.c"
    "opl.c"
    "dbopl.c"
    "oplplayer.c"
)
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "duke3d")
register_component()

# The mixer runs for every active voice on the real-time audio task. In
# particular, keep its interpolation and fixed-point accumulation speed
# optimized rather than inheriting the firmware's size-oriented default.
set_source_files_properties(
    multivoc.c mv_mix.c mvreverb.c
    opl.c dbopl.c oplplayer.c
    PROPERTIES COMPILE_FLAGS
    -O2
)

rg_setup_compile_options(-Wno-error=pointer-sign -Wno-error=maybe-uninitialized -DHAVE_CONFIG_H)
