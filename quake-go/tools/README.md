# Quake music converter

Requires Python 3.8+ and FFmpeg. Python's standard library cannot decode
MP3/OGG; this script uses FFmpeg to generate Quake-compatible WAV files.

Put `ffmpeg.exe` (Windows) or a native executable named `ffmpeg` (Linux)
in the `ffmpeg` folder beside `convert_music.py`. On Linux ensure the binary
is executable. Alternatively pass `--ffmpeg /path/to/ffmpeg` or
`--ffmpeg ffmpeg` for a PATH installation.

Put MP3/OGG tracks in this folder or its subfolders, preferably:

```
music/id1/track002.ogg
music/hipnotic/track02.mp3
music/rogue/track02.ogg
```

Keep each soundtrack in a separate folder. Run from this folder:

```
python convert_music.py
```

On Linux use `python3`. From elsewhere, invoke the script using its path;
input discovery always starts beside the script, not in the current working
directory. The entire `tools` folder can be copied to another PC.

The converter scans subfolders and normalizes `track002` to `track02`.
Each soundtrack gets its own `wav` output folder, containing 22,050 Hz,
stereo, signed 16-bit PCM WAV. Duplicate OGG/MP3 tracks in the same folder
prefer OGG. Originals and existing outputs are preserved; pass `--overwrite`
to regenerate outputs. Failed conversions preserve any previous usable WAV.
Generated `wav` folders, `ffmpeg`, `__pycache__` and directory links are skipped.

Optional input/output overrides:

```
python convert_music.py /path/to/soundtracks --output /path/to/converted
```

With `--output`, subfolder structure is retained beneath that output root;
the output root is excluded from input scanning. The input and output roots
must differ.

Copy each soundtrack's resulting WAVs to its SD-card folder:
`roms/quake/id1/music`, `roms/quake/hipnotic/music` or
`roms/quake/rogue/music`. Do not copy the enclosing `wav` folder.

## FFmpeg alone (without Python)

Run from this `tools` folder, creating the destination folder first. Convert
one track with the bundled Windows executable:

```bat
ffmpeg\ffmpeg.exe -n -i "music\id1\track002.ogg" -map 0:a:0 -vn -ac 2 -ar 22050 -c:a pcm_s16le -map_metadata -1 "music\id1\wav\track02.wav"
```

On Linux:

```sh
./ffmpeg/ffmpeg -n -i "music/id1/track002.ogg" -map 0:a:0 -vn -ac 2 -ar 22050 -c:a pcm_s16le -map_metadata -1 "music/id1/wav/track02.wav"
```

Replace the input with your MP3 or OGG and choose the corresponding output
track number. Use `track02.wav`, even when the source is named `track002`.
`-n` preserves existing outputs; use `-y` instead to overwrite deliberately.
These commands produce the same 22,050 Hz stereo PCM16 format as the script.
The script additionally handles whole folders, naming and validation, and
preserves existing usable WAVs if a replacement conversion fails.
