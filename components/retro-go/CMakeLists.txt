set(COMPONENT_SRCDIRS ". fonts libs/netplay libs/lodepng")
set(COMPONENT_ADD_INCLUDEDIRS ". libs/netplay libs/lodepng")

message(STATUS "ESP-IDF version: ${IDF_VER}")

if (IDF_VERSION_MAJOR EQUAL 5)
    set(COMPONENT_REQUIRES "spi_flash fatfs app_update json nvs_flash esp_adc esp_timer esp_psram esp_wifi esp_http_client")
else() # esp-idf 4.1 - 4.4
    set(COMPONENT_REQUIRES "spi_flash fatfs app_update esp_adc_cal json nvs_flash esp_wifi esp_http_client")
endif()

register_component()

# Small size is preferred because of the small cache and most things here aren't performance sensitive!
# rg_display and rg_audio benefit from higher optimization (which of -O2 or -O3 is better depends...)

component_compile_options(
    -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
    -DLODEPNG_NO_COMPILE_ERROR_TEXT
    -Os
    -Wno-unused-function
)
set_source_files_properties(
    rg_audio.c rg_display.c rg_utils.c rg_surface.c
    PROPERTIES COMPILE_FLAGS
    -O2
)

if (IDF_VERSION_MAJOR EQUAL 5)
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
    component_compile_options(-DRG_ENABLE_PROFILING)
endif()

if(RG_PROJECT_VERSION)
    component_compile_options(-DRG_PROJECT_VERSION="${RG_PROJECT_VERSION}")
endif()

set(RG_TARGET "RG_TARGET_${RG_BUILD_TARGET}")
string(TIMESTAMP RG_TIME "%s")

message(STATUS "Target: ${RG_TARGET}")
message(STATUS "Time: ${RG_TIME}")

component_compile_options(-DRG_BUILD_TIME=${RG_TIME})
component_compile_options(-D${RG_TARGET})
