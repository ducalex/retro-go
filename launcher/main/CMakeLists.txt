set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
if(IDF_TARGET STREQUAL "esp32")
    component_compile_options(-mfix-esp32-psram-cache-issue)
endif()
rg_setup_compile_options(-O2 -Wno-error=format -Wno-error=char-subscripts)

# add_custom_command(OUTPUT images.c
#                    COMMAND python ${COMPONENT_DIR}/gen_images.py
#                    VERBATIM)
# add_custom_target(images DEPENDS images.c)
# add_dependencies(${COMPONENT_LIB} images)
