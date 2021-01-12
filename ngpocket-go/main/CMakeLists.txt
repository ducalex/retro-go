set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
rg_setup_compile_options(
    -fno-rtti
    -fno-exceptions
    -Wno-error=format-truncation
    -DTARGET_OD
)
