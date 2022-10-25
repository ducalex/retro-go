# Notes:
# - Enabling netplay in an emulator increases its size by ~350KB (~450KB in esp-idf 4.0)
# - Enabling profiling in an emulator increases its size by ~75KB (without no-inline)
# - Keep at least 32KB free in a partition for future updates
# - Partitions must be 64K aligned
# - Partitions of type data are ignored when building a .fw.
# - Subtypes and offsets and size may be adjusted when building a .fw or .img

PROJECT_NAME = "Retro-Go"
PROJECT_ICON = "assets/icon.raw"
PROJECT_APPS = {
  # Project name  Type, SubType, Size
  'launcher':     [0, 0, 786432],
  'retro-run':    [0, 0, 655360],
  'smsplusgx-go': [0, 0, 393216],
  'snes9x-go':    [0, 0, 524288],
  'prboom-go':    [0, 0, 786432],
  'gwenesis':     [0, 0, 983040],
}
