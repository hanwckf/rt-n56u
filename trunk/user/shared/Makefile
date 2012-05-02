#
# Linux router shared code Makefile
#
# Copyright 2005, ASUSTeK Inc.
# All Rights Reserved.
# 
# THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
# KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
# SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
#
#

CFLAGS	+= -fPIC
CFLAGS	+= -I. -I./include -I../httpd -Wall -DASUS_NVRAM -D__CONFIG_NAT__ -DDLM 

CFLAGS	+= -s
#CFLAGS	 += -DW7_LOGO
#CFLAGS  += -DW7_WIFI
#CFLAGS  += -DMR
LDFLAGS += -L.

# 2008.10 magic. add notify_rc.o
OBJS := shutils.o crc.o http.o defaults.o nvram_linux.o linux_timer.o nvparse.o notify_rc.o bin_sem_asus.o 
OBJS += semaphore.o flash_mtd.o pids.o files.o

all: libshared.so

libshared.so: $(OBJS)
	$(LD) -shared -o $@ $^

romfs:
	$(ROMFSINST) -S /lib/libshared.so

clean:
	rm -f *.o libshared.so
