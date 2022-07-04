set(COMPONENT_SRCDIRS ". fonts netplay")
set(COMPONENT_ADD_INCLUDEDIRS ". netplay")
set(COMPONENT_REQUIRES "spi_flash fatfs app_update esp_adc_cal esp32 json")
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

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()

set(RG_TARGET "RG_TARGET_$ENV{RG_TARGET}")
component_compile_options(-D${RG_TARGET})
message("Target: ${RG_TARGET}")
