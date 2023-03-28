set(COMPONENT_SRCDIRS ". libs")
set(COMPONENT_ADD_INCLUDEDIRS ". libs")
register_component()
rg_setup_compile_options(-O2 -Wno-error=format -Wno-error=char-subscripts -mfix-esp32-psram-cache-issue)
