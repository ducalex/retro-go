set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "odroid")
register_component()
component_compile_options(-O3 -Wall -Wno-comment -Wno-error=comment)

if(DEFINED ENABLE_PROFILING)
component_compile_options(-finstrument-functions)
endif()
