set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ". libretro/libretro-common/include")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -O3
    -fomit-frame-pointer -ffast-math
    -DOVERCLOCK_60FPS
    -DROM_BUFFER_SIZE=8
    -Wno-unused-variable
    -Wno-implicit-fallthrough
    -Wno-missing-field-initializers
    -Wno-error=unused-label
    -Wno-unused-label
)
