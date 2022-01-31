#!/usr/bin/env python
import argparse
import math
import subprocess
import hashlib
import shutil
import shlex
import time
import sys
import re
import os

try:
    import serial
except:
    serial = None

DEFAULT_TARGET = os.getenv("RG_TOOL_TARGET", "odroid-go")
DEFAULT_OFFSET = os.getenv("RG_TOOL_OFFSET", "0x100000")
DEFAULT_BAUD = os.getenv("RG_TOOL_BAUD", "1152000")
DEFAULT_PORT = os.getenv("RG_TOOL_PORT", "COM3")

PROJECT_PATH = os.getcwd() if os.path.exists("rg_config.py") else sys.path[0]
IDF_PATH = os.getenv("IDF_PATH")

if not IDF_PATH:
    exit("IDF_PATH is not defined.")

os.chdir(PROJECT_PATH)

try:
    with open("rg_config.py", "rb") as f:
        exec(f.read())
except:
    PROJECT_NAME = os.path.basename(PROJECT_PATH).title()
    PROJECT_VER = subprocess.check_output("git describe --dirty --always", shell=True).decode().rstrip()
    PROJECT_TILE = "icon.raw"
    PROJECT_APPS = {} # TO DO: discover subprojects automatically

symbols_cache = dict()


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


def build_firmware(targets, shrink=False, device_type=None):
    args = [
        sys.executable,
        "tools/mkfw.py",
        ("%s_%s_%s.fw" % (PROJECT_NAME, PROJECT_VER, device_type)).lower(),
        ("%s %s" % (PROJECT_NAME, PROJECT_VER)),
        PROJECT_TILE
    ]

    if device_type in ["mrgc-g32", "esplay"]:
        args.append("--esplay")

    for target in targets:
        part = PROJECT_APPS[target]
        size = 0 if shrink else part[1]
        args += [str(0), str(part[0]), str(size), target, os.path.join(target, "build", target + ".bin")]

    commandline = ' '.join(shlex.quote(arg) for arg in args[1:]) # shlex.join()
    print("Building firmware: %s\n" % commandline)
    subprocess.run(args, check=True, cwd=PROJECT_PATH)


def clean_app(target):
    print("Cleaning up app '%s'..." % target)
    try:
        os.unlink(os.path.join(target, "sdkconfig"))
        os.unlink(os.path.join(target, "sdkconfig.old"))
    except:
        pass
    try:
        shutil.rmtree(os.path.join(target, "build"))
    except:
        pass
    print("Done.\n")


def build_app(target, build_type=None, with_netplay=False, build_target=None):
    # To do: clean up if any of the flags changed since last build
    print("Building app '%s'" % target)
    os.putenv("ENABLE_PROFILING", "1" if build_type == "profile" else "0")
    os.putenv("ENABLE_NETPLAY", "1" if with_netplay else "0")
    os.putenv("PROJECT_VER", PROJECT_VER)
    if build_target:
        os.putenv("RG_TARGET", re.sub(r'[^A-Z0-9]', '_', build_target.upper()))
    subprocess.run("idf.py app", shell=True, check=True, cwd=os.path.join(PROJECT_PATH, target))

    try:
        print("\nPatching esp_image_header_t to skip sha256 on boot... ", end="")
        with open(os.path.join(target, "build", target + ".bin", "r+b")) as fp:
            fp.seek(23)
            fp.write(b"\0")
        print("done!\n")
    except: # don't really care if that fails
        print("failed!\n")
        pass


def find_app(target, offset, app_offset=None):
    if app_offset != None:
        return int(str(app_offset), 0)
    offset = int(str(offset), 0)
    for key, value in PROJECT_APPS.items():
        if key == target:
            return offset
        offset += math.ceil(value[1] / 0x10000) * 0x10000


def flash_app(target, port, offset, baud):
    print("Flashing app '%s' at offset %s" % (target, hex(offset)))
    subprocess.run([
        sys.executable,
        os.path.join(IDF_PATH, "components", "esptool_py", "esptool", "esptool.py"),
        "--chip", "esp32",
        "--port", port,
        "--baud", baud,
        "--before", "default_reset",
        "write_flash",
        "--flash_mode", "qio",
        "--flash_freq", "80m",
        "--flash_size", "detect",
        hex(offset),
        os.path.join(target, "build", target + ".bin"),
    ], check=True)
    print("Done.\n")


def monitor_app(target, port, baudrate=115200):
    if not serial:
        exit("serial module not installed. You can try running 'pip install pyserial'.")
    print("Starting monitor for app '%s'" % target)
    mon = serial.Serial(port, baudrate=baudrate, timeout=0)
    elf = os.path.join(target, "build", target + ".elf")

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
    "command", choices=["build-fw", "release", "build", "clean", "flash", "monitor", "run"],
)
parser.add_argument(
    "apps", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys())
)
parser.add_argument(
    "--shrink", action="store_const", const=True, help="Reduce partition size where possible"
)
parser.add_argument(
    "--target", default=DEFAULT_TARGET, choices=["odroid-go", "esp32s2", "mrgc-g32", "qtpy-gamer"], help="Device to target"
)
parser.add_argument(
    "--build-type", default="release", choices=["release", "debug", "profile"], help="Build type"
)
parser.add_argument(
    "--with-netplay", action="store_const", const=True, help="Build with netplay enabled"
)
parser.add_argument(
    "--port", default=DEFAULT_PORT, help="Serial port to use for flash and monitor"
)
parser.add_argument(
    "--baud", default=DEFAULT_BAUD, help="Serial baudrate to use for flashing"
)
parser.add_argument(
    "--offset", default=DEFAULT_OFFSET, help="Flash offset where %s is installed" % PROJECT_NAME,
)
parser.add_argument(
    "--app-offset", default=None, help="Absolute offset where the app will be flashed (ignores partition table)"
)
args = parser.parse_args()


command = args.command
apps = args.apps if "all" not in args.apps else PROJECT_APPS.keys()


if command in ["clean", "release"]:
    print("=== Step: Cleaning ===\n")
    for app in apps:
        clean_app(app)

if command in ["build", "build-fw", "release", "run"]:
    print("=== Step: Building ===\n")
    for app in apps:
        build_app(app, args.build_type, args.with_netplay, args.target)

if command in ["build-fw", "release"]:
    print("=== Step: Packing ===\n")
    build_firmware(apps, args.shrink, args.target)

if command in ["flash", "run"]:
    print("=== Step: Flashing ===\n")
    for app in apps:
        flash_app(app, args.port, find_app(app, args.offset, args.app_offset), args.baud)

if command in ["monitor", "run"]:
    print("=== Step: Monitoring ===\n")
    if len(apps) == 1:
        monitor_app(apps[0], args.port)
    else:
        monitor_app("dummy", args.port)

print("All done!")
