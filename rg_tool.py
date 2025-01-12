#!/usr/bin/env python3
import argparse
import subprocess
import shutil
import glob
import math
import sys
import re
import os

IDF_PATH = os.getenv("IDF_PATH")
if not IDF_PATH:
    exit("IDF_PATH is not defined. Are you running inside esp-idf environment?")

TARGETS = ["odroid-go"] # We just need to specify the default, the others are discovered below
for t in glob.glob("components/retro-go/targets/*/config.h"):
    TARGETS.append(os.path.basename(os.path.dirname(t)))

DEFAULT_TARGET = os.getenv("RG_TOOL_TARGET", TARGETS[0])
DEFAULT_BAUD = os.getenv("RG_TOOL_BAUD", "1152000")
DEFAULT_PORT = os.getenv("RG_TOOL_PORT", "COM3")
PROJECT_NAME = os.getenv("PROJECT_NAME", "Retro-Go") # os.path.basename(os.getcwd()).title()
PROJECT_ICON = os.getenv("PROJECT_ICON", "icon.raw")
PROJECT_APPS = {}
try:
    PROJECT_VER = os.getenv("PROJECT_VER") or subprocess.check_output(
        "git describe --tags --abbrev=5 --dirty --always", shell=True
    ).decode().rstrip()
except:
    PROJECT_VER = "unknown"

if os.name == 'nt':
    IDF_PY = os.path.join(IDF_PATH, "tools", "idf.py")
    IDF_MONITOR_PY = os.path.join(IDF_PATH, "tools", "idf_monitor.py")
    ESPTOOL_PY = os.path.join(IDF_PATH, "components", "esptool_py", "esptool", "esptool.py")
    PARTTOOL_PY = os.path.join(IDF_PATH, "components", "partition_table", "parttool.py")
    GEN_ESP32PART_PY = os.path.join(IDF_PATH, "components", "partition_table", "gen_esp32part.py")
else:
    IDF_PY = "idf.py"
    IDF_MONITOR_PY = "idf_monitor.py"
    ESPTOOL_PY = "esptool.py"
    PARTTOOL_PY = "parttool.py"
    GEN_ESP32PART_PY = "gen_esp32part.py"
MKFW_PY = os.path.join("tools", "mkfw.py")

if os.path.exists("rg_config.py"):
    with open("rg_config.py", "rb") as f:
        exec(f.read())


def run(cmd, cwd=None, check=True):
    print(f"Running command: {' '.join(cmd)}")
    if os.name == 'nt' and cmd[0].endswith(".py"):
        return subprocess.run(["python", *cmd], shell=True, cwd=cwd, check=check)
    return subprocess.run(cmd, shell=False, cwd=cwd, check=check)


def build_firmware(output_file, apps, fw_format="odroid-go", fatsize=0):
    print("Building firmware with: %s\n" % " ".join(apps))
    args = [MKFW_PY, output_file, f"{PROJECT_NAME} {PROJECT_VER}", PROJECT_ICON]

    if fw_format == "esplay":
        args.append("--esplay")

    for app in apps:
        part = PROJECT_APPS[app]
        args += [str(part[0]), str(part[1]), str(part[2]), app, os.path.join(app, "build", app + ".bin")]

    run(args)


