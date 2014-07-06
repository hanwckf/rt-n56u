#!/bin/sh

result=0
mtd_part_name="Storage"
mtd_part_dev="/dev/mtdblock5"
mtd_part_size=65536
dir_storage="/etc/storage"
ers="/tmp/.storage_erase"
tmp="/tmp/.storage_tar"
hsh="/tmp/hashes/storage_md5"

func_get_mtd()
{
	local mtd_part mtd_char mtd_index
	mtd_part=`cat /proc/mtd | grep \"$mtd_part_name\"`
	mtd_char=`echo $mtd_part | awk -F':' '{ print $1 }'`
	if [ -n "$mtd_char" ] ; then
		mtd_index=${mtd_char/mtd/}
		if [ -n "$mtd_index" ] && [ $mtd_index -ge 4 ] ; then
			mtd_part_dev="/dev/mtdblock${mtd_index}"
			mtd_part_size=`echo $mtd_part | awk '{ printf("0x%d\n",$2) }' | awk '{ printf("%d\n",$1) }'`
		fi
	fi
}

func_load()
{
	local fsz
	
	[ ! -d "$dir_storage" ] && mkdir -p -m 755 $dir_storage
	echo "Loading files from mtd partition \"$mtd_part_dev\""
	
	bzcat $mtd_part_dev > $tmp 2>/dev/null
	fsz=`stat -c %s $tmp 2>/dev/null`
	if [ -n "$fsz" ] && [ $fsz -gt 0 ] ; then
		md5sum $tmp > $hsh
		tar xf $tmp -C $dir_storage 2>/dev/null
		echo "Done."
	else
		result=1
		rm -f $hsh
		echo "Error! Invalid storage data"
	fi
	rm -f $tmp
	rm -f $ers
}

func_save()
{
	local fsz tbz2
	
	[ -f "$ers" ] && return 1
	
	[ ! -d "$dir_storage" ] && mkdir -p -m 755 $dir_storage
	echo "Save files to mtd partition \"$mtd_part_dev\""
	
	tbz2="${tmp}.bz2"
	rm -f $tmp
	rm -f $tbz2
	cd $dir_storage
	find * -print0 | xargs -0 touch -c -h -t 201001010000.00
	find * ! -type d -print0 | sort -z | xargs -0 tar -cf $tmp 2>/dev/null
	cd - >>/dev/null
	md5sum -c -s $hsh 2>/dev/null
	if [ $? -eq 0 ] ; then
		echo "Storage hash is not changed, skip write to mtd partition. Exit."
		rm -f $tmp
		return 0
	fi
	[ -f "$tmp" ] && md5sum $tmp > $hsh
	bzip2 -9 $tmp 2>/dev/null
	fsz=`stat -c %s $tbz2 2>/dev/null`
	if [ -n "$fsz" ] && [ $fsz -gt 0 ] && [ $fsz -lt $mtd_part_size ] ; then
		mtd_write write $tbz2 $mtd_part_name
		if [ $? -eq 0 ] ; then
			echo "Done."
		else
			result=1
			echo "Error! mtd write FAILED"
		fi
	else
		result=1
		echo "Error! Invalid storage data size: $fsz"
	fi
	rm -f $tmp
	rm -f $tbz2
}

func_erase()
{
	echo "Erase mtd partition \"$mtd_part_dev\""
	
	mtd_write erase $mtd_part_name
	if [ $? -eq 0 ] ; then
		rm -f $hsh
		rm -rf $dir_storage
		mkdir -p -m 755 $dir_storage
		touch "$ers"
		echo "Done."
	else
		result=1
		echo "Error! mtd erase FAILED"
	fi
}

func_reset()
{
	rm -f $ers
	rm -rf $dir_storage
	mkdir -p -m 755 $dir_storage
}

