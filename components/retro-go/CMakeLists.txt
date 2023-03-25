set(COMPONENT_SRCDIRS ". fonts libs/netplay libs/lodepng")
set(COMPONENT_ADD_INCLUDEDIRS ". libs/netplay libs/lodepng")

# esp-idf 4.1 - 4.4
set(COMPONENT_REQUIRES "spi_flash fatfs app_update esp_adc_cal json nvs_flash esp_wifi esp_http_client")
# esp-idf 5.0 -
#set(COMPONENT_REQUIRES "spi_flash fatfs app_update esp_adc esp_timer json nvs_flash esp_wifi esp_http_client")

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
    rg_audio.c rg_display.c
    PROPERTIES COMPILE_FLAGS
    -O3
)

if($ENV{RG_ENABLE_NETPLAY})
    component_compile_options(-DRG_ENABLE_NETWORKING -DRG_ENABLE_NETPLAY)
elseif($ENV{RG_ENABLE_NETWORKING})
    component_compile_options(-DRG_ENABLE_NETWORKING)
endif()

if($ENV{RG_ENABLE_PROFILING})
    component_compile_options(-DRG_ENABLE_PROFILING)
endif()

if($ENV{RG_BUILD_TIME})
    component_compile_options(-DRG_BUILD_TIME=$ENV{RG_BUILD_TIME})
endif()

set(RG_TARGET "RG_TARGET_$ENV{RG_BUILD_TARGET}")
component_compile_options(-D${RG_TARGET})
message("Target: ${RG_TARGET}")
