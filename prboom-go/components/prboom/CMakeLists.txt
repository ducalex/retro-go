set(COMPONENT_SRCDIRS ". tables")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -Wno-error=maybe-uninitialized
    -Wno-error=parentheses
    -Wno-error=unused-value
    -Wno-pointer-sign
    -Wno-implicit-fallthrough
    -DHAVE_CONFIG_H
    -O2
)
