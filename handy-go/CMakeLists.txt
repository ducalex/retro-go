cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "../components")
string(TIMESTAMP TODAY "%Y%m%d")
execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GITREV
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PROJECT_VER "${TODAY}-${GITREV}")
project(handy-go)
