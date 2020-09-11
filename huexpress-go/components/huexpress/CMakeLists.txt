set(COMPONENT_SRCDIRS ". engine odroid")
set(COMPONENT_ADD_INCLUDEDIRS ". includes engine netplay")
set(COMPONENT_REQUIRES "odroid")
register_component()

component_compile_options(-O3 -Wno-all -Wno-error -Wno-sequence-point)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING -finstrument-functions)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()
