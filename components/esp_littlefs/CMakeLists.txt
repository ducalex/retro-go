cmake_minimum_required(VERSION 3.10)

file(GLOB SOURCES src/littlefs/*.c)
list(APPEND SOURCES src/esp_littlefs.c src/littlefs_esp_part.c src/lfs_config.c)

if(IDF_TARGET STREQUAL "esp8266")
    # ESP8266 configuration here
else()
    # non-ESP8266 configuration
    list(APPEND pub_requires sdmmc)

    if(CONFIG_LITTLEFS_SDMMC_SUPPORT)
        list(APPEND SOURCES src/littlefs_sdmmc.c)
    endif()
endif()

list(APPEND pub_requires esp_partition)
list(APPEND priv_requires esptool_py spi_flash vfs)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS include
    PRIV_INCLUDE_DIRS src
    REQUIRES ${pub_requires}
    PRIV_REQUIRES ${priv_requires}
)

set_source_files_properties(
    ${SOURCES}
    PROPERTIES COMPILE_FLAGS "-DLFS_CONFIG=lfs_config.h"
)

if(CONFIG_LITTLEFS_FCNTL_GET_PATH)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC -DF_GETPATH=${CONFIG_LITTLEFS_FCNTL_F_GETPATH_VALUE})
endif()

if(CONFIG_LITTLEFS_MULTIVERSION)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC -DLFS_MULTIVERSION)
endif()

if(CONFIG_LITTLEFS_MALLOC_STRATEGY_DISABLE)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC -DLFS_NO_MALLOC)
endif()

if(NOT CONFIG_LITTLEFS_ASSERTS)
    target_compile_definitions(${COMPONENT_LIB} PUBLIC -DLFS_NO_ASSERT)
endif()
