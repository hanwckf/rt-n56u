# This makefile is a wrapper around the vendor Makefile to allow default
# targets to be added for all vendor Makefiles

include $(dir_v)/Makefile

# This provides a dummy romfs.post target which runs after all of the other romfs targets.
# You can add your own romfs.post:: (not the double colon) target in your vendor Makefile

romfs.post::