func_fill()
{
	dir_httpssl="$dir_storage/https"
	dir_dnsmasq="$dir_storage/dnsmasq"
	dir_ovpnsvr="$dir_storage/openvpn/server"
	dir_ovpncli="$dir_storage/openvpn/client"
	dir_inadyn="$dir_storage/inadyn"

	script_start="$dir_storage/start_script.sh"
	script_started="$dir_storage/started_script.sh"
	script_postf="$dir_storage/post_iptables_script.sh"
	script_postw="$dir_storage/post_wan_script.sh"
	script_inets="$dir_storage/inet_state_script.sh"
	script_vpnsc="$dir_storage/vpns_client_script.sh"
	script_vpncs="$dir_storage/vpnc_server_script.sh"

	user_hosts="$dir_dnsmasq/hosts"
	user_dnsmasq_conf="$dir_dnsmasq/dnsmasq.conf"
	user_ovpnsvr_conf="$dir_ovpnsvr/server.conf"
	user_ovpncli_conf="$dir_ovpncli/client.conf"
	user_inadyn_conf="$dir_inadyn/inadyn.conf"

	# create https dir
	[ ! -d "$dir_httpssl" ] && mkdir -p -m 700 "$dir_httpssl"

	# create start script
	if [ ! -f "$script_start" ] ; then
		reset_ss.sh -a
	fi

	# create started script
	if [ ! -f "$script_started" ] ; then
		cat > "$script_started" <<EOF
#!/bin/sh

### Custom user script
### Called after router started and network is ready

### Example - load ipset modules
#modprobe ip_set
#modprobe ip_set_hash_ip
#modprobe ip_set_hash_net
#modprobe ip_set_bitmap_ip
#modprobe ip_set_list_set
#modprobe xt_set

EOF
		chmod 755 "$script_started"
	fi

	# create post-iptables script
	if [ ! -f "$script_postf" ] ; then
		cat > "$script_postf" <<EOF
#!/bin/sh

### Custom user script
### Called after internal iptables reconfig (firewall update)

EOF
		chmod 755 "$script_postf"
	fi

	# create post-wan script
	if [ ! -f "$script_postw" ] ; then
		cat > "$script_postw" <<EOF
#!/bin/sh

### Custom user script
### Called after internal WAN up/down action
### \$1 - WAN action (up/down)
### \$2 - WAN interface name (e.g. eth3 or ppp0)

EOF
		chmod 755 "$script_postw"
	fi

	# create inet-state script
	if [ ! -f "$script_inets" ] ; then
		cat > "$script_inets" <<EOF
#!/bin/sh

### Custom user script
### Called on Internet status changed
### \$1 - Internet status (0/1)
### \$2 - elapsed time (s) from previous state

logger -t "di" "Internet state: \$1, elapsed time: \$2s."

EOF
		chmod 755 "$script_inets"
	fi

	# create vpn server action script
	if [ ! -f "$script_vpnsc" ] ; then
		cat > "$script_vpnsc" <<EOF
#!/bin/sh

### Custom user script
### Called after remote peer connected/disconnected to internal VPN server
### \$1 - peer action (up/down)
### \$2 - peer interface name (e.g. ppp10)
### \$3 - peer local IP address
### \$4 - peer remote IP address
### \$5 - peer name

peer_if="\$2"
peer_ip="\$4"
peer_name="\$5"

### example: add static route to private LAN subnet behind a remote peer

func_ipup()
{
#  if [ "\$peer_name" == "dmitry" ] ; then
#    route add -net 192.168.5.0 netmask 255.255.255.0 dev \$peer_if
#  elif [ "\$peer_name" == "victoria" ] ; then
#    route add -net 192.168.8.0 netmask 255.255.255.0 dev \$peer_if
#  fi
}

func_ipdown()
{
#  if [ "\$peer_name" == "dmitry" ] ; then
#    route del -net 192.168.5.0 netmask 255.255.255.0 dev \$peer_if
#  elif [ "\$peer_name" == "victoria" ] ; then
#    route del -net 192.168.8.0 netmask 255.255.255.0 dev \$peer_if
#  fi
}

case "\$1" in
up)
  func_ipup
  ;;
down)
  func_ipdown
  ;;
esac

EOF
		chmod 755 "$script_vpnsc"
	fi

	# create vpn client action script
	if [ ! -f "$script_vpncs" ] ; then
		cat > "$script_vpncs" <<EOF
#!/bin/sh

### Custom user script
### Called after internal VPN client connected/disconnected to remote VPN server
### \$1        - action (up/down)
### \$IFNAME   - tunnel interface name (e.g. ppp5)
### \$IPLOCAL  - tunnel local IP address
### \$IPREMOTE - tunnel remote IP address
### \$DNS1     - peer DNS1
### \$DNS2     - peer DNS2

# private LAN subnet behind a remote server (example)
peer_lan="192.168.9.0"
peer_msk="255.255.255.0"

### example: add static route to private LAN subnet behind a remote server

func_ipup()
{
#  route add -net \$peer_lan netmask \$peer_msk gw \$IPREMOTE dev \$IFNAME
}

func_ipdown()
{
#  route del -net \$peer_lan netmask \$peer_msk gw \$IPREMOTE dev \$IFNAME
}

case "\$1" in
up)
  func_ipup
  ;;
down)
  func_ipdown
  ;;
esac

EOF
		chmod 755 "$script_vpncs"
	fi

	# create user dnsmasq.conf
	[ ! -d "$dir_dnsmasq" ] && mkdir -p -m 755 "$dir_dnsmasq"
	for i in dnsmasq.conf hosts ; do
		[ -f "$dir_storage/$i" ] && mv -n "$dir_storage/$i" "$dir_dnsmasq"
	done
	if [ ! -f "$user_dnsmasq_conf" ] ; then
		cat > "$user_dnsmasq_conf" <<EOF
# Custom user conf file for dnsmasq
# Please add needed params only!

### Web Proxy Automatic Discovery (WPAD)
dhcp-option=252,"\n"

