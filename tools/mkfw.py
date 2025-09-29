#!/usr/bin/env python
import argparse, math, struct, time, zlib

def readfile(filepath):
    try:
        with open(filepath, "rb") as f:
            return f.read()
    except OSError as err:
        exit("\nERROR: Failed to read file '%s': %s\n" % (err.filename, err.strerror))


def get_partitions(args, flash_offset = 0):
    print("\nPartitions:")
    partitions = []
    # ota_next_id = 16
    while len(args) >= 5:
        partype = int(args.pop(0), 0)
        subtype = int(args.pop(0), 0)
        size = int(args.pop(0), 0)
        label = args.pop(0)
        filename = args.pop(0)

        data = readfile(filename) if filename != "none" else b""

        flash_alignment = 0x10000 if partype == 0 else 0x1000
        flash_offset = math.ceil(flash_offset / flash_alignment) * flash_alignment
        # Technically the size only has to be aligned to 0x1000, but it's wasted space if two apps follow eachother
        flash_size = math.ceil(max(size, len(data)) / flash_alignment) * flash_alignment

        print("  [%d]: type=%d, subtype=%d, size=%d (%d%% used), label=%s"
            % (len(partitions), partype, subtype, flash_size, len(data) / flash_size * 100, label))

        if size != 0 and len(data) > size:
            print("      > WARNING: File larger than partition (+%d bytes), increased size to %d"
                % (len(data) - size, flash_size))
        elif size != 0 and size != flash_size:
            print("      > WARNING: Incorrect alignment, adjusted size to %d" % (flash_size))

        partitions.append((partype, subtype, label, flash_offset, flash_size, data))
        flash_offset += flash_size

    if len(args) > 0:
        print(f"WARNING: Trailing arguments (incomplete partition?): {args}")

    print("Flash size: %.3f MB\n" % (flash_offset / 1048576))
    return partitions


def create_firmware(fw_type, partitions, icon_file, name, version, target):
    description =  f"{name} {version}".encode()
    tile_bin = readfile(icon_file) if icon_file != "none" else b"\x00" * 8256
    if fw_type == "odroid":
        fw_data = struct.pack("<24s40s8256s", b"ODROIDGO_FIRMWARE_V00_01", description, tile_bin)
    else:
        fw_data = struct.pack("<22s40s8256s", b"ESPLAY_FIRMWARE_V00_01", description, tile_bin)
    for partype, subtype, label, offset, size, data in partitions:
        fw_data += struct.pack("<BBxx16sIII", partype, subtype, label.encode(), 0, size, len(data))
        fw_data += data
    return fw_data + struct.pack("I", zlib.crc32(fw_data))


def create_image(chip_type, partitions, bootloader_file, name, version, target):
    bootloader_offset, table_offset, prog_offset = {
        "esp32":   (0x1000, 0x8000, 0x10000),
        "esp32s3": (0x0000, 0x8000, 0x10000),
        "esp32p4": (0x2000, 0x8000, 0x10000),
    }.get(chip_type)
    partitions = [
        (1, 0x02, "nvs", 0x9000, 0x4000, b""),
        (1, 0x00, "otadata", 0xD000, 0x2000, b""),
        (1, 0x01, "phy_init", 0xF000, 0x1000, b""),
    ] + partitions
    output_data = bytearray(b"\xFF" * max(p[3] + p[4] for p in partitions))

    # It would be better to call gen_esp32part.py but it's a hassle to make it work for all our supported platforms...
    table_bin = b""
    for partype, subtype, label, offset, size, data in partitions:
        table_bin += struct.pack("<2sBBLL16sL", b"\xAA\x50", partype, subtype, offset, size, label.encode(), 0)
    table_bin += b"\xFF" * (0xC00 - len(table_bin))  # Pad table with 0xFF otherwise it will be invalid
    # The lack of an MD5 entry is deliberate to maintain compatibility with very old bootloaders
    output_data[table_offset : table_offset + len(table_bin)] = table_bin

    bootloader_bin = readfile(bootloader_file)
    output_data[bootloader_offset : bootloader_offset + len(bootloader_bin)] = bootloader_bin

    for partype, subtype, label, offset, size, data in partitions:
        output_data[offset : offset + len(data)] = data

    # Append the information structure used by retro-go's updater.
    # (Maybe I could hide it somewhere else to avoid changing the size of the image)
    output_data += struct.pack(
        "<8s28s28s28sII156s",
        "RG_IMG_0".encode(),    # Magic number
        name.encode(),          # Project name
        version.encode(),       # Project version
        target.encode(),        # Project target device
        int(time.time()),       # Unix timestamp
        zlib.crc32(output_data),# CRC of the image not including this footer
        b"\xff" * 156,          # 0xFF padding of the reserved area
    )
    return output_data


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Retro-Go firmware tool")
    parser.add_argument("--type", choices=["odroid", "esplay", "esp32", "esp32s3", "esp32p4"], default="odroid")
    parser.add_argument("--bootloader", default="none")
    parser.add_argument("--name", default="unknown")
    parser.add_argument("--icon", default="none")
    parser.add_argument("--version", default="unknown")
    parser.add_argument("--target", default="unknown")
    parser.add_argument("output_file")
    parser.add_argument("partitions", nargs="*", metavar="(type subtype size label file.bin)")
    args = parser.parse_args()

    if args.type in ["odroid", "esplay"]:
        partitions = get_partitions(args.partitions)
        data = create_firmware(args.type, partitions, args.icon, args.name, args.version, args.target)
    else:
        partitions = get_partitions(args.partitions, 0x10000)
        data = create_image(args.type, partitions, args.bootloader, args.name, args.version, args.target)

    with open(args.output_file, "wb") as fp:
        fp.write(data)

    print("File '%s' successfully created.\n" % (args.output_file))
