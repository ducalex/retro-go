include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "../components")
set(SDKCONFIG_DEFAULTS "../base.sdkconfig")

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
    component_compile_options(-O3 -Wno-comment -Wno-error=comment)
    component_compile_options(-DIS_LITTLE_ENDIAN)
    component_compile_options(${ARGV})

    if($ENV{ENABLE_PROFILING})
        # Still debating whether -fno-inline is necessary or not...
        component_compile_options(-DENABLE_PROFILING -finstrument-functions)
    endif()

    if($ENV{ENABLE_DEBUGGING})
        component_compile_options(-DENABLE_DEBUGGING)
    endif()

    if($ENV{ENABLE_NETPLAY})
        component_compile_options(-DENABLE_NETPLAY)
    endif()
endmacro()
