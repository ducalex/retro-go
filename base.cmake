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

    if(IDF_VERSION_MAJOR EQUAL 5)
        if(RG_ENABLE_NETPLAY OR RG_ENABLE_NETWORKING)
            message(WARNING "Retro-Go's networking isn't compatible with esp-idf 5.x")
        endif()
    else()
        if(RG_ENABLE_NETPLAY)
            component_compile_options(-DRG_ENABLE_NETWORKING -DRG_ENABLE_NETPLAY)
        elseif(RG_ENABLE_NETWORKING)
            component_compile_options(-DRG_ENABLE_NETWORKING)
        endif()
    endif()

    if(RG_ENABLE_PROFILING)
        # Still debating whether -fno-inline is necessary or not...
        component_compile_options(-DRG_ENABLE_PROFILING -finstrument-functions)
    endif()
endmacro()
