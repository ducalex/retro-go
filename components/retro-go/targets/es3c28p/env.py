# This file is injected late into rg_tool.py, you can run arbitrary python code here
# For example override python variables or set environment variables with os.putenv

# Espressif chip in the device
IDF_TARGET = "esp32s3"
# .fw file format, if supported by the device
FW_FORMAT = "none"
# Default apps to build when none is specified (comment to build all)
# DEFAULT_APPS = "launcher prboom-go"

# Build the Bluetooth LE HID gamepad driver (see RG_GAMEPAD_BLE_MAP in config.h)
ENABLE_BLUETOOTH = True

# The BLE stack costs roughly 200KB of flash per app, which doesn't fit in the default
# 1MB OTA partitions. This board has 16MB, so grow the two apps we build for it. Done
# here rather than in rg_tool.py so that no other target's flash layout changes.
PROJECT_APPS = dict(PROJECT_APPS)
PROJECT_APPS["launcher"] = [0, 16, 1572864]
PROJECT_APPS["retro-core"] = [0, 16, 1835008]
