#
# Makefile for Shortcut FE.
#

obj-m += shortcut-fe.o fast-classifier.o

# ipv6
ccflags-y += -DSFE_SUPPORT_IPV6
obj-m += shortcut-fe-ipv6.o
shortcut-fe-ipv6-objs := \
	sfe_ipv6.o

# ipv4
shortcut-fe-objs := \
	sfe_ipv4.o

# cm
obj-m += shortcut-fe-cm.o
shortcut-fe-cm-objs := \
	sfe_cm.o
