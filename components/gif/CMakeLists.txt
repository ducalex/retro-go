set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
component_compile_options(-Os -Wno-strict-aliasing -Wno-maybe-uninitialized)
