#!/usr/bin/env python
from os import path, getenv, getcwd, chdir
import sys, math, zlib, struct
import subprocess, argparse, shutil

IDF_PATH = getenv("IDF_PATH")
PRJ_PATH = sys.path[0]  # getcwd()

if not IDF_PATH:
    exit("IDF_PATH is not defined.")

def read_file(filepath):
    with open(filepath, "rb") as f: return f.read()
def shell_exec(cmd):
    return subprocess.check_output(cmd, shell=True).decode().rstrip()

chdir(PRJ_PATH)

exec(read_file("rg_config.py"))

parser = argparse.ArgumentParser(description="Retro-Go build tool")
parser.add_argument("command", choices=["build", "clean", "release", "mkfw", "flash", "flashmon", "monitor"])
parser.add_argument("targets", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys()))
parser.add_argument("--use-make", action="store_const", const=True, help="Use legacy make build system")
parser.add_argument("--port", default="COM3", help="Serial port to use for flash and monitor")
parser.add_argument("--offset", default="0x100000", help="Flash offset where %s is installed" % PROJECT_NAME)
parser.add_argument("--app-offset", default=None, help="Absolute offset where target will be flashed")
args = parser.parse_args()

command = args.command
targets = args.targets if not "all" in args.targets else PROJECT_APPS.keys()
prj_offset = int(args.offset, 0)
app_offset = int(args.app_offset, 0) if args.app_offset else None

def build_firmware():
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

    chdir(PRJ_PATH)

    description = ("%s %s" % (PROJECT_NAME, PROJECT_VER)).encode()
    filename = ("%s_%s.fw" % (PROJECT_NAME, PROJECT_VER)).lower()

    fw_data = struct.pack("<24s 40s 8256s", b"ODROIDGO_FIRMWARE_V00_01", description, read_file(PROJECT_TILE))

    for key in targets:
        data = read_file(path.join(key, "build", key + ".bin"))
        part = PROJECT_APPS[key]
        size = max(part[1], math.ceil(len(data) / 0x10000) * 0x10000)
        if size > part[1]:
            print("WARNING: Partition %s will be expanded by %d bytes" % (key, size - part[1]))
        fw_data += struct.pack("<BBxx16sIII", 0, part[0], key.encode(), 0, size, len(data))
        fw_data += data
        print("Partition: %s %d" % (key, size))

    fw_data += struct.pack("I", zlib.crc32(fw_data))

    fp = open(filename, "wb")
    fp.write(fw_data)
    fp.close()

    print("%s successfully created.\n" % filename)


def clean_app(target):
    print("Cleaning up app '%s'..." % target)
    try:  # idf.py fullclean
        shutil.rmtree(path.join(PRJ_PATH, target, "build"))
        print("Done.\n")
    except FileNotFoundError:
        print("Nothing to do.\n")


def build_app(target):
    print("Building app '%s'" % target)
    chdir(path.join(PRJ_PATH, target))
    if args.use_make:
        subprocess.run("make -j 6 app", shell=True, check=True)
    else:
        subprocess.run("idf.py app", shell=True, check=True)
    print("Done.\n")


def flash_app(target):
    if app_offset != None:
        offset = app_offset
    else:
        offset = prj_offset
        for key, value in PROJECT_APPS.items():
            if key == target: break
            offset += (math.ceil(value[1] / 0x10000) * 0x10000)

    print("Flashing app '%s' at offset %s" % (target, hex(offset)))
    subprocess.run([
        sys.executable,
        path.join(IDF_PATH, "components", "esptool_py", "esptool", "esptool.py"),
        "--chip", "esp32",
        "--port", args.port,
        "--baud", "1152000",
        "--before", "default_reset",
        "write_flash",
        "--flash_mode", "qio",
        "--flash_freq", "80m",
        "--flash_size", "detect",
        hex(offset),
        path.join(PRJ_PATH, target, "build", target + ".bin")
    ], check=True)
    print("Done.\n")


def monitor_app(target):
    print("Starting monitor for app '%s'" % target)
    subprocess.run([
        sys.executable,
        path.join(IDF_PATH, "tools", "idf_monitor.py"),
        "--port", args.port,
        path.join(PRJ_PATH, target, "build", target + ".elf")
    ])


if command == "clean" or command == "release":
    for key in targets:
        clean_app(key)

if command == "build" or command == "release":
    for key in targets:
        build_app(key)

if command == "release" or command == "mkfw":
    build_firmware()

if command == "flash" or command == "flashmon":
    for key in targets:
        flash_app(key)

if command == "monitor" or command == "flashmon":
    monitor_app(targets[0])


print("All done!")
