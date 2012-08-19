# Linux NVRAM Makefile
#
# Copyright 2004, ASUSTeK Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
# $Id: Makefile,v 1.1 2007/06/08 10:22:42 arthur Exp $
#

#.PHONY: all clean tags dep romfs

SHDIR = $(ROOTDIR)/user/shared

CFLAGS	+= -Wall -DASUS -I.
CFLAGS	+= -I$(SHDIR)/include
CFLAGS	+= -s
LDFLAGS	+= -L. -L../shared -lshared

SOURCES = nvram.c test_nvram.c
OBJS = nvram.o 
TARGETS = nvram

all: $(TARGETS)

nvram: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJS)
	$(STRIP) $@

clean:
	rm -f $(TARGETS) *.o *.so
	rm -f test_nvram

romfs:
	$(ROMFSINST) /usr/sbin/nvram

