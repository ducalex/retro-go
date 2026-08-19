idf_component_register(SRCS "hooks.c"
                       PRIV_REQUIRES hal bootloader_support spi_flash soc esp_rom)

target_link_libraries(${COMPONENT_LIB} INTERFACE "-Wl,--undefined=bootloader_hooks_include")



