#!/bin/sh
if [ ! -f "/etc/storage/ad_config_script.sh" ] ; then
cp -rf /etc_ro/ad_config_script.sh /etc/storage/
chmod 755 "/etc/storage/ad_config_script.sh"
fi
if [ ! -f "/etc/storage/adbyby_adblack.sh" ] ; then
cp -rf /etc_ro/adbyby_adblack.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adblack.sh"
fi
if [ ! -f "/etc/storage/adbyby_adesc.sh" ] ; then
cp -rf /etc_ro/adbyby_adesc.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adesc.sh"
fi
if [ ! -f "/etc/storage/adbyby_adhost.sh" ] ; then
cp -rf /etc_ro/adbyby_adhost.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_adhost.sh"
fi
if [ ! -f "/etc/storage/adbyby_blockip.sh" ] ; then
cp -rf /etc_ro/adbyby_blockip.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_blockip.sh"
fi
if [ ! -f "/etc/storage/adbyby_rules.sh" ] ; then
cp -rf /etc_ro/adbyby_rules.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_rules.sh"
fi
