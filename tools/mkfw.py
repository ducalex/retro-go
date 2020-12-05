#!/usr/bin/env python
import sys, math, zlib, struct

def readfile(filepath):
    try:
        with open(filepath, "rb") as f:
            return f.read()
    except FileNotFoundError as err:
        exit("\nERROR: Unable to open partition file '%s' !\n" % err.filename)

if len(sys.argv) < 4:
    exit("usage: mkfw.py output_file.fw 'description' tile.raw type subtype size label file.bin "
         "[type subtype size label file.bin, ...]")

fw_name = sys.argv[1]

fw_data = struct.pack(
    "<24s40s8256s", b"ODROIDGO_FIRMWARE_V00_01", sys.argv[2].encode(), readfile(sys.argv[3])
)

fw_size = 0
fw_part = 0
pos = 4

while pos < len(sys.argv):
    partype = int(sys.argv[pos + 0], 0)
    subtype = int(sys.argv[pos + 1], 0)
    size = int(sys.argv[pos + 2], 0)
    label = sys.argv[pos + 3]
    filename = sys.argv[pos + 4]
    pos += 5

    if not subtype:
        subtype = 16 + fw_part # OTA starts at 16

    data = readfile(filename)
    real_size = max(size, math.ceil(len(data) / 0x10000) * 0x10000)
    usage = len(data) / real_size * 100

    print("[%d]: type=%d, subtype=%d, size=%d (%d%% used), label=%s"
        % (fw_part, partype, subtype, real_size, usage, label))

    if real_size > size and size != 0:
        print(" > WARNING: Partition smaller than file (+%d bytes), increasing size to %d"
            % (len(data) - size, real_size))

    fw_data += struct.pack("<BBxx16sIII", partype, subtype, label.encode(), 0, real_size, len(data))
    fw_data += data
    fw_size += real_size
    fw_part += 1

fw_data += struct.pack("I", zlib.crc32(fw_data))

fp = open(fw_name, "wb")
fp.write(fw_data)
fp.close()

print("\nTotal size: %.3f MB" % (fw_size / 1048576))
print("\nFile '%s' successfully created.\n" % (fw_name))
