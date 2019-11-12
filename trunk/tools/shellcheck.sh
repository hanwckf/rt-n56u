#!/bin/sh

TOPDIR="."

check_dirs="trunk/user/scutclient/scripts \
	trunk/user/shadowsocks/scripts \
	trunk/user/mentohust/scripts \
	trunk/user/chnroute/scripts \
	trunk/user/dns-forwarder \
	trunk/user/softethervpn \
	trunk/user/ttyd \
	trunk/user/vlmcsd \
	trunk/user/aria2 \
	trunk/user/openssh \
	trunk/user/scripts"

exs="SC2166,SC2164,SC2174,SC2039"
sc_opts="-W 0 -x -S warning -e $exs"

for s in $check_dirs; do
	shellcheck ${sc_opts} ${TOPDIR}/${s}/*.sh
done

exit 0
