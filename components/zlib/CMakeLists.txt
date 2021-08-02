set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
register_component()
component_compile_options(-Os -Wno-implicit-fallthrough -DHAVE_UNISTD_H -DHAVE_STDARG_H -DMAX_MEM_LEVEL=6 -DNO_GZIP)
