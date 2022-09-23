set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -Wno-error=format-truncation
    -Wno-error=stringop-overflow
    -Wno-missing-field-initializers
    -Wno-unused-function
    -Wno-error=array-bounds
    -Wno-error=restrict
    -Wno-array-bounds
    -O2
    -DFAST_LSB_WORD_ACCESS
    -DNO_ZERO_LUT
)
