idf_component_register(SRCS "demo_esp_littlefs.c"
                    INCLUDE_DIRS "."
                    )

# Note: you must have a partition named the first argument (here it's "littlefs")
# in your partition table csv file.
littlefs_create_partition_image(littlefs ../flash_data FLASH_IN_PROJECT)
