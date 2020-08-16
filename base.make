PROJECT_VER := $(shell git describe --tags --abbrev=5 --long --dirty='*')
PROJECT_VER := $(or $(PROJECT_VER), $(shell date +%Y%m%d))

SDKCONFIG_DEFAULTS := ../base.sdkconfig

EXTRA_COMPONENT_DIRS := ../components

include $(IDF_PATH)/make/project.mk
