set(COMPONENT_SRCDIRS ". apu apu/bapu/dsp apu/bapu/smp")
set(COMPONENT_ADD_INCLUDEDIRS ". apu apu/bapu/dsp apu/bapu/smp")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -Ofast
    -fno-rtti
    -fno-exceptions
    -fno-math-errno
    -DRIGHTSHIFT_IS_SAR
    -DHAVE_STDINT_H
    -DIS_LITTLE_ENDIAN
    -Wno-error=format-truncation
)
