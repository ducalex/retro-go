# This file contains a list of Retro-Go's sub-projects. It is used by build_all.py, flashmon.py,
# and mkfw.py to derive flash offsets and a partition table.

# Notes:
# - Enabling netplay in an emulator increases its size by ~350KB
# - Keep at least 32KB free in a partition for future updates
# - Partitions must be 64K aligned

PROJECTS = {
 # Project name    Sub, Size
  'retro-go':     [16,  524288],
  'nofrendo-go':  [17,  458752],
  'gnuboy-go':    [18,  458752],
  'smsplusgx-go': [19,  524288],
  'huexpress-go': [20,  458752],
  'handy-go':     [21,  458752],
}
