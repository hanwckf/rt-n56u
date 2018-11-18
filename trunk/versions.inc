FIRMWARE_ROOTFS_VER=3.9
FIRMWARE_BUILDS_VER=099
FIRMWARE_BUILDS_REV=mao-$(shell git rev-list HEAD --first-parent --count 2>/dev/null)

#FIRMWARE_BUILDS_REV=$(shell git rev-parse --short HEAD 2>/dev/null)-mao