set(COMPONENT_SRCDIRS "cpu nes mappers .")
set(COMPONENT_ADD_INCLUDEDIRS "cpu nes mappers .")
set(COMPONENT_REQUIRES "odroid")
register_component()

component_compile_options(-O3 -Wno-error=char-subscripts -Wno-error=attributes)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING -finstrument-functions)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()
