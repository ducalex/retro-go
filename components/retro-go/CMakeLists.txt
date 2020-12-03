set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "nvs_flash spi_flash fatfs app_update esp_adc_cal esp32 json")
register_component()

component_compile_options(-O3)

if($ENV{ENABLE_PROFILING})
    component_compile_options(-DENABLE_PROFILING)
endif()

if($ENV{ENABLE_DEBUGGING})
    component_compile_options(-DENABLE_DEBUGGING)
endif()

if($ENV{ENABLE_NETPLAY})
    component_compile_options(-DENABLE_NETPLAY)
endif()
