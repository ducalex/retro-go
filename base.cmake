include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/components")

macro(rg_setup_compile_options)
    component_compile_options(
        -D${RG_BUILD_TARGET}=1
        -DRETRO_GO=1
        -fjump-tables -ftree-switch-conversion
        ${ARGV}
    )

    # The PSRAM cache bug is responsible for many subtile bugs and crashes. The workaround has a
    # significant performance impact but the alternative is instability... Enabling the fix here
    # instead of sdkconfig prevents the new libc and wifi from being linked in which increases
    # size and reduces performance further. It's not entirely safe but retro-go has been built
    # with the workaround fully disabled for years, so clearly this compromise should be "fine".
    # memw seems to have the least impact both in terms of size and performance
    if(IDF_TARGET STREQUAL "esp32")
        component_compile_options(-mfix-esp32-psram-cache-issue -mfix-esp32-psram-cache-strategy=memw)
    endif()

    if(RG_ENABLE_NETPLAY)
        component_compile_options(-DRG_ENABLE_NETWORKING -DRG_ENABLE_NETPLAY)
    elseif(RG_ENABLE_NETWORKING)
        component_compile_options(-DRG_ENABLE_NETWORKING)
    endif()

    if(RG_ENABLE_PROFILING)
        # Still debating whether -fno-inline is necessary or not...
        component_compile_options(-DRG_ENABLE_PROFILING -finstrument-functions)
    endif()
endmacro()
