include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")
set(SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/base.sdkconfig")

execute_process(
  COMMAND git describe --tags --abbrev=5 --long --dirty=*
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_VARIABLE PROJECT_VER
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT PROJECT_VER)
    string(TIMESTAMP TODAY "%Y.%m.%d")
    set(PROJECT_VER "${TODAY}-nogit")
endif()

macro(rg_setup_compile_options)
    component_compile_options(-Wno-comment -Wno-error=comment -Wno-missing-field-initializers)
    component_compile_options(-DIS_LITTLE_ENDIAN)
    component_compile_options(${ARGV})

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

    set(RG_TARGET "RG_TARGET_$ENV{RG_TARGET}")
    component_compile_options(-D${RG_TARGET})
    message("Target: ${RG_TARGET}")
endmacro()
