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
  'retro-core':   [0, 0, 1638400],
  'prboom-go':    [0, 0, 851968],
  'gwenesis':     [0, 0, 983040],
  'fmsx':         [0, 0, 589824],
}
