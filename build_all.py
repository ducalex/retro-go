#!/usr/bin/env python
import sys, os, subprocess, argparse
from projects import PROJECTS

os.chdir(sys.path[0])

if not os.getenv("IDF_PATH"):
    exit("IDF_PATH is not defined.")

if "make" in sys.argv:
    BUILD_CMD = "make -j 6 app"
    CLEAN_CMD = "make clean"
else:
    BUILD_CMD = "idf.py app"
    CLEAN_CMD = "idf.py fullclean"

for key in PROJECTS.keys():
    print("Building %s\n" % key)
    os.chdir(sys.path[0] + "/" + key)
    subprocess.run(CLEAN_CMD, shell=True, check=True)
    subprocess.run(BUILD_CMD, shell=True, check=True)
    print("\n************************************\n")

print("All done!")
