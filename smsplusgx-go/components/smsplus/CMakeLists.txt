set(COMPONENT_SRCDIRS ". cpu sound")
set(COMPONENT_ADD_INCLUDEDIRS ". cpu sound")
set(COMPONENT_REQUIRES "odroid")
register_component()
component_compile_options(-O3 -DIS_LITTLE_ENDIAN)

if(DEFINED ENABLE_PROFILING)
component_compile_options(-finstrument-functions)
endif()
