#!/bin/sh
mkdir -p /etc/ssl
tar -xzf /etc_ro/certs.tgz -C /etc/ssl
if [ ! -f "/etc/storage/koolproxy_rules_script.sh" ] ; then
cp -rf /etc_ro/koolproxy_rules_script.sh /etc/storage/
chmod 755 "/etc/storage/koolproxy_rules_script.sh"
fi
if [ ! -f "/etc/storage/koolproxy_rules_list.sh" ] ; then
cp -rf /etc_ro/koolproxy_rules_list.sh /etc/storage/
chmod 755 "/etc/storage/koolproxy_rules_list.sh"
fi
if [ ! -f "/etc/storage/ad_config_script.sh" ] ; then
cp -rf /etc_ro/ad_config_script.sh /etc/storage/
chmod 755 "/etc/storage/ad_config_script.sh"
fi
if [ ! -f "/etc/storage/frp_script.sh" ] ; then
cp -rf /etc_ro/frp_script.sh /etc/storage/
chmod 755 "/etc/storage/frp_script.sh"
fi
if [ ! -f "/etc/storage/caddy_script.sh" ] ; then
cp -rf /etc_ro/caddy_script.sh /etc/storage/
chmod 755 "/etc/storage/caddy_script.sh"
fi
if [ ! -f "/etc/storage/smartdns_address.conf" ] ; then
cp -rf /etc_ro/smartdns_address.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_address.conf"
fi
if [ ! -f "/etc/storage/smartdns_blacklist-ip.conf" ] ; then
cp -rf /etc_ro/smartdns_blacklist-ip.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_blacklist-ip.conf"
fi
if [ ! -f "/etc/storage/smartdns_whitelist-ip.conf" ] ; then
cp -rf /etc_ro/smartdns_whitelist-ip.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_whitelist-ip.conf"
fi
if [ ! -f "/etc/storage/smartdns_custom.conf" ] ; then
cp -rf /etc_ro/smartdns_custom.conf /etc/storage/
chmod 755 "/etc/storage/smartdns_custom.conf"
fi
if [ ! -f "/etc/storage/ddns_script.sh" ] ; then
cp -rf /etc_ro/ddns_script.sh /etc/storage/
chmod 755 "/etc/storage/ddns_script.sh"
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
if [ ! -f "/etc/storage/adbyby_host.sh" ] ; then
cp -rf /etc_ro/adbyby_host.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_host.sh"
fi
if [ ! -f "/etc/storage/adbyby_blockip.sh" ] ; then
cp -rf /etc_ro/adbyby_blockip.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_blockip.sh"
fi
if [ ! -f "/etc/storage/adbyby_rules.sh" ] ; then
cp -rf /etc_ro/adbyby_rules.sh /etc/storage/
chmod 755 "/etc/storage/adbyby_rules.sh"
fi
if [ ! -f "/etc/storage/ss_dlink.sh" ] ; then
cp -rf /etc_ro/ss_dlink.sh /etc/storage/
chmod 755 "/etc/storage/ss_dlink.sh"
fi
if [ ! -f "/etc/storage/ss_dom.sh" ] ; then
cp -rf /etc_ro/ss_dom.sh /etc/storage/
chmod 755 "/etc/storage/ss_dom.sh"
fi
if [ ! -f "/etc/storage/uss_dom.sh" ] ; then
cp -rf /etc_ro/uss_dom.sh /etc/storage/
chmod 755 "/etc/storage/uss_dom.sh"
fi
if [ ! -f "/etc/storage/ss_ip.sh" ] ; then
cp -rf /etc_ro/ss_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_ip..sh"
fi
if [ ! -f "/etc/storage/ss_lan_bip.sh" ] ; then
cp -rf /etc_ro/ss_lan_bip.sh /etc/storage/
chmod 755 "/etc/storage/ss_lan_bip..sh"
fi
if [ ! -f "/etc/storage/ss_lan_ip.sh" ] ; then
cp -rf /etc_ro/ss_lan_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_lan_ip..sh"
fi
if [ ! -f "/etc/storage/ss_wan_ip.sh" ] ; then
cp -rf /etc_ro/ss_wan_ip.sh /etc/storage/
chmod 755 "/etc/storage/ss_wan_ip..sh"
fi
if [ ! -f "/etc/storage/dnsmasq.oversea/oversea_list.conf" ] ; then
mkdir -p /etc/storage/dnsmasq.oversea
cp -rf /etc_ro/oversea_list.conf /etc/storage/dnsmasq.oversea/
chmod 755 "/etc/storage/dnsmasq.oversea/oversea_list.conf"
fi
if [ ! -f "/etc/storage/gfwlist/gfwlist_list.conf" ] ; then
mkdir -p /etc/storage/gfwlist
cp -rf /etc_ro/gfwlist_list.conf /etc/storage/gfwlist/
chmod 755 "/etc/storage/gfwlist/gfwlist_list.conf"
fi