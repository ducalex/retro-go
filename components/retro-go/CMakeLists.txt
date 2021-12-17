set(COMPONENT_SRCDIRS ". fonts")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "spi_flash fatfs app_update esp_adc_cal esp32 json")
register_component()

component_compile_options(-O3)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()

set(RG_TARGET "RG_TARGET_$ENV{RG_TARGET}")
component_compile_options(-D${RG_TARGET})
message("Target: ${RG_TARGET}")
