# This file is injected late into rg_tool.py, you can run arbitrary python code here
# For example override python variables or set environment variables with os.putenv

# Espressif chip in the device
IDF_TARGET = "esp32p4"
# .fw file format, if supported by the device
# FW_FORMAT = "odroid"
# Default apps to build when none is specified (comment to build all)
DEFAULT_APPS = "launcher retro-core gwenesis fmsx prboom-go gbsp"
