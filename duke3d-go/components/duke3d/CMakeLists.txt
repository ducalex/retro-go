set(COMPONENT_SRCDIRS "Game SDL Engine")
set(COMPONENT_ADD_INCLUDEDIRS ". Game SDL Engine ../audiolib")
set(COMPONENT_REQUIRES "retro-go")
register_component()

# Keep most of the legacy port size-optimized, but optimize the renderer,
# cache/asset path, actor/script interpreter, player, and game-loop hot paths
# for speed.
# The pixel kernels in draw.c already request O3 locally.
set_source_files_properties(
    Engine/cache.c Engine/engine.c Engine/filesystem.c
    Engine/fixedPoint_math.c Engine/tiles.c
    Game/actors.c Game/gamedef.c Game/game.c Game/player.c
    Game/sector.c Game/sounds.c
    PROPERTIES COMPILE_FLAGS
    -O2
)

rg_setup_compile_options(
    -Wno-error=unused-label
    -Wno-int-conversion
    -Wno-error=pointer-sign
    -Wno-error=misleading-indentation
    -Wno-error=format-overflow=
    -Wno-error=stringop-truncation
    -Wno-error=uninitialized
    -DLOG_LOCAL_LEVEL=ESP_LOG_ERROR
    -Wno-error=maybe-uninitialized
    -Wno-error=array-bounds
    -Wno-error=stringop-overflow=
    -Wno-error=char-subscripts
    -Wno-error=tautological-compare
    -Wno-int-conversion
    -Wno-error=pointer-sign
    -Wno-error=misleading-indentation
    -Wno-error=format-overflow=
    -Wno-error=stringop-truncation
    -Wno-error=format-truncation
    -Wno-implicit-fallthrough
    -Os
)
