include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")

macro(rg_setup_compile_options)
    set(RG_TARGET "RG_TARGET_${RG_BUILD_TARGET}")
    message("Target: ${RG_TARGET}")

    component_compile_options(
        -D${RG_TARGET}
        -DRETRO_GO
        ${ARGV}
    )

    if(NOT ";${ARGV};" MATCHES ";-O[0123gs];")
        # Only default to -O3 if not specified by the app
        component_compile_options(-O3)
    endif()

    if(RG_ENABLE_NETPLAY)
        component_compile_options(-DRG_ENABLE_NETWORKING -DRG_ENABLE_NETPLAY)
    elseif(RG_ENABLE_NETWORKING)
        component_compile_options(-DRG_ENABLE_NETWORKING)
    endif()

    if(RG_ENABLE_PROFILING)
        # Still debating whether -fno-inline is necessary or not...
        component_compile_options(-DRG_ENABLE_PROFILING -finstrument-functions)
    endif()
endmacro()
