set(COMPONENT_SRCDIRS ". fonts libs/netplay libs/lodepng")
set(COMPONENT_ADD_INCLUDEDIRS ". libs/netplay libs/lodepng")
if("${IDF_VERSION_MAJOR}.${IDF_VERSION_MINOR}" VERSION_GREATER_EQUAL "5.0")
    set(COMPONENT_REQUIRES "spi_flash fatfs nvs_flash app_update driver esp_timer esp_adc json")
else()
    set(COMPONENT_REQUIRES "spi_flash fatfs nvs_flash app_update esp_adc_cal esp32 json")
endif()
register_component()

# Small size is preferred because of the small cache and most things here aren't performance sensitive!
# rg_display and rg_audio benefit from higher optimization (which of -O2 or -O3 is better depends...)

component_compile_options(
    -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
    -DLODEPNG_NO_COMPILE_ERROR_TEXT
    -DRG_REPLACE_PRINTF_FUNCTIONS
    -Os
)
set_source_files_properties(
    rg_audio.c rg_display.c
    PROPERTIES COMPILE_FLAGS
    -O3
)

if($ENV{RG_ENABLE_WIFI})
    component_compile_options(-DRG_ENABLE_WIFI)
endif()

if($ENV{RG_ENABLE_PROFILING})
    component_compile_options(-DRG_ENABLE_PROFILING -finstrument-functions)
endif()

if($ENV{RG_ENABLE_NETPLAY})
    component_compile_options(-DRG_ENABLE_NETPLAY)
endif()

if($ENV{RG_BUILD_TIME})
    component_compile_options(-DRG_BUILD_TIME=$ENV{RG_BUILD_TIME})
endif()

set(RG_TARGET "RG_TARGET_$ENV{RG_BUILD_TARGET}")
component_compile_options(-D${RG_TARGET})
message("Target: ${RG_TARGET}")
