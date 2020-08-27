#!/usr/bin/env python
from os import path, getenv, getcwd, chdir
import sys, math, subprocess, argparse, shutil

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


def build_firmware(targets):
    chdir(PRJ_PATH)
    args = [
        sys.executable,
        "tools/mkfw.py",
        ("%s_%s.fw" % (PROJECT_NAME, PROJECT_VER)).lower(),
        ("%s %s" % (PROJECT_NAME, PROJECT_VER)),
        PROJECT_TILE
    ]
    for target in targets:
        part = PROJECT_APPS[target]
        args += [str(0), str(part[0]), str(part[1]), target, path.join(target, "build", target + ".bin")]

    subprocess.run(args, check=True)


def clean_app(target):
    print("Cleaning up app '%s'..." % target)
    try:  # idf.py fullclean
        shutil.rmtree(path.join(PRJ_PATH, target, "build"))
        print("Done.\n")
    except FileNotFoundError:
        print("Nothing to do.\n")


def build_app(target, use_make):
    print("Building app '%s'" % target)
    chdir(path.join(PRJ_PATH, target))
    if use_make:
        subprocess.run("make -j 6 app", shell=True, check=True)
    else:
        subprocess.run("idf.py app", shell=True, check=True)
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
        path.join(IDF_PATH, "components", "esptool_py", "esptool", "esptool.py"),
        "--chip", "esp32",
        "--port", port,
        "--baud", "1152000",
        "--before", "default_reset",
        "write_flash",
        "--flash_mode", "qio",
        "--flash_freq", "80m",
        "--flash_size", "detect",
        hex(offset),
        path.join(PRJ_PATH, target, "build", target + ".bin"),
    ], check=True)
    print("Done.\n")


def monitor_app(target, port):
    print("Starting monitor for app '%s'" % target)
    subprocess.run([
        sys.executable,
        path.join(IDF_PATH, "tools", "idf_monitor.py"),
        "--port", port,
        path.join(PRJ_PATH, target, "build", target + ".elf"),
    ])


parser = argparse.ArgumentParser(description="Retro-Go build tool")
parser.add_argument(
    "command", choices=["build", "clean", "release", "mkfw", "flash", "flashmon", "monitor"],
)
parser.add_argument(
    "targets", nargs="*", default="all", choices=["all"] + list(PROJECT_APPS.keys())
)
parser.add_argument(
    "--use-make", action="store_const", const=True, help="Use legacy make build system"
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
targets = args.targets if not "all" in args.targets else PROJECT_APPS.keys()

if command == "clean" or command == "release":
    for target in targets:
        clean_app(target)

if command == "build" or command == "release":
    for target in targets:
        build_app(target, args.use_make)

if command == "release" or command == "mkfw":
    build_firmware(targets)

if command == "flash" or command == "flashmon":
    for target in targets:
        offset = find_app(target, args.offset, args.app_offset)
        flash_app(target, args.port, offset)

if command == "monitor" or command == "flashmon":
    monitor_app(targets[0], args.port)


print("All done!")
