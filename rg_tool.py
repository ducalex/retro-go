#!/usr/bin/env python
import argparse
import math
import subprocess
import hashlib
import shutil
import serial
import time
import sys
import re
import os

IDF_PATH = os.getenv("IDF_PATH")
PRJ_PATH = sys.path[0]  # os.getcwd()

if not IDF_PATH:
    exit("IDF_PATH is not defined.")

def read_file(filepath):
    with open(filepath, "rb") as f: return f.read()
def shell_exec(cmd):
    return subprocess.check_output(cmd, shell=True).decode().rstrip()
def debug_print(text):
    print("\033[0;33m%s\033[0m" % text)

os.chdir(PRJ_PATH)

exec(read_file("rg_config.py"))


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


def find_symbol(elf_file, addr):
    try:
        if addr not in symbols_cache:
            symbols_cache[addr] = Symbol(0, "??")
            lines = shell_exec(["xtensa-esp32-elf-addr2line", "-ifCe", elf_file, addr]).splitlines()
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


def build_firmware(targets):
    os.chdir(PRJ_PATH)
    args = [
        sys.executable,
        "tools/mkfw.py",
        ("%s_%s.fw" % (PROJECT_NAME, PROJECT_VER)).lower(),
        ("%s %s" % (PROJECT_NAME, PROJECT_VER)),
        PROJECT_TILE
    ]
    for target in targets:
        part = PROJECT_APPS[target]
        args += [str(0), str(part[0]), str(part[1]), target, os.path.join(target, "build", target + ".bin")]

    subprocess.run(args, check=True)


def clean_app(target):
    print("Cleaning up app '%s'..." % target)
    try:  # idf.py fullclean
        shutil.rmtree(os.path.join(PRJ_PATH, target, "build"))
        print("Done.\n")
    except FileNotFoundError:
        print("Nothing to do.\n")


def build_app(target, with_debugging=False, with_profiling=False, with_netplay=False):
    # To do: clean up if any of the flags changed since last build
    print("Building app '%s'" % target)
    os.chdir(os.path.join(PRJ_PATH, target))
    os.putenv("ENABLE_PROFILING", "1" if with_profiling else "0")
    os.putenv("ENABLE_DEBUGGING", "1" if with_debugging else "0")
    os.putenv("ENABLE_NETPLAY", "1" if with_netplay else "0")
    subprocess.run(["idf.py", "app"], shell=True, check=True)

    print("Patching esp_image_header_t to skip sha256 on boot...")
    with open("build/" + target + ".bin", "r+b") as fp:
        fp.seek(23)
        fp.write(b"\0")

    print("Done.\n")


def find_app(target, offset, app_offset=None):
    if app_offset != None:
        return int(str(app_offset), 0)
    offset = int(str(offset), 0)
    for key, value in PROJECT_APPS.items():
        if key == target:
            return offset
        offset += math.ceil(value[1] / 0x10000) * 0x10000


def flash_app(target, port, offset=0x100000):
    print("Flashing app '%s' at offset %s" % (target, hex(offset)))
    subprocess.run([
        sys.executable,
        os.path.join(IDF_PATH, "components", "esptool_py", "esptool", "esptool.py"),
        "--chip", "esp32",
        "--port", port,
        "--baud", "1152000",
        "--before", "default_reset",
        "write_flash",
        "--flash_mode", "qio",
        "--flash_freq", "80m",
        "--flash_size", "detect",
        hex(offset),
        os.path.join(PRJ_PATH, target, "build", target + ".bin"),
    ], check=True)
    print("Done.\n")


def monitor_app(target, port, baudrate=115200):
    print("Starting monitor for app '%s'" % target)
    mon = serial.Serial(port, baudrate=baudrate, timeout=0)
    elf = os.path.join(PRJ_PATH, target, "build", target + ".elf")

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
    "command", choices=["build-fw", "build", "clean", "flash", "monitor", "run"],
)
parser.add_argument(
    "targets", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys())
)
parser.add_argument(
    "--compact-fw", action="store_const", const=True, help="Ignore the partition sizes set in rg_config.py when building .fw"
)
parser.add_argument(
    "--with-netplay", action="store_const", const=True, help="Build with netplay enabled"
)
parser.add_argument(
    "--with-profiling", action="store_const", const=True, help="Build with profiling enabled"
)
parser.add_argument(
    "--with-debugging", action="store_const", const=True, help="Build with debugging enabled"
)
parser.add_argument(
    "--port", default="COM3", help="Serial port to use for flash and monitor"
)
parser.add_argument(
    "--offset", default="0x100000", help="Flash offset where %s is installed" % PROJECT_NAME,
)
parser.add_argument(
    "--app-offset", default=None, help="Absolute offset where target will be flashed"
)
args = parser.parse_args()


command = args.command
targets = args.targets if "all" not in args.targets else PROJECT_APPS.keys()


if command == "build-fw":
    for target in targets:
        clean_app(target)
        build_app(target, args.with_debugging, args.with_profiling, args.with_netplay)
    build_firmware(targets)

if command == "build":
    for target in targets:
        build_app(target,args.with_debugging, args.with_profiling, args.with_netplay)

if command == "clean":
    for target in targets:
        clean_app(target)

if command == "flash":
    for target in targets:
        flash_app(target, args.port, find_app(target, args.offset, args.app_offset))

if command == "monitor":
    monitor_app(targets[0], args.port)

if command == "run":
    build_app(targets[0], args.with_debugging, args.with_profiling, args.with_netplay)
    flash_app(targets[0], args.port, find_app(targets[0], args.offset, args.app_offset))
    monitor_app(targets[0], args.port)


print("All done!")
