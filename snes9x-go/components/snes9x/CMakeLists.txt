set(COMPONENT_SRCDIRS ". apu")
set(COMPONENT_ADD_INCLUDEDIRS ". apu")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -fno-rtti
    -fno-exceptions
    -fno-math-errno
    -fomit-frame-pointer
    -fno-stack-protector
    -DRIGHTSHIFT_IS_SAR
    -DHAVE_STDINT_H
    -DIS_LITTLE_ENDIAN
    -Wno-error=format-truncation
)
