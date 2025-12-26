idf_component_register(SRC_DIRS "."
                    INCLUDE_DIRS "."
                    REQUIRES spi_flash unity test_utils littlefs spiffs fatfs esp_timer vfs)

target_add_binary_data(${COMPONENT_TARGET} "./testfs.bin" BINARY)