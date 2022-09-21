set(COMPONENT_SRCDIRS "Game SDL Engine")
set(COMPONENT_ADD_INCLUDEDIRS ". Game SDL Engine ../audiolib")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
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
    -Os
)
