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
	find * ! -type l -print0 | xargs -0 touch -c -t 201001010000.00
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
	script_start="$dir_storage/start_script.sh"
	script_started="$dir_storage/started_script.sh"
	script_postf="$dir_storage/post_iptables_script.sh"
	script_postw="$dir_storage/post_wan_script.sh"
	script_vpnsc="$dir_storage/vpns_client_script.sh"
	user_hosts="$dir_storage/hosts"
	user_dnsmasq_conf="$dir_storage/dnsmasq.conf"

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

	# create vpn server action script
	if [ ! -f "$script_vpnsc" ] ; then
		cat > "$script_vpnsc" <<EOF
#!/bin/sh

### Custom user script
### Called after peer connect/disconnect to internal VPN server
### \$1 - peer action (up/down), where up - peer connect, down - peer disconnect
### \$2 - peer interface name (e.g. ppp10)
### \$3 - peer local IP address
### \$4 - peer remote IP address
### \$5 - peer name

EOF
		chmod 755 "$script_vpnsc"
	fi

	# create user hosts
	if [ ! -f "$user_hosts" ] ; then
		cat > "$user_hosts" <<EOF
# Custom user hosts file
# Example:
# 192.168.1.100		Obi-Wan
EOF
		chmod 644 "$user_hosts"
	fi

	# create user dnsmasq.conf
	if [ ! -f "$user_dnsmasq_conf" ] ; then
		cat > "$user_dnsmasq_conf" <<EOF
# Custom user dnsmasq.conf file

#Examples:

### Enable built-in TFTP server
#enable-tftp

### Set the root directory for files available via TFTP.
#tftp-root=/opt/srv/tftp

### Make the TFTP server more secure
#tftp-secure

### Set the boot filename for netboot/PXE
#dhcp-boot=pxelinux.0

### Set the limit on DHCP leases, the default is 150
#dhcp-lease-max=150

EOF
		chmod 644 "$user_dnsmasq_conf"
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
