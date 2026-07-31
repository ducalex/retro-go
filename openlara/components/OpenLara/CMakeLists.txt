set(OPENLARA_SRC_DIR "${CMAKE_CURRENT_LIST_DIR}/src")

set(COMPONENT_SRCS
    "src/platform/retrogo/main.cpp"
    "src/libs/stb_vorbis/stb_vorbis.c"
    "src/libs/minimp3/minimp3.cpp"
    "src/libs/tinf/tinflate.c"
)

set(COMPONENT_ADD_INCLUDEDIRS
    "src"
    "src/platform/retrogo"
)

set(COMPONENT_REQUIRES retro-go esp_hw_support)

register_component()

if(RG_BUILD_TARGET)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE "-D${RG_BUILD_TARGET}=1")
endif()

target_compile_definitions(${COMPONENT_LIB} PRIVATE "-D__RETROGO__" "-D_GAPI_SW")
target_compile_options(${COMPONENT_LIB} PRIVATE
    "-std=gnu++17"
    "-fno-exceptions"
    "-fno-rtti"
    "-O3"
    "-w"
)
