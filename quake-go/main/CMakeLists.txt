idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS ".")

rg_setup_compile_options()

# Keep the platform component fixed; hand menu exits back to app_main before
# Retro-Go accesses flash from the engine task's PSRAM-backed stack.
target_link_options(${COMPONENT_LIB} INTERFACE "-Wl,--wrap=rg_system_exit")
