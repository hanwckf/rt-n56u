#
# Makefile for Shortcut FE.
#

obj-m += shortcut-fe.o fast-classifier.o

# ipv6
ifdef CONFIG_IPV6
SFE_SUPPORT_IPV6=y
endif
ccflags-$(SFE_SUPPORT_IPV6) += -DSFE_SUPPORT_IPV6

ccflags-y += -g -DDEBUG
CC += -g -DDEBUG

ifdef SFE_SUPPORT_IPV6
obj-m += shortcut-fe-ipv6.o
shortcut-fe-ipv6-objs := \
	sfe_ipv6.o
endif

# ipv4
shortcut-fe-objs := \
	sfe_ipv4.o

# cm
#obj-m += shortcut-fe-cm.o
#shortcut-fe-cm-objs := \
#	sfe_cm.o
