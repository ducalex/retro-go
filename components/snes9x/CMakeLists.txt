set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -Wno-unused-function
    -Wno-unused-variable
    -Wno-array-bounds
    -Wno-implicit-fallthrough
    -DRIGHTSHIFT_IS_SAR
    -DFAST_LSB_WORD_ACCESS
    -DNO_ZERO_LUT
    -O2
)
