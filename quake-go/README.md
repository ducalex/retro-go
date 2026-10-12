# Quake-Go

Quake-Go is a WinQuake software-renderer port integrated with Retro-Go for the
ESP32-S3 and ESP32-P4. It uses Retro-Go for launch content, storage, input,
display, audio, menus, screenshots, timing statistics, and shutdown.

## Game data

Provide your own Quake PAK files and select a `.pak` in the Retro-Go launcher.
Keep related files together: selecting `pak0.pak` also discovers sequential
siblings such as `pak1.pak` in the same directory. Selecting either conventional
PAK name launches the same data set: `pak0.pak` is required and `pak1.pak`
enables registered Quake. The exact selected file is honored even if it has a
non-standard name. The `rogue` and `hipnotic`
subdirectories enable their official mission-pack modes; other subdirectories
under `/sd/roms/quake` are launched with Quake's `-game` mod path.

The conventional locations are:

- `/sd/roms/quake/pak0.pak`
- `/sd/roms/quake/id1/pak0.pak`
- `/sd/roms/quake/id1/pak1.pak`
- `/sd/roms/quake/hipnotic/pak0.pak`
- `/sd/roms/quake/rogue/pak0.pak`

Mission packs require the registered base-game `pak0.pak` and `pak1.pak` in
`id1` (or at the Quake root as a fallback). *Scourge of Armagon* (`hipnotic`)
and *Dissolution of Eternity* (`rogue`) contain maps whose working sets exceed
the practical combined hunk and renderer-cache capacity of 8 MiB S3 devices.
Both mission packs are supported on P4 only. S3 launch is blocked with an
explanatory alert rather than risking an out-of-memory panic later in a map.

Quake configuration and native save games are stored under Retro-Go's config
and save roots. Use Quake's in-game menus for native saving, loading, and a new
game. Retro-Go's screenshot command is supported.

## Music

Optional music uses 22,050 Hz stereo, signed 16-bit PCM WAV files named
`track02.wav`, `track03.wav`, etc. Put them in the active game's music folder:

- `roms/quake/id1/music/` for the base game
- `roms/quake/hipnotic/music/` for Scourge of Armagon
- `roms/quake/rogue/music/` for Dissolution of Eternity

Mods use their own `music/` folder beside their PAK files. Music is
optional and does not prevent playing. Retro-Go's emulator options include a
saved **Music On/Off** setting, defaulting to On. Quake's sound and music volume
sliders adjust the mix; Retro-Go controls the combined output volume.

See [tools/README.md](tools/README.md) for MP3/OGG conversion with Python or
FFmpeg alone, naming, and installation instructions. You can Google search for the following files to find the OGG versions: `quake_music.zip, rogue_music.zip, hipnotic_music.zip` or follow this link: https://www.quaddicted.com/files/music/

## Controls

- D-pad: move and turn; navigate native menus
- A: fire; accept/yes in native menus
- B: jump/swim up; back/no in native menus
- X: swim down (duplicates START)
- Y: next weapon (duplicates SELECT)
- L/R: strafe
- SELECT: next weapon
- START: swim down (hold)
- Short MENU: Quake menu
- Long MENU (500 ms): Retro-Go game menu
- Long OPTION (500 ms): Retro-Go options menu

Buttons still held when a Retro-Go dialog closes are suppressed until release.

## Target configuration

- ESP32-S3: 320x200, 6.25 MiB Quake hunk
- ESP32-P4: 320x240, 10 MiB Quake hunk
- Original ESP32: experimental 160x120, 3.4 MiB hunk; not currently supported

Fullscreen scaling is the first-launch default. Subsequent launches preserve
the user's Retro-Go display setting.

## Credits and license

This port is based on work by `thisiseth` from the
[`tang-primer-25k-spi-io`](https://github.com/thisiseth/tang-primer-25k-spi-io)
project.

Quake is Copyright 1996-1997 id Software. This port is distributed under the
GNU General Public License included with the source.
