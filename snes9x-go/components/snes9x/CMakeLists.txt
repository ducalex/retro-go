set(COMPONENT_SRCS "c4.c c4emu.c clip.c cpu.c cpuexec.c cpuops.c data.c dma.c dsp1.c gfx.c getset.c globals.c memmap.c obc1.c ppu.c srtc.c tile.c apu_blargg.c apu.c soundux.c spc700.c")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_REQUIRES "retro-go")
register_component()
rg_setup_compile_options(
    -Wno-error=format-truncation
    -Wno-error=stringop-overflow
    -Wno-missing-field-initializers
    -Wno-unused-function
    -Wno-error=array-bounds
    -Wno-error=restrict
    -Wno-array-bounds
    -O2
)
