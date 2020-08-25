#!/usr/bin/env python
import sys, os, subprocess, struct, zlib
from projects import PROJECTS

os.chdir(sys.path[0])

FW_VERSION = subprocess.check_output("git describe --tags --abbrev=5 --dirty", shell=True, universal_newlines=True).strip()
FW_DESC = "Retro-Go " + FW_VERSION
FW_TILE = "assets/tile.raw"
FW_FILE = "retro-go_%s.fw" % FW_VERSION

# Header:
#   "ODROIDGO_FIRMWARE_V00_01"  24 bytes
#   Firmware Description        40 bytes
#   RAW565 86x48 tile           8256 bytes
# Partition [, ...]:
#   Type                        1 byte
#   Subtype                     1 byte
#   Padding                     2 bytes
#   Label                       16 bytes
#   Flags                       4 bytes
#   Size                        4 bytes
#   Data length                 4 bytes
# Footer:
#   CRC32                       4 bytes

with open(FW_TILE, "rb") as fp:
    tile_data = fp.read()

fw_data = struct.pack("24s 40s 8256s", b"ODROIDGO_FIRMWARE_V00_01", FW_DESC.encode(), tile_data)

for key, value in PROJECTS.items():
    with open(key + "/build/" + key + ".bin", "rb") as fp:
        data = fp.read()
    fw_data += struct.pack("BBxx16sIII", 0, value[0], key.encode(), 0, value[1], len(data))
    fw_data += data

fw_data += struct.pack("I", zlib.crc32(fw_data))

fp = open(FW_FILE, "wb")
fp.write(fw_data)
fp.close()