def build_image(output_file, apps, img_format="esp32", fatsize=0):
    print("Building image with: %s\n" % " ".join(apps))
    image_data = bytearray(b"\xFF" * 0x10000)
    table_ota = 0
    table_csv = [
        "nvs, data, nvs, 36864, 16384",
        "otadata, data, ota, 53248, 8192",
        "phy_init, data, phy, 61440, 4096",
    ]

    for app in apps:
        with open(os.path.join(app, "build", app + ".bin"), "rb") as f:
            data = f.read()
        part_size = max(PROJECT_APPS[app][2], math.ceil(len(data) / 0x10000) * 0x10000)
        table_csv.append("%s, app, ota_%d, %d, %d" % (app, table_ota, len(image_data), part_size))
        table_ota += 1
        image_data += data + b"\xFF" * (part_size - len(data))

    if fatsize:
        # Use "vfs" label, same as MicroPython, in case the storage is to be shared with a MicroPython install
        table_csv.append("vfs, data, fat, %d, %s" % (len(image_data), fatsize))

    print("Generating partition table...")
    with open("partitions.csv", "w") as f:
        f.write("\n".join(table_csv))
    run([GEN_ESP32PART_PY, "partitions.csv", "partitions.bin"])
    with open("partitions.bin", "rb") as f:
        table_bin = f.read()

    print("Building bootloader...")
    bootloader_file = os.path.join(os.getcwd(), list(apps)[0], "build", "bootloader", "bootloader.bin")
    if not os.path.exists(bootloader_file):
        run([IDF_PY, "bootloader"], cwd=os.path.join(os.getcwd(), list(apps)[0]))
    with open(bootloader_file, "rb") as f:
        bootloader_bin = f.read()

    if img_format == "esp32s3":
        image_data[0x0000:0x0000+len(bootloader_bin)] = bootloader_bin
        image_data[0x8000:0x8000+len(table_bin)] = table_bin
    else:
        image_data[0x1000:0x1000+len(bootloader_bin)] = bootloader_bin
        image_data[0x8000:0x8000+len(table_bin)] = table_bin

    with open(output_file, "wb") as f:
        f.write(image_data)

    print("\nSaved image '%s' (%d bytes)\n" % (output_file, len(image_data)))


def clean_app(app):
    print("Cleaning up app '%s'..." % app)
    try:
        os.unlink(os.path.join(app, "sdkconfig"))
        os.unlink(os.path.join(app, "sdkconfig.old"))
    except:
        pass
    try:
        shutil.rmtree(os.path.join(app, "build"))
    except:
        pass
    print("Done.\n")


def build_app(app, device_type, with_profiling=False, no_networking=False, is_release=False):
    # To do: clean up if any of the flags changed since last build
    print("Building app '%s'" % app)
    args = [IDF_PY, "app"]
    args.append(f"-DRG_PROJECT_APP={app}")
    args.append(f"-DRG_PROJECT_VER={PROJECT_VER}")
    args.append(f"-DRG_BUILD_TARGET=RG_TARGET_{re.sub(r'[^A-Z0-9]', '_', device_type.upper())}")
    args.append(f"-DRG_BUILD_RELEASE={1 if is_release else 0}")
    args.append(f"-DRG_ENABLE_PROFILING={1 if with_profiling else 0}")
    args.append(f"-DRG_ENABLE_NETWORKING={0 if no_networking else 1}")
    with open("partitions.csv", "w") as f:
        f.write("# This table isn't used, it's just needed to avoid esp-idf build failures.\n")
        f.write("dummy, app, ota_0, 65536, 3145728\n")
    run(args, cwd=os.path.join(os.getcwd(), app))
    print("Done.\n")


def flash_app(app, port, baudrate=1152000):
    os.putenv("ESPTOOL_CHIP", os.getenv("IDF_TARGET", "auto"))
    os.putenv("ESPTOOL_BAUD", str(baudrate))
    os.putenv("ESPTOOL_PORT", port)
    if not os.path.exists("partitions.bin"):
        print("Reading device's partition table...")
        run([ESPTOOL_PY, "read_flash", "0x8000", "0x1000", "partitions.bin"], check=False)
        run([GEN_ESP32PART_PY, "partitions.bin"], check=False)
    app_bin = os.path.join(app, "build", app + ".bin")
    print(f"Flashing '{app_bin}' to port {port}")
    run([PARTTOOL_PY, "--partition-table-file", "partitions.bin", "write_partition", "--partition-name", app, "--input", app_bin])


def flash_image(image_file, port, baudrate=1152000):
    os.putenv("ESPTOOL_CHIP", os.getenv("IDF_TARGET", "auto"))
    os.putenv("ESPTOOL_BAUD", str(baudrate))
    os.putenv("ESPTOOL_PORT", port)
    print(f"Flashing image file '{image_file}' to {port}")
    run([ESPTOOL_PY, "write_flash", "--flash_size", "detect", "0x0", image_file])


