set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -fno-rtti
    -fno-exceptions
    -fno-math-errno
    -fomit-frame-pointer
    -fno-stack-protector
    -Wno-error=format-truncation
    -DTARGET_OD
)