### Set the limit on DHCP leases, the default is 150
#dhcp-lease-max=150

### Add local-only domains, queries are answered from hosts or DHCP only
#local=/router/localdomain/

### Examples:

### Tells dnsmasq to forward queries for this domains to DNS 10.25.11.30
#server=/mit.ru/izmuroma.ru/10.25.11.30

### Enable built-in TFTP server
#enable-tftp

### Set the root directory for files available via TFTP.
#tftp-root=/opt/srv/tftp

### Make the TFTP server more secure
#tftp-secure

### Set the boot filename for netboot/PXE
#dhcp-boot=pxelinux.0

EOF
		chmod 644 "$user_dnsmasq_conf"
	fi

	# create user inadyn.conf"
	[ ! -d "$dir_inadyn" ] && mkdir -p -m 755 "$dir_inadyn"
	if [ ! -f "$user_inadyn_conf" ] ; then
		cat > "$user_inadyn_conf" <<EOF
# Custom user conf file for inadyn DDNS client
# Please add only new custom system!

### Example for twoDNS.de:

#system custom@http_srv_basic_auth
#  ssl
#  checkip-url checkip.two-dns.de /
#  server-name update.twodns.de
#  server-url /update\?hostname=
#  username account
#  password secret
#  alias example.dd-dns.de

EOF
		chmod 644 "$user_inadyn_conf"
	fi

	# create user hosts
	if [ ! -f "$user_hosts" ] ; then
		cat > "$user_hosts" <<EOF
# Custom user hosts file
# Example:
# 192.168.1.100		Boo
EOF
		chmod 644 "$user_hosts"
	fi

	# create openvpn files
	if [ -x /usr/sbin/openvpn ] ; then
		[ ! -d "$dir_ovpncli" ] && mkdir -p -m 700 "$dir_ovpncli"
		[ ! -d "$dir_ovpnsvr" ] && mkdir -p -m 700 "$dir_ovpnsvr"
		dir_ovpn="$dir_storage/openvpn"
		for i in ca.crt dh1024.pem server.crt server.key server.conf ta.key ; do
			[ -f "$dir_ovpn/$i" ] && mv -n "$dir_ovpn/$i" "$dir_ovpnsvr"
		done
		if [ ! -f "$user_ovpnsvr_conf" ] ; then
			cat > "$user_ovpnsvr_conf" <<EOF
# Custom user conf file for OpenVPN server
# Please add needed params only!

### Authenticate packets with HMAC using message digest algorithm
auth SHA1      # SHA1 160 bit (default)
;auth SHA256    # SHA256 256 bit
;auth SHA512    # SHA512 512 bit

### Encrypt packets with cipher algorithm
cipher BF-CBC        # Blowfish 128 bit (default)
;cipher AES-128-CBC   # AES 128 bit
;cipher AES-256-CBC   # AES 256 bit
;cipher DES-EDE3-CBC  # Triple-DES 192 bit
;cipher none          # No encryption

### Enable LZO compression on the VPN link
comp-lzo

### Max clients limit
max-clients 10

### Internally route client-to-client traffic
client-to-client

### Allow clients with duplicate "Common Name"
;duplicate-cn

### Keepalive and timeout
keepalive 10 60

### Process priority level (0..19)
nice 3

### Syslog verbose level
verb 0
mute 10

EOF
			chmod 644 "$user_ovpnsvr_conf"
		fi

		if [ ! -f "$user_ovpncli_conf" ] ; then
			cat > "$user_ovpncli_conf" <<EOF
# Custom user conf file for OpenVPN client
# Please add needed params only!

### Authenticate packets with HMAC using message digest algorithm
auth SHA1      # SHA1 160 bit (default)
;auth SHA256    # SHA256 256 bit
;auth SHA512    # SHA512 512 bit

### Encrypt packets with cipher algorithm
cipher BF-CBC        # Blowfish 128 bit (default)
;cipher AES-128-CBC   # AES 128 bit
;cipher AES-256-CBC   # AES 256 bit
;cipher DES-EDE3-CBC  # Triple-DES 192 bit
;cipher none          # No encryption

### Enable LZO compression on the VPN link
comp-lzo

### If your server certificates with the nsCertType field set to "server"
ns-cert-type server

### All outgoing IP traffic will be redirected over the VPN
;redirect-private def1

### Process priority level (0..19)
nice 0

### Syslog verbose level
verb 0
mute 10

EOF
			chmod 644 "$user_ovpncli_conf"
		fi
	fi
}

case "$1" in
load)
	func_get_mtd
	func_load
	;;
save)
	func_get_mtd
	func_save
	;;
erase)
	func_get_mtd
	func_erase
	;;
reset)
	func_reset
	func_fill
	;;
fill)
	func_fill
	;;
*)
	echo "Usage: $0 {load|save|erase|reset|fill}"
	exit 1
	;;
esac

exit $result
