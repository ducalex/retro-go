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


def flags_changed():
    return os.getenv("RGP_PREV")

def analyze_profile(frames):
    flatten = False
    tree = dict()

    for caller, callee, num_calls, run_time in frames:
        branch = callee.hash if flatten else caller.hash
        if branch not in tree:
            tree[branch] = dict()
        if callee.hash not in tree[branch]:
            tree[branch][callee.hash] = [caller, callee, num_calls, run_time]
        else:
            tree[branch][callee.hash][2] += num_calls
            tree[branch][callee.hash][3] += run_time

    tree_sorted = sorted(tree.items(), key=lambda x: list(x[1].values())[0][3], reverse=True)

    for key, branch in tree_sorted:
        branch_leafs = list(branch.values())
        if branch_leafs[0][3] >= 1000: # 1ms or more
            print("%-32s%dms" % (branch_leafs[0][1].name, branch_leafs[0][3] / 1000))
            for caller, callee, num_calls, run_time in branch_leafs[1:]:
                print("%-32s%dms" % ("    " + callee.name, run_time / 1000))
            if not flatten:
                print()


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


def build_app(target, use_make=False, with_debugging=False, with_profiling=False):
    # To do: clean up if any of the flags changed since last build
    print("Building app '%s'" % target)
    os.chdir(os.path.join(PRJ_PATH, target))
    os.putenv("ENABLE_PROFILING", "1" if with_profiling else "0")
    os.putenv("ENABLE_DEBUGGING", "1" if with_debugging else "0")
    if use_make:
        subprocess.run(["make", "-j", "6", "app"], shell=True, check=True)
    else:
        subprocess.run(["idf.py", "app"], shell=True, check=True)
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
        if mon.in_waiting > 0:
            byte = mon.read()
            sys.stdout.buffer.write(byte) # byte.decode()
            if byte == b"\n":
                line = line_bytes.decode(errors="ignore").rstrip()
                line_bytes = b''
                # check for profile data
                if line.startswith("RGP:BEGIN"):
                    profile_frames.clear()
                elif line.startswith("RGP:END"):
                    analyze_profile(profile_frames)
                elif line.startswith("RGP:FRAME"):
                    m = re.match(r"^RGP:FRAME ([x0-9a-f]+)\s([x0-9a-f]+)\s(\d+)\s(\d+)", line)
                    if m:
                        profile_frames.append([
                            find_symbol(elf, m.group(1)),
                            find_symbol(elf, m.group(2)),
                            int(m.group(3)),
                            int(m.group(4)),
                        ])
                # check for symbol addresses
                else:
                    for addr in re.findall(r"0x4[0-9a-fA-F]{7}", line):
                        symbol = find_symbol(elf, addr)
                        if symbol and "??:" not in symbol.source:
                            print("\033[0;33m%s\033[0m" % symbol)
            else:
                line_bytes += byte
        else:
            sys.stdout.flush()
            time.sleep(0.010)


parser = argparse.ArgumentParser(description="Retro-Go build tool")
parser.add_argument(
    "command", choices=["build-fw", "build", "clean", "flash", "monitor", "run"],
)
parser.add_argument(
    "targets", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys())
)
parser.add_argument(
    "--use-make", action="store_const", const=True, help="Use legacy make build system"
)
parser.add_argument(
    "--netplay", action="store_const", const=True, help="Build with netplay enabled"
)
parser.add_argument(
    "--profile", action="store_const", const=True, help="Build with profiling enabled"
)
parser.add_argument(
    "--debug", action="store_const", const=True, help="Build with debugging enabled"
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
        build_app(target, args.use_make, args.debug, args.profile)
    build_firmware(targets)

if command == "build":
    for target in targets:
        build_app(target, args.use_make, args.debug, args.profile)

if command == "clean":
    for target in targets:
        clean_app(target)

if command == "flash":
    for target in targets:
        flash_app(target, args.port, find_app(target, args.offset, args.app_offset))

if command == "monitor":
    monitor_app(targets[0], args.port)

if command == "run":
    build_app(targets[0], args.use_make, args.debug, args.profile)
    flash_app(targets[0], args.port, find_app(targets[0], args.offset, args.app_offset))
    monitor_app(targets[0], args.port)


print("All done!")
