# This file is injected late into rg_tool.py, you can run arbitrary python code here
# For example override python variables or set environment variables with os.putenv

# Espressif chip in the device
IDF_TARGET = "esp32c5"
# .fw file format, if supported by the device
FW_FORMAT = "none"
# Keep the default PoC image small and focused on the launcher plus light emulators.
DEFAULT_APPS = "launcher retro-core"
PROJECT_APPS["retro-core"][2] = 0x110000
