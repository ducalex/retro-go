set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()

# The PSRAM cache bug seems responsible for very odd bugs that I spent way too much time
# trying to debug... In retro-go the fix is disabled because of the huge performance overhead
# but I guess we'll have to live with it in at least the prboom-go module...

rg_setup_compile_options(
    -Wno-missing-field-initializers
    -mfix-esp32-psram-cache-issue
    -DHAVE_CONFIG_H
    -O2
)
