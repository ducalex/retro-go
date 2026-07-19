file(GLOB CORE_SRCS "emucore/*.cpp")

set(COMPONENT_SRCS
    "StellaDS.cpp"
    "savestate.cpp"
    ${CORE_SRCS}
)

set(COMPONENT_ADD_INCLUDEDIRS ". emucore")
set(COMPONENT_REQUIRES "retro-go")

register_component()

rg_setup_compile_options()
