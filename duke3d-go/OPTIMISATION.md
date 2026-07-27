# Duke3D Retro-Go Optimisation Summary

## Background

`duke3d-go` is based on Chocolate Duke3D and runs at the original 320x200 game resolution on the ESP32. Before optimisation it was already playable on the ESP32-S3 and ESP32-P4 at 320x240, but the original ESP32 commonly ran at around 15 FPS and could fall into single digits during combat or first-use sound loading.

The original ESP32 is the most difficult target because it combines:

- much less CPU performance than the S3 and P4;
- limited fast internal RAM;
- higher sensitivity to data placed in PSRAM;
- synchronous GRP, tile and sound I/O on the engine task;
- expensive software rendering at the full 320x200 resolution;
- audio mixing, music synthesis and first-use sound loading competing with rendering.

Lower rendering resolutions improved speed but produced an unacceptable loss of image quality, so all optimisation work retained 320x200 rendering and was applied across all supported targets.

## Main Optimisations

### Display and double buffering

The old display path rendered into one framebuffer and copied the complete 64 KB 320x200 image into a submission buffer every frame.

The optimised path renders directly into two stable indexed Retro-Go surfaces:

- each buffer owns stable pixel and palette storage;
- Retro-Go can consume one buffer asynchronously while Duke renders into the other;
- the fully redrawn 3D viewport is not copied between buffers;
- only incremental areas such as the HUD, status bar and borders are synchronized;
- splash screens, fades and loading screens request a full synchronization when required.

This removes the old full-frame copy from normal gameplay while preserving HUD, palette and loading-screen correctness.

### Renderer and game hot paths

Frequently executed renderer and game sources are compiled for speed rather than size. Additional work was applied to the Build renderer, drawing kernels, fixed-point maths, sprite/wall processing and other frequently executed loops.

The changes favour:

- simpler inner loops;
- fewer repeated calculations and memory accesses;
- fast internal memory for latency-sensitive lookup data where capacity permits;
- PSRAM for large cold or capacity-oriented data;
- bounds validation and sufficient wall capacity for all bundled demo maps.

These optimisations are common to ESP32, ESP32-S3 and ESP32-P4 rather than being disabled on faster chips.

### File and asset I/O

The original engine performs synchronous reads from the GRP and filesystem. This is especially visible when a tile or sound is requested for the first time.

The optimised I/O path includes:

- a hash index for faster GRP member lookup;
- buffered small and interleaved reads;
- a persistent DMA-capable bounce buffer for large reads;
- direct reads from memory-backed GRP archives;
- reduced temporary allocation and copying on common asset paths;
- dynamic cache sizing based on available external memory while retaining a safety reserve.

I/O can still cause a short stall when genuinely new data must be loaded, but the duration and frequency are reduced.

### Sound and music

Sound was a major source of visible frame-rate dips on the original ESP32. Mixing, OPL music rendering, synchronous first-use loads and audio submission could all contend with the renderer.

The optimised audio path:

- runs effects and music generation in a dedicated task;
- keeps the renderer on the other core where applicable;
- uses an audio priority high enough to prevent buffer starvation and first-sound lockups;
- mixes effects and stereo OPL music into a correctly counted Retro-Go stereo buffer;
- reuses fixed audio buffers instead of allocating during each mix;
- retains frequently requested or costly first-use sounds within a bounded cache budget;
- includes lower-cost mixing and reverb paths.

This does not eliminate the cost of every first-use sound, but it substantially reduces severe stalls and allows the game to recover more quickly.

### Retro-Go integration

The final build uses:

- native indexed Retro-Go display surfaces and `rg_display_submit()`;
- correctly counted stereo frames through `rg_audio_submit()`;
- abstract Retro-Go input;
- Retro-Go paths and storage APIs;
- `rg_system_tick()` once per presented frame for the standard FPS monitor;
- Retro-Go's standard `STACK`, `HEAP`, `BUSY`, `FPS` and battery diagnostics.


## Performance Results

Both builds played the complete built-in `demo1.dmo`. Results use the per-second FPS values reported by Retro-Go during active demo playback. Startup, loading, zero-FPS intervals and the first partial transition sample were excluded.

### Original build

| Platform | Minimum FPS | Maximum FPS | Average FPS |
|---|---:|---:|---:|
| ESP32 | 7 | 25 | 15.81 |
| ESP32-S3 | 20 | 37 | 29.92 |
| ESP32-P4 | 48 | 65 | 56.01 |

### Optimised build

| Platform | Minimum FPS | Maximum FPS | Average FPS |
|---|---:|---:|---:|
| ESP32 | 13 | 32 | 24.71 |
| ESP32-S3 | 27 | 39 | 33.01 |
| ESP32-P4 | 62 | 75 | 67.52 |

### Average improvement

| Platform | Original average | Optimised average | Improvement |
|---|---:|---:|---:|
| ESP32 | 15.81 FPS | 24.71 FPS | 56.3% |
| ESP32-S3 | 29.92 FPS | 33.01 FPS | 10.3% |
| ESP32-P4 | 56.01 FPS | 67.52 FPS | 20.6% |

The largest improvement is on the original ESP32. Its average increased by almost 9 FPS, while its measured minimum increased from 7 to 13 FPS. Combat and asset-loading dips remain possible, but the game spends substantially more time in the playable 20-30 FPS range and recovers from stalls more quickly.

The S3 and P4 also benefit, although their higher starting performance means the improvement is less dramatic visually. The same shared optimisations remain active on every supported target.
