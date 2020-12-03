# Important: This file is now exec() in the global scope
# Notes:
# - Enabling netplay in an emulator increases its size by ~350KB (~450KB in esp-idf 4.0)
# - Enabling profiling in an emulator increases its size by ~75KB (without no-inline)
# - Keep at least 32KB free in a partition for future updates
# - Partitions must be 64K aligned

PROJECT_NAME = "Retro-Go"
PROJECT_VER  = shell_exec("git describe --tags --abbrev=5 --dirty")
PROJECT_TILE = "fw-icon.raw"
PROJECT_APPS = {
  # Note: Size will be adjusted if needed but flashmon needs accurate values to work correctly
  # Project name   Sub, Size
  'retro-go':     [16,  524288],
  'nofrendo-go':  [17,  458752],
  'gnuboy-go':    [18,  458752],
  'smsplusgx-go': [19,  524288],
  'huexpress-go': [20,  458752],
  'handy-go':     [21,  458752],
}
