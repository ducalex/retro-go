set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "odroid")
register_component()

component_compile_options(-O3 -DIS_LITTLE_ENDIAN)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING -finstrument-functions)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()
