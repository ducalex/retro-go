#!/usr/bin/env python
import sys, math, zlib, struct

def readfile(filepath):
    with open(filepath, "rb") as f:
        return f.read()

if len(sys.argv) < 4:
    exit("usage: mkfw.py output_file description tile <type subtype length label binfile> [, ...]")

fw_name = sys.argv[1]

fw_data = struct.pack(
    "<24s40s8256s", b"ODROIDGO_FIRMWARE_V00_01", sys.argv[2].encode(), readfile(sys.argv[3])
)

count = 0
pos = 4

while pos < len(sys.argv):
    partype = int(sys.argv[pos + 0], 0)
    subtype = int(sys.argv[pos + 1], 0)
    length = int(sys.argv[pos + 2], 0)
    label = sys.argv[pos + 3]
    data = readfile(sys.argv[pos + 4])
    pos += 5
    usage = len(data) / length * 100

    print("[%d]: type=%d, subtype=%d, length=%d (%d%% used), label=%s"
        % (count, partype, subtype, length, usage, label))

    size = max(length, math.ceil(len(data) / 0x10000) * 0x10000)
    if size > length:
        print(" > WARNING: Partition smaller than file (+%d bytes), increasing length to %d"
            % (len(data) - length, size))

    fw_data += struct.pack("<BBxx16sIII", partype, subtype, label.encode(), 0, size, len(data))
    fw_data += data
    count += 1

fw_data += struct.pack("I", zlib.crc32(fw_data))

fp = open(fw_name, "wb")
fp.write(fw_data)
fp.close()

print("%s successfully created.\n" % fw_name)
