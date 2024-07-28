#!/usr/bin/env python3
import argparse
import subprocess
import shutil
import glob
import math
import sys
import re
import os

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

if os.path.exists("rg_config.py"):
    with open("rg_config.py", "rb") as f:
        exec(f.read())
# else: something like
#     for file in glob(*/CMakeLists.txt):
#       PROJECT_APPS[basename(dirname(file))] = [0, 0, 0, 0]



def run(cmd, cwd=None, check=True):
    print(f"Running command: {' '.join(cmd)}")
    return subprocess.run(cmd, shell=os.name == 'nt', cwd=cwd, check=check)


def build_firmware(apps, device_type, fw_format="odroid-go"):
    print("Building firmware with: %s\n" % " ".join(apps))
    args = [
        os.path.join("tools", "mkfw.py"),
        ("%s_%s_%s.fw" % (PROJECT_NAME, PROJECT_VER, device_type)).lower(),
        ("%s %s" % (PROJECT_NAME, PROJECT_VER)),
        PROJECT_ICON
    ]

    if fw_format == "esplay":
        args.append("--esplay")

    for app in apps:
        part = PROJECT_APPS[app]
        args += [str(part[0]), str(part[1]), str(part[2]), app, os.path.join(app, "build", app + ".bin")]

    run(args)


def build_image(apps, device_type, img_format="esp32"):
    print("Building image with: %s\n" % " ".join(apps))
    image_file = ("%s_%s_%s.img" % (PROJECT_NAME, PROJECT_VER, device_type)).lower()
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

    print("Generating partition table...")
    with open("partitions.csv", "w") as f:
        f.write("\n".join(table_csv))
    run(["gen_esp32part.py", "partitions.csv", "partitions.bin"])
    with open("partitions.bin", "rb") as f:
        table_bin = f.read()

    print("Building bootloader...")
    run(["idf.py", "bootloader"], cwd=os.path.join(os.getcwd(), list(apps)[0]))
    with open(os.path.join(os.getcwd(), list(apps)[0], "build", "bootloader", "bootloader.bin"), "rb") as f:
        bootloader_bin = f.read()

    if img_format == "esp32s3":
        image_data[0x0000:0x0000+len(bootloader_bin)] = bootloader_bin
        image_data[0x8000:0x8000+len(table_bin)] = table_bin
    else:
        image_data[0x1000:0x1000+len(bootloader_bin)] = bootloader_bin
        image_data[0x8000:0x8000+len(table_bin)] = table_bin

    with open(image_file, "wb") as f:
        f.write(image_data)

    print("\nSaved image '%s' (%d bytes)\n" % (image_file, len(image_data)))


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
    args = ["idf.py", "app"]
    args.append(f"-DRG_BUILD_VERSION={PROJECT_VER}")
    args.append(f"-DRG_BUILD_TARGET={re.sub(r'[^A-Z0-9]', '_', device_type.upper())}")
    args.append(f"-DRG_BUILD_TYPE={1 if is_release else 0}")
    args.append(f"-DRG_ENABLE_PROFILING={1 if with_profiling else 0}")
    args.append(f"-DRG_ENABLE_NETWORKING={0 if no_networking else 1}")
    run(args, cwd=os.path.join(os.getcwd(), app))
    print("Done.\n")


def flash_app(app, port, baudrate=1152000):
    os.putenv("ESPTOOL_CHIP", os.getenv("IDF_TARGET", "auto"))
    os.putenv("ESPTOOL_BAUD", baudrate)
    os.putenv("ESPTOOL_PORT", port)
    if not os.path.exists("partitions.bin"):
        print("Reading device's partition table...")
        run(["esptool.py", "read_flash", "0x8000", "0x1000", "partitions.bin"], check=False)
        run(["gen_esp32part.py", "partitions.bin"], check=False)
    app_bin = os.path.join(app, "build", app + ".bin")
    print(f"Flashing '{app_bin}' to port {port}")
    run(["parttool.py", "--partition-table-file", "partitions.bin", "write_partition", "--partition-name", app, "--input", app_bin])


def monitor_app(app, port, baudrate=115200):
    print(f"Starting monitor for app {app} on port {port}")
    elf_file = os.path.join(os.getcwd(), app, "build", app + ".elf")
    if os.path.exists(elf_file):
        args = ["idf_monitor.py", "--port", port, elf_file]
    else: # We must pass a file to idf_monitor.py but it doesn't have to be valid with -d
        args = ["idf_monitor.py", "--port", port, "-d", sys.argv[0]]
    run(args)


parser = argparse.ArgumentParser(description="Retro-Go build tool")
parser.add_argument(
# To do: Learn to use subcommands instead...
    "command", choices=["build-fw", "build-img", "release", "build", "clean", "flash", "monitor", "run", "profile"],
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
args = parser.parse_args()

command = args.command
apps = [app for app in PROJECT_APPS.keys() if app in args.apps or "all" in args.apps]


if not os.getenv("IDF_PATH"):
    exit("IDF_PATH is not defined. Are you running inside esp-idf environment?")

if os.path.exists(f"components/retro-go/targets/{args.target}/sdkconfig"):
    os.putenv("SDKCONFIG_DEFAULTS", os.path.abspath(f"components/retro-go/targets/{args.target}/sdkconfig"))

if os.path.exists(f"components/retro-go/targets/{args.target}/env.py"):
    with open(f"components/retro-go/targets/{args.target}/env.py", "rb") as f:
        exec(f.read())


try:
    if command in ["build-fw", "build-img", "release"] and "launcher" not in apps:
        print("\nWARNING: The launcher is mandatory for those apps and will be included!\n")
        apps.insert(0, "launcher")

    if command in ["clean", "release"]:
        print("=== Step: Cleaning ===\n")
        for app in apps:
            clean_app(app)

    if command in ["build", "build-fw", "build-img", "release", "run", "profile"]:
        print("=== Step: Building ===\n")
        for app in apps:
            build_app(app, args.target, command == "profile", args.no_networking, command == "release")

    if command in ["build-fw", "release"]:
        print("=== Step: Packing ===\n")
        build_firmware(apps, args.target, os.getenv("FW_FORMAT"))

    if command in ["build-img", "release"]:
        print("=== Step: Packing ===\n")
        build_image(apps, args.target, os.getenv("IMG_FORMAT", os.getenv("IDF_TARGET")))

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
