#!/usr/bin/env python
import argparse
import hashlib
import subprocess
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

try:
    sys.path.append(os.path.join(os.environ["IDF_PATH"], "components", "partition_table"))
    import parttool
except:
    pass

DEFAULT_TARGET = os.getenv("RG_TOOL_TARGET", "odroid-go")
DEFAULT_BAUD = os.getenv("RG_TOOL_BAUD", "1152000")
DEFAULT_PORT = os.getenv("RG_TOOL_PORT", "COM3")
PROJECT_NAME = os.getenv("PROJECT_NAME", os.path.basename(os.getcwd()).title())
PROJECT_ICON = os.getenv("PROJECT_ICON", "icon.raw")
PROJECT_APPS = os.getenv("PROJECT_APPS", "partitions.csv")
try:
    PROJECT_VER = os.getenv("PROJECT_VER") or subprocess.check_output(
        "git describe --tags --abbrev=5 --dirty --always", shell=True
    ).decode().rstrip()
except:
    PROJECT_VER = "unknown"

if type(PROJECT_APPS) is str: # Assume it's a partitions.csv, we must then parse it
    csv = re.compile(r"^\s*([^#]+)\s*,\s*(app|0)\s*,\s*(.+)\s*,\s*(.+)\s*,\s*(.+)\s*(#.+)?$")
    filename = PROJECT_APPS
    PROJECT_APPS = {}
    try:
        with open(filename, "r") as f:
            for line in f:
                if m := csv.match(line):
                    PROJECT_APPS[m[1]] = [0, int(m[5], base=0), m[2], m[3]]
    except:
        exit("Failed reading partitions from '%s' (PROJECT_APPS)." % filename)

if not PROJECT_APPS:
    exit("No subprojects defined. Are you running from the project's directory?")

if not os.getenv("IDF_PATH"):
    exit("IDF_PATH is not defined. Are you running inside esp-idf environment?")


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


def build_firmware(targets, shrink=False, device_type=None):
    args = [
        sys.executable,
        "tools/mkfw.py",
        ("%s_%s_%s.fw" % (PROJECT_NAME, PROJECT_VER, device_type)).lower(),
        ("%s %s" % (PROJECT_NAME, PROJECT_VER)),
        PROJECT_ICON
    ]

    if device_type in ["mrgc-g32", "esplay"]:
        args.append("--esplay")

    for target in targets:
        part = PROJECT_APPS[target]
        size = 0 if shrink else part[1]
        args += [str(0), str(part[0]), str(size), target, os.path.join(target, "build", target + ".bin")]

    commandline = ' '.join(shlex.quote(arg) for arg in args[1:]) # shlex.join()
    print("Building firmware: %s\n" % commandline)
    subprocess.run(args, check=True)


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
    subprocess.run("idf.py app", shell=True, check=True, cwd=os.path.join(os.getcwd(), target))

    try:
        print("\nPatching esp_image_header_t to skip sha256 on boot... ", end="")
        with open(os.path.join(target, "build", target + ".bin"), "r+b") as fp:
            fp.seek(23)
            fp.write(b"\0")
        print("done!\n")
    except: # don't really care if that fails
        print("failed!\n")
        pass


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
    try:
        pt = parttool.ParttoolTarget(args.port, args.baud)
    except:
        exit("Failed to read device's partition table!")
    try:
        for app in apps:
            print("Flashing app '%s'" % app)
            pt.write_partition(parttool.PartitionName(app), os.path.join(app, "build", app + ".bin"))
    except Exception as e:
        print("Error: {}".format(e))
        if "does not exist" in str(e):
            print("This indicates that the partition table on your device is incorrect.")
            print("Make sure you've installed a recent retro-go-*.fw!")
        exit("Task failed.")

if command in ["monitor", "run"]:
    print("=== Step: Monitoring ===\n")
    if len(apps) == 1:
        monitor_app(apps[0], args.port)
    else:
        monitor_app("dummy", args.port)

print("All done!")
