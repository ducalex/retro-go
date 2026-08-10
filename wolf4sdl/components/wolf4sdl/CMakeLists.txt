set(SDL_SRCS
    "SDL/SDL_audio.c"
    "SDL/SDL_blit.c"
    "SDL/SDL_error.c"
    "SDL/SDL_event.c"
    "SDL/SDL_input.c"
    "SDL/SDL_log.c"
    "SDL/SDL_rwops.c"
    "SDL/SDL_stdlib.c"
    "SDL/SDL_system.c"
    "SDL/SDL_video.c"
    "SDL/SDL_wave.c"
)

set(SDL_MIXER_SRCS
    "SDL_mixer-1.2.12/effects_internal.c"
    "SDL_mixer-1.2.12/effect_position.c"
    "SDL_mixer-1.2.12/load_aiff.c"
    "SDL_mixer-1.2.12/load_voc.c"
    "SDL_mixer-1.2.12/mixer.c"
    "SDL_mixer-1.2.12/music.c"
    "SDL_mixer-1.2.12/wavestream.c"
)

set(WOLF_SRCS
    "wl_main/fmopl.cpp"
    "wl_main/id_ca.cpp"
    "wl_main/id_in.cpp"
    "wl_main/id_pm.cpp"
    "wl_main/id_sd.cpp"
    "wl_main/id_us_1.cpp"
    "wl_main/id_vh.cpp"
    "wl_main/id_vl.cpp"
    "wl_main/signon.cpp"
    "wl_main/wl_act1.cpp"
    "wl_main/wl_act2.cpp"
    "wl_main/wl_agent.cpp"
    "wl_main/wl_atmos.cpp"
    "wl_main/wl_cloudsky.cpp"
    "wl_main/wl_debug.cpp"
    "wl_main/wl_dir3dspr.cpp"
    "wl_main/wl_draw.cpp"
    "wl_main/wl_floorceiling.cpp"
    "wl_main/wl_game.cpp"
    "wl_main/wl_inter.cpp"
    "wl_main/wl_main.cpp"
    "wl_main/wl_menu.cpp"
    "wl_main/wl_parallax.cpp"
    "wl_main/wl_play.cpp"
    "wl_main/wl_shade.cpp"
    "wl_main/wl_state.cpp"
    "wl_main/wl_text.cpp"
)

set(COMPONENT_SRCS ${SDL_SRCS} ${SDL_MIXER_SRCS} ${WOLF_SRCS})
set(COMPONENT_ADD_INCLUDEDIRS "SDL" "SDL_mixer-1.2.12" "wl_main")
set(COMPONENT_REQUIRES "retro-go")

register_component()
rg_setup_compile_options(-O2)

target_compile_definitions(${COMPONENT_LIB} PRIVATE 
    PLATFORM_ESP32
)

target_compile_options(${COMPONENT_LIB} PRIVATE 
    -Wno-maybe-uninitialized 
    -Wno-unused-variable
    -Wno-unused-but-set-variable
    -Wno-unused-function
    -Wno-unused-label
    -Wno-implicit-fallthrough
    -Wno-char-subscripts
    -Wno-misleading-indentation
    -Wno-error=parentheses 
    -Wno-error=switch 
    -Wno-error=unknown-pragmas 
    -Wno-error=narrowing 
    -Wno-error=format
    -Wno-error=format-overflow
    -Wno-error=restrict
)
