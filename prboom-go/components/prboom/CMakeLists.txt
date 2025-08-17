set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()

# The PSRAM cache bug seems responsible for very odd bugs that I spent way too much time
# trying to debug... In retro-go the fix is disabled because of the huge performance overhead
# but I guess we'll have to live with it in at least the prboom-go module...
if(IDF_TARGET STREQUAL "esp32")
    component_compile_options(-mfix-esp32-psram-cache-issue)
endif()

rg_setup_compile_options(
    -Wno-error=address
    -Wno-misleading-indentation
    -Wno-format-overflow
    -Wno-char-subscripts
    -Wno-missing-field-initializers
    -DHAVE_CONFIG_H
    -O2
)
