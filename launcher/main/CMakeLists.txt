set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
rg_setup_compile_options(-Wno-error=format-truncation -DRG_REPLACE_PRINTF_FUNCTIONS)
