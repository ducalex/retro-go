#!/usr/bin/env python3
import argparse
import hashlib
import subprocess
import shutil
import glob
import time
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


class Symbol:
    def __init__(self, address, name, source="??:?", inlined=None):
        self.address = int(str(address), 0)
        self.name = name
        self.source = os.path.normpath(source)
        self.inlined = inlined
        self.hash = hashlib.sha1(bytes(self.source + self.name, "UTF-8")).hexdigest()

    def __str__(self):
        text = "0x%x: %s at %s" % (self.address, self.name, self.source)
        if self.inlined:
            text += "\ninlined by %s" % str(self.inlined)
        return text.replace("\n", "\n  ")


class CallBranch:
    def __init__(self, name, parent=None):
        self.name = name
        self.parent = parent
        self.run_time = 0;
        self.children = dict()

    def add_frame(self, caller, callee, num_calls, run_time):
        if callee.hash not in self.children:
            self.children[callee.hash] = [caller, callee, num_calls, run_time]
        else:
            self.children[callee.hash][2] += num_calls
            self.children[callee.hash][3] += run_time
        self.run_time += run_time


def debug_print(text):
    print("\033[0;33m%s\033[0m" % text)


def find_symbol(elf_file, addr):
    try:
        if addr not in symbols_cache:
            symbols_cache[addr] = Symbol(0, "??")
            out = subprocess.check_output(["xtensa-esp32-elf-addr2line", "-ifCe", elf_file, addr], shell=True)
            lines = out.decode().rstrip().splitlines()
            if len(lines) > 2:
                symbols_cache[addr] = Symbol(addr, lines[0], lines[1], Symbol(0, lines[2], lines[3]))
            elif len(lines) == 2:
                symbols_cache[addr] = Symbol(addr, lines[0], lines[1])
    except:
        pass
    return symbols_cache[addr]
symbols_cache = dict()


def analyze_profile(frames):
    flatten = True # False is currently not working correctly
    tree = dict()

    for caller, callee, num_calls, run_time in frames:
        branch = '*' if flatten else caller.name + "@" + os.path.basename(caller.source)
        if branch not in tree:
            tree[branch] = CallBranch(branch, caller)
        tree[branch].add_frame(caller, callee, num_calls, run_time)

    tree_sorted = sorted(tree.values(), key=lambda x: x.run_time, reverse=True)

    for branch in tree_sorted:
        if branch.run_time < 100_000:
            continue

        debug_print("%-68s %dms" % (branch.name, branch.run_time / 1000))
        children = sorted(branch.children.values(), key=lambda x: x[3], reverse=True)

        for caller, callee, num_calls, run_time in children:
            if run_time < 10_000:
                continue
            debug_print("    %-32s %-20s %-10d %dms"
                % (callee.name, os.path.basename(callee.source), num_calls, run_time / 1000))

        debug_print("")


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
    try:
        import serial
        mon = serial.Serial(port, baudrate=baudrate, timeout=0)
        elf = os.path.join(app, "build", app + ".elf")
    except:
        exit("Failed to load the serial module. You can try running 'pip install pyserial'.")

    mon.setDTR(False)
    mon.setRTS(False)

    # To do: detect ctrl+r ctrl+c etc

    profile_frames = list()

    line_bytes = b''
    while 1:
        if mon.in_waiting == 0:
            sys.stdout.flush()
            time.sleep(0.010)
            continue

        byte = mon.read()

        if byte != b"\n":
            sys.stdout.buffer.write(byte) # byte.decode()
            line_bytes += byte
        else:
            line = line_bytes.decode(errors="ignore").rstrip()
            line_bytes = b''

            # check for debug data meant to be analyzed, not displayed
            m = re.match(r"^RGD:([A-Z0-9]+):([A-Z0-9]+)\s*(.*)$", line)
            if m:
                rg_debug_ns  = m.group(1)
                rg_debug_cmd = m.group(2)
                rg_debug_arg = m.group(3)

                sys.stdout.buffer.write(b"\r") # Clear the line

                if rg_debug_ns == "PROF":
                    if rg_debug_cmd == "BEGIN":
                        profile_frames.clear()
                    if rg_debug_cmd == "END":
                        analyze_profile(profile_frames)
                    if rg_debug_cmd == "DATA":
                        m = re.match(r"([x0-9a-f]+)\s([x0-9a-f]+)\s(\d+)\s(\d+)", rg_debug_arg)
                        if m:
                            profile_frames.append([
                                find_symbol(elf, m.group(1)),
                                find_symbol(elf, m.group(2)),
                                int(m.group(3)),
                                int(m.group(4)),
                            ])
                    continue

            sys.stdout.buffer.write(b"\n")

            # check for symbol addresses
            for addr in re.findall(r"0x4[0-9a-fA-F]{7}", line):
                symbol = find_symbol(elf, addr)
                if symbol and "??:" not in symbol.source:
                    debug_print(symbol)


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
        monitor_app(apps[0] if len(apps) else "dummy", args.port)

    print("All done!")

except KeyboardInterrupt as e:
    exit("\n")

except Exception as e:
    exit(f"\nTask failed: {e}")
