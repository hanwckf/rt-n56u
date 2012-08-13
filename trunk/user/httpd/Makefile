#
# milli_httpd Makefile
#
# Copyright (C) 2001 ASUSTeK Inc.
#
# $Id: Makefile,v 1.5 2007/04/14 11:19:47 shinjung Exp $
#

#.PHONY: all clean sym_link install romfs gen_syms

-include $(LINUX_CONFIG)

SHDIR = $(ROOTDIR)/user/shared
USERDIR = $(ROOTDIR)/user

DOIPV6 = $(if $(CONFIG_IPV6),-DUSE_IPV6,)

CFLAGS += -I. -I$(SHDIR) -I$(SHDIR)/include -I$(USERDIR)/libdisk -I$(USERDIR)/iptables/include -I$(USERDIR)/wireless_tools
CFLAGS += -I$(LINUXINCDIR)
CFLAGS += -Wall -DASUS_NVRAM -DASUS_DDNS -DTRANSLATE_ON_FLY -DLinux -DMIPS $(GLOBAL_OPTIONS)
CFLAGS += $(DOIPV6)

LDFLAGS	= -L$(SHDIR) -L$(USERDIR)/libdisk -L$(USERDIR)/iptables/libiptc -L$(USERDIR)/wireless_tools
LDFLAGS += -lm -lshared -ldisk -liptc -liw

#CFLAGS += -DW7_LOGO
#CFLAGS += -DWIFI_LOGO

vpath %.o $(ROOTDIR)/user/shared #2008.10 magic

LANG_FILE = EN.dict

WWWDIR = ../www/www_N56U  #Viz changed 2010.08 

vpath %.dict $(WWWDIR)/

all: httpd 
	$(STRIP) httpd

dep:
	@echo "Generate depend file..."
	@for i in `find . -name "*.c"`; do ${CC} -MM ${CFLAGS} "$${i}"; done > $@

clean:
	rm -f *.o *~ httpd
	for f in $(LANG_FILE) ; do rm -f $$f; done
	rm -f *.dict

#%.dict: translate_%.txt #2008.10 magic
%.dict: %.dict #2008.10 magic
	egrep '[^=]*=[^=]*' $^ | sed -e 's,<#ZVMODELVZ#>,$(MODEL_NAME),g' -e 's,\r,,' > $@

romfs:
	$(ROMFSINST) /usr/sbin/httpd

# 2008.10 magic. add notify_rc.o & discover.o & initial_web_hook.o
# Viz add aspbw.o 2010.08
httpd: httpd.o notify_rc.o discover.o ej.o cgi.o web_ex.o common.o nvram_f.o broadcom.o crc32.o initial_web_hook.o aspbw.o netconf_linux.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