def monitor_app(app, port, baudrate=115200):
    print(f"Starting monitor for app {app} on port {port}")
    elf_file = os.path.join(os.getcwd(), app, "build", app + ".elf")
    if os.path.exists(elf_file):
        run([IDF_MONITOR_PY, "--port", port, elf_file])
    else: # We must pass a file to idf_monitor.py but it doesn't have to be valid with -d
        run([IDF_MONITOR_PY, "--port", port, "-d", sys.argv[0]])


parser = argparse.ArgumentParser(description="Retro-Go build tool")
parser.add_argument(
# To do: Learn to use subcommands instead...
    "command", choices=["build-fw", "build-img", "release", "build", "clean", "flash", "monitor", "run", "profile", "install"],
)
parser.add_argument(
    "apps", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys())
)
parser.add_argument(
    "--target", default=DEFAULT_TARGET, choices=set(TARGETS), help="Device to target"
)
parser.add_argument(
    "--no-networking", action="store_const", const=True, help="Build without networking support"
)
parser.add_argument(
    "--port", default=DEFAULT_PORT, help="Serial port to use for flash and monitor"
)
parser.add_argument(
    "--baud", default=DEFAULT_BAUD, help="Serial baudrate to use for flashing"
)
parser.add_argument(
    "--fatsize", help="Add FAT storage partition of provided size (500K, 5M,...) to the built image."
)
args = parser.parse_args()

command = args.command
apps = [app for app in PROJECT_APPS.keys() if app in args.apps or "all" in args.apps]


if os.path.exists(f"components/retro-go/targets/{args.target}/sdkconfig"):
    os.putenv("SDKCONFIG_DEFAULTS", os.path.abspath(f"components/retro-go/targets/{args.target}/sdkconfig"))

if os.path.exists(f"components/retro-go/targets/{args.target}/env.py"):
    with open(f"components/retro-go/targets/{args.target}/env.py", "rb") as f:
        exec(f.read())

try:
    if command in ["build-fw", "build-img", "release", "install"] and "launcher" not in apps:
        print("\nWARNING: The launcher is mandatory for those apps and will be included!\n")
        apps.insert(0, "launcher")

    if command in ["clean", "release"]:
        print("=== Step: Cleaning ===\n")
        for app in apps:
            clean_app(app)

    if command in ["build", "build-fw", "build-img", "release", "run", "profile", "install"]:
        print("=== Step: Building ===\n")
        for app in apps:
            build_app(app, args.target, command == "profile", args.no_networking, command == "release")

    if command in ["build-fw", "release"]:
        print("=== Step: Packing ===\n")
        fw_format = os.getenv("FW_FORMAT")
        fw_file = ("%s_%s_%s.fw" % (PROJECT_NAME, PROJECT_VER, args.target)).lower()
        if fw_format in ["odroid", "esplay"]:
            build_firmware(fw_file, apps, os.getenv("FW_FORMAT"), args.fatsize)
        else:
            print("Device doesn't support fw format, try build-img!")

    if command in ["build-img", "release", "install"]:
        print("=== Step: Packing ===\n")
        img_file = ("%s_%s_%s.img" % (PROJECT_NAME, PROJECT_VER, args.target)).lower()
        build_image(img_file, apps, os.getenv("IMG_FORMAT", os.getenv("IDF_TARGET")), args.fatsize)

    if command in ["install"]:
        print("=== Step: Flashing entire image to device ===\n")
        # Should probably show a warning here and ask for confirmation...
        img_file = ("%s_%s_%s.img" % (PROJECT_NAME, PROJECT_VER, args.target)).lower()
        flash_image(img_file, args.port, args.baud)

    if command in ["flash", "run", "profile"]:
        print("=== Step: Flashing ===\n")
        try: os.unlink("partitions.bin")
        except: pass
        for app in apps:
            flash_app(app, args.port, args.baud)

    if command in ["monitor", "run", "profile"]:
        print("=== Step: Monitoring ===\n")
        monitor_app(apps[0] if len(apps) else "none", args.port)

    print("All done!")

except KeyboardInterrupt as e:
    exit("\n")

except Exception as e:
    exit(f"\nTask failed: {e}")
