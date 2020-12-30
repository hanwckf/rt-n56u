#
#	Makefile -- Vendor specific build instructions
#

.EXPORT_ALL_VARIABLES:
.PHONY: all romfs image clean

#
# Include architecture specific build rules.
#

ifndef ROOTDIR
ROOTDIR=..
endif

UCLINUX_BUILD_USER=1
-include $(LINUX_CONFIG)
include $(PROJECT_CONFIG)
include $(ARCH_CONFIG)

VEND=$(ROOTDIR)/vendors

dir_v := $(VEND)/$(CONFIG_VENDOR)/.

# Note: the all and clean targets are not normally used,
# since user/Makefile jumps directory to the product directory
all image clean romfs romfs.post:
	$(MAKE) -C $(dir_v) dir_v=$(dir_v) -f $(VEND)/vendors-common.mak $@

