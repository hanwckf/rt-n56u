#!/bin/sh

script_start="/etc/storage/start_script.sh"

# create start script
cat > "$script_start" <<EOF
#!/bin/sh

### Custom user script for tuning router before start

########################################################
### tune linux kernel
########################################################
# core
echo 524288   > /proc/sys/net/core/rmem_max
echo 524288   > /proc/sys/net/core/wmem_max

# backlog for UNIX sockets
echo 64       > /proc/sys/net/unix/max_dgram_qlen

# igmp
echo 30       > /proc/sys/net/ipv4/igmp_max_memberships
echo 2        > /proc/sys/net/ipv4/conf/all/force_igmp_version
echo 2        > /proc/sys/net/ipv4/conf/default/force_igmp_version

# arp
echo 1        > /proc/sys/net/ipv4/conf/all/arp_filter
echo 1        > /proc/sys/net/ipv4/conf/all/arp_announce
echo 1        > /proc/sys/net/ipv4/conf/default/arp_announce

# neigh ipv4
echo 256      > /proc/sys/net/ipv4/neigh/default/gc_thresh1
echo 1024     > /proc/sys/net/ipv4/neigh/default/gc_thresh2
echo 2048     > /proc/sys/net/ipv4/neigh/default/gc_thresh3

# neigh ipv6
if [ -d /proc/sys/net/ipv6 ] ; then
	echo 256      > /proc/sys/net/ipv6/neigh/default/gc_thresh1
	echo 1024     > /proc/sys/net/ipv6/neigh/default/gc_thresh2
	echo 2048     > /proc/sys/net/ipv6/neigh/default/gc_thresh3
fi

# reverse-path filter
echo 1        > /proc/sys/net/ipv4/conf/default/rp_filter
echo 1        > /proc/sys/net/ipv4/conf/eth2/rp_filter

# conntrack
echo 0        > /proc/sys/net/netfilter/nf_conntrack_checksum
echo 1        > /proc/sys/net/netfilter/nf_conntrack_tcp_be_liberal
echo 300      > /proc/sys/net/netfilter/nf_conntrack_generic_timeout
echo 1800     > /proc/sys/net/netfilter/nf_conntrack_tcp_timeout_established
echo 30       > /proc/sys/net/netfilter/nf_conntrack_icmp_timeout
echo 50       > /proc/sys/net/netfilter/nf_conntrack_udp_timeout

# panic
echo 1        > /proc/sys/kernel/panic
echo 1        > /proc/sys/kernel/panic_on_oops
echo 0        > /proc/sys/vm/panic_on_oom

EOF
chmod 755 "$script_start"

if [ -z "$1" ] ; then
	$script_start
	mtd_storage.sh save
fi
