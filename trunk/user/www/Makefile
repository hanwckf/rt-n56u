#
# Broadcom web pages
#
# Copyright 2003, ASUSTeK Inc.
# All Rights Reserved.                
#                                     
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of ASUSTeK Inc.;   
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior      
# written permission of ASUSTeK Inc..                            
#
# $Id: Makefile,v 1.3 2007/04/09 12:08:09 shinjung Exp $
#

PAGES := *
PAGES_DIR := graph/*
LANGUAGE = EN
ROMFS_DIR = $(ROOTDIR)/romfs

#all: install-$(MODEL_NAME)
all:
#pages.mk

romfs:
	echo "Make www romfs"
	rm -Rf $(ROMFS_DIR)/www
	cp -R www_N56U $(ROMFS_DIR)/www
	rm -f $(ROMFS_DIR)/www/httpd_check.htm
	echo "ASUSTeK" > $(ROMFS_DIR)/www/httpd_check.htm
	touch $(ROMFS_DIR)/tmp/settings
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings.CFG && cd ../..
	cd $(ROMFS_DIR)/www && ln -sf ../tmp/settings Settings_RT-N56U.CFG && cd ../..
	cd $(ROMFS_DIR)/www && ln -sf /opt/share/www/custom custom
	rm -f $(ROMFS_DIR)/tmp/settings
	
# Padavan. Remove some languages (sorry, I'm not racialist, but more space needed in 8MB NOR Flash)
	rm -f $(ROMFS_DIR)/www/CN.dict
	rm -f $(ROMFS_DIR)/www/JP.dict
	rm -f $(ROMFS_DIR)/www/MS.dict
	rm -f $(ROMFS_DIR)/www/TH.dict
	rm -f $(ROMFS_DIR)/www/TR.dict
	rm -f $(ROMFS_DIR)/www/TW.dict
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_BR)" = "y" ] ; then \
		echo "LANG_BR=Brazil" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/BR.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_CZ)" = "y" ] ; then \
		echo "LANG_CZ=Česky" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/CZ.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_DA)" = "y" ] ; then \
		echo "LANG_DA=Dansk" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/DA.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_DE)" = "y" ] ; then \
		echo "LANG_DE=Deutsch" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/DE.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_ES)" = "y" ] ; then \
		echo "LANG_ES=Español" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/ES.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_FI)" = "y" ] ; then \
		echo "LANG_FI=Finsk" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/FI.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_FR)" = "y" ] ; then \
		echo "LANG_FR=Français" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/FR.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_NO)" = "y" ] ; then \
		echo "LANG_NO=Norsk" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/NO.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_PL)" = "y" ] ; then \
		echo "LANG_PL=Polski" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/PL.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_RU)" = "y" ] ; then \
		echo "LANG_RU=Pусский" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/RU.dict ; \
	fi
	
	if [ "$(CONFIG_FIRMWARE_INCLUDE_LANG_SV)" = "y" ] ; then \
		echo "LANG_SV=Svensk" >> $(ROMFS_DIR)/www/EN.header ; \
	else \
		rm -f $(ROMFS_DIR)/www/SV.dict ; \
	fi
	
	cat $(ROMFS_DIR)/www/EN.header $(ROMFS_DIR)/www/EN.footer > $(ROMFS_DIR)/www/EN.dict
	rm -f $(ROMFS_DIR)/www/EN.header
	rm -f $(ROMFS_DIR)/www/EN.footer
	
# remove unused resorces
	rm -f $(ROMFS_DIR)/www/upnp.asp
	rm -f $(ROMFS_DIR)/www/QoS_EZQoS.asp
	rm -f $(ROMFS_DIR)/www/EZQoS.asp
	rm -f $(ROMFS_DIR)/www/Advanced_QOSUserSpec_Content.asp
	rm -f $(ROMFS_DIR)/www/images/ill-*.gif
	rm -f $(ROMFS_DIR)/www/images/band-*.gif

#pages.mk:
#	echo "PAGES := $(PAGES)" > $@

install:
	./mkweb WL500gx "EN"	
	install -d $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.asp $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.js $(INSTALLDIR)/www
	install -m 644 web_WL500gx/*.css $(INSTALLDIR)/www
	install -d $(INSTALLDIR)/www/graph
	install -m 644 web_WL500gx/$(PAGES_DIR).jpg $(INSTALLDIR)/www/graph
	install -m 644 web_WL500gx/$(PAGES_DIR).gif $(INSTALLDIR)/www/graph
	rm -rf web
	rm -rf web_WL500gx

install-%:
	echo "www install-"
	echo $(LANGUAGE)
	./mkweb $* "$(LANGUAGE)"
	rm -rf www_$*	
	install -d www_$*
	install -m 644 web_$*/*.asp www_$*
	install -m 644 web_$*/*.js www_$*
	install -m 644 web_$*/*.css www_$*
#	( if [ "$*" != "WL520g" ] && [ "$*" != "WL550gE" ] && [ "$*" != "WL500bl" ] && [ "$*" != "WL331gl" ]; then \
		install -m 644 web_$*/*.cab www_$*;\
	fi )
	install -d www_$*/graph
	install -m 644 web_$*/$(PAGES_DIR).jpg www_$*/graph
	install -m 644 web_$*/$(PAGES_DIR).gif www_$*/graph

	ln -sf /tmp/settings www_$*/Settings.CFG
	sleep 1
	#cp ../httpd/*.dict www_$*/ -f		# disable for tmp
	#rm -rf ../httpd/*.dict			# disable for tmp
	( if [ "$*" = "WL500gpv2" ]; then \
		install -d www_$*/slider; \
		install -m 644 web_$*/slider/*.js www_$*/slider; \
		install -m 644 web_$*/slider/*.gif www_$*/slider; \
	fi )

	ln -sf /var/tmp/display.jpg www_$*/display.jpg

	rm -rf web
	rm -rf web_$*

clean:
#	rm -f pages.mk
