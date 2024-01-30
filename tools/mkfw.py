#!/usr/bin/env python
import sys, math, zlib, struct

def readfile(filepath):
    try:
        with open(filepath, "rb") as f:
            return f.read()
    except FileNotFoundError as err:
        exit("\nERROR: Unable to open partition file '%s' !\n" % err.filename)

if len(sys.argv) < 9:
    exit("usage: mkfw.py [--esplay] output_file.fw 'description' tile.raw type subtype size label file.bin "
         "[type subtype size label file.bin, ...]")

args = sys.argv[1:]

try:
    args.remove("--esplay")
    fw_head = b"ESPLAY_FIRMWARE_V00_01"
    fw_pack = "<22s40s8256s"
except:
    fw_head = b"ODROIDGO_FIRMWARE_V00_01"
    fw_pack = "<24s40s8256s"

fw_name = args.pop(0)
fw_desc = args.pop(0)
fw_tile = args.pop(0)
fw_size = 0
fw_part = 0

fw_data = struct.pack(fw_pack, fw_head, fw_desc.encode(), readfile(fw_tile))

while len(args) >= 5:
    partype = int(args.pop(0), 0)
    subtype = int(args.pop(0), 0)
    size = int(args.pop(0), 0)
    label = args.pop(0)
    filename = args.pop(0)

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
