#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := retro-go
PROJECT_VER = \"$(shell date "+%Y-%m-%d")\"-$(shell git rev-parse HEAD | cut -b 1-10)\"

EXTRA_COMPONENT_DIRS := ../components

include $(IDF_PATH)/make/project.mk
