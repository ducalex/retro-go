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

if($ENV{ENABLE_NETPLAY})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_NETPLAY")
set(ENABLE_NETPLAY 1)
endif()

if($ENV{ENABLE_PROFILING})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DENABLE_PROFILING")
set(ENABLE_PROFILING 1)
endif()
