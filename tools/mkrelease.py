#!/usr/bin/env python3
import glob
import os
import shutil

OUTPUT_DIR = "build/release"
TARGETS = [os.path.basename(t[0:-1]) for t in glob.glob("components/retro-go/targets/*/")]

shutil.rmtree(OUTPUT_DIR, ignore_errors=True)
os.makedirs(OUTPUT_DIR, exist_ok=True)

for target in TARGETS:
    print(f"Building {target}")
    os.system(f"python rg_tool.py --target={target} release")
    for f in glob.glob(f"retro-go_*_{target}.*"):
        os.rename(f, f"{OUTPUT_DIR}/{f}")

os.system(f"python rg_tool.py clean")
