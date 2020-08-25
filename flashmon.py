#!/usr/bin/env python
import sys, os, subprocess, argparse
from projects import PROJECTS

os.chdir(sys.path[0])

if not os.getenv("IDF_PATH"):
    exit("IDF_PATH is not defined.")

parser = argparse.ArgumentParser(description='Retro-Go flash and monitor')
parser.add_argument('project', choices=PROJECTS.keys())
parser.add_argument('--port', default="COM3", help="Serial port to use")
parser.add_argument('--offset', type=lambda x: int(x,0), default=0x100000, help="Flash offset where Retro-Go is installed")
args = parser.parse_args()

if args.offset < 0x100000:
    parser.error("Offset doesn't seem right.")

ESP_TOOL = os.getenv("IDF_PATH") + "/components/esptool_py/esptool/esptool.py"
IDF_TOOL = os.getenv("IDF_PATH") + "/tools/idf_monitor.py" # or idf.py monitor -p %COMPORT%

APP_OFFSET = args.offset

for key, value in PROJECTS.items():
    if key == args.project: break
    APP_OFFSET += value[1] # To do: Make sure we are on 64k offset

os.chdir(sys.path[0] + "/" + args.project + "/build")

# Flash app
subprocess.run([
    sys.executable,
    ESP_TOOL,
    "--chip", "esp32",
    "--port", args.port,
    "--baud", "1152000",
    "--before", "default_reset",
    "write_flash",
    "--flash_mode", "qio",
    "--flash_freq", "80m",
    "--flash_size", "detect",
    hex(APP_OFFSET),
    args.project + ".bin"
], check=True)

# Start monitor
subprocess.run([sys.executable, IDF_TOOL, "--port", args.port, args.project + ".elf"])
