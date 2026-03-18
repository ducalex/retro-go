set(fceu_srcs
    "src/cheat.c"
    "src/fceu-cart.c"
    "src/fceu-crc32.c"
    "src/fceu-debug.c"
    "src/fceu-endian.c"
    "src/fceu-memory.c"
    "src/fceu-sound.c"
    "src/fceu-state.c"
    "src/fceu.c"
    "src/fds.c"
    "src/fds_apu.c"
    "src/file.c"
    "src/filter.c"
    "src/general.c"
    "src/ines.c"
    "src/input.c"
    "src/md5.c"
    "src/nsf.c"
    "src/palette.c"
    "src/ppu.c"
    "src/unif.c"
    "src/video.c"
    "src/vsuni.c"
    "src/x6502.c"
)

# Boards
file(GLOB boards_srcs "src/boards/*.c")

# Input
file(GLOB input_srcs "src/input/*.c")

# Libretro Common Utilities
set(libretro_common_srcs
    "src/drivers/libretro/libretro-common/file/file_path.c"
    "src/drivers/libretro/libretro-common/file/file_path_io.c"
    "src/drivers/libretro/libretro-common/streams/file_stream.c"
    "src/drivers/libretro/libretro-common/streams/file_stream_transforms.c"
    "src/drivers/libretro/libretro-common/streams/memory_stream.c"
    "src/drivers/libretro/libretro-common/string/stdstring.c"
    "src/drivers/libretro/libretro-common/compat/compat_strl.c"
    "src/drivers/libretro/libretro-common/vfs/vfs_implementation.c"
)

set(COMPONENT_SRCS ${fceu_srcs} ${boards_srcs} ${input_srcs} ${libretro_common_srcs})
set(COMPONENT_ADD_INCLUDEDIRS
    "src"
    "src/boards"
    "src/input"
    "src/drivers/libretro/libretro-common/include"
    "."
)
set(COMPONENT_PRIV_REQUIRES
        retro-go
)

register_component()

rg_setup_compile_options(
    -DTARGET_GNW
    -D__LIBRETRO__
    -DFRONTEND_SUPPORTS_RGB565
    -DPATH_MAX=1024
    -DFCEU_VERSION_NUMERIC=9813
    -DIS_LITTLE_ENDIAN
    -DFCEU_NO_MALLOC
    -DENABLE_EMULATOR_NES
    -DFORCE_NOFRENDO=0
    -DNES_MAPPER_NSF
    -DSD_CARD=1
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-format
    -Wno-maybe-uninitialized
    -Wno-misleading-indentation
    -Wno-strict-aliasing
    -Wno-parentheses
    -Wno-implicit-fallthrough
    -Wno-unused-but-set-variable
    -Wno-unused-variable
    -O3
    -ffast-math
    -fno-exceptions
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
    -funroll-loops
    -fomit-frame-pointer
    -fno-strict-aliasing
)
