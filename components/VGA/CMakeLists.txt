set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "driver retro-go")

register_component()

# Small size is preferred because of the small cache and most things here aren't performance sensitive!
# rg_display and rg_audio benefit from higher optimization (which of -O2 or -O3 is better depends...)

component_compile_options(
    -DLODEPNG_NO_COMPILE_ANCILLARY_CHUNKS
    -DLODEPNG_NO_COMPILE_ERROR_TEXT
    -O3
    -Wno-unused-function
    -fexceptions
    -Wno-error=address
)

if(RG_ENABLE_PROFILING)
    component_compile_options(-DRG_ENABLE_PROFILING)
endif()

if(RG_BUILD_VERSION)
    component_compile_options(-DRG_BUILD_VERSION="${RG_BUILD_VERSION}")
endif()

if(RG_BUILD_TYPE)
    component_compile_options(-DRG_BUILD_TYPE=${RG_BUILD_TYPE})
endif()

set(RG_TARGET "RG_TARGET_${RG_BUILD_TARGET}")
string(TIMESTAMP RG_TIME "%s")

message(STATUS "Target: ${RG_TARGET}")
message(STATUS "Time: ${RG_TIME}")

component_compile_options(-DRG_BUILD_TIME=${RG_TIME})
component_compile_options(-D${RG_TARGET})