include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")
set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/base.sdkconfig")

macro(rg_setup_compile_options)
    set(RG_TARGET "RG_TARGET_$ENV{RG_TARGET}")
    message("Target: ${RG_TARGET}")

    component_compile_options(-DRETRO_GO -D${RG_TARGET} ${ARGV})

    if(NOT ";${ARGV};" MATCHES ";-O[0123gs];")
        # Only default to -O3 if not specified by the app
        component_compile_options(-O3)
    endif()

    if($ENV{ENABLE_PROFILING})
        # Still debating whether -fno-inline is necessary or not...
        component_compile_options(-DENABLE_PROFILING -finstrument-functions)
    endif()

    if($ENV{ENABLE_NETPLAY})
        component_compile_options(-DENABLE_NETPLAY)
    endif()
endmacro()
