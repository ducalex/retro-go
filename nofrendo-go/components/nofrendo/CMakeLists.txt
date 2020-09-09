set(COMPONENT_SRCDIRS "cpu nes mappers .")
set(COMPONENT_ADD_INCLUDEDIRS "cpu nes mappers .")
set(COMPONENT_REQUIRES "odroid")
register_component()
component_compile_options(-O3 -Wno-error=char-subscripts -Wno-error=attributes)

if(DEFINED ENABLE_PROFILING)
component_compile_options(-finstrument-functions)
endif()
