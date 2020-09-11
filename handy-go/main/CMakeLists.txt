set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()

component_compile_options(-Wall -Wno-comment -Wno-error=comment)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING -finstrument-functions)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()
